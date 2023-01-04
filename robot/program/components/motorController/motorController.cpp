#include "motorController.hpp"


MotorController::MotorController(
    gpio_num_t pinIn1, gpio_num_t pinIn2, 
    gpio_num_t encoderPin, ledc_channel_t ledcChanel1,
    ledc_channel_t ledcChanel2)

    :PIDController(8, 0.1, 0.1, TIMER_CLEAR),
    _encoderTicks(0),_absoluteEncoderTicks(0),
    _engineRadialSpeed(0.0f), _currentRadialEngineSpeed(0.0f)
{
    this->_chanels[0] = ledcChanel1;
    this->_chanels[1] = ledcChanel2;

    // konfiguracja przerwania enkodera, programowo można zastąpić licznikiem esp32
    gpio_config_t pinEncoderConfig;
    memset(&pinEncoderConfig, 0, sizeof(gpio_config_t));
    pinEncoderConfig.intr_type       = GPIO_INTR_NEGEDGE;
    pinEncoderConfig.mode            = GPIO_MODE_INPUT;
    pinEncoderConfig.pin_bit_mask    = ((1 << (int)encoderPin));
    pinEncoderConfig.pull_up_en      = GPIO_PULLUP_DISABLE;
    pinEncoderConfig.pull_down_en    = GPIO_PULLDOWN_DISABLE;

    gpio_config(&pinEncoderConfig);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);   
    gpio_isr_handler_add(encoderPin, MotorController::encoderInteruptHandler, this);

    this->_updater = xTimerCreate("encoTimer", pdMS_TO_TICKS(TIMER_CLEAR), pdTRUE, (void*)this, MotorController::clearEncoderTimer);
    xTimerStart(this->_updater, 0);

    // motor controlPins configuration
    ledc_channel_config_t motor_first_channel;
    motor_first_channel.speed_mode          = LEDC_LOW_SPEED_MODE;
    motor_first_channel.channel             = ledcChanel1;
    motor_first_channel.timer_sel           = LEDC_TIMER_0;
    motor_first_channel.intr_type           = LEDC_INTR_DISABLE;
    motor_first_channel.gpio_num            = (pinIn1); // output pin
    motor_first_channel.duty                = 0; // Set duty to 0%
    motor_first_channel.hpoint              = 0;
    motor_first_channel.flags.output_invert = 1;
    ESP_ERROR_CHECK(ledc_channel_config(&motor_first_channel));

    ledc_channel_config_t motor_second_channel;
    motor_second_channel.speed_mode             = LEDC_LOW_SPEED_MODE;
    motor_second_channel.channel                = ledcChanel2;
    motor_second_channel.timer_sel              = LEDC_TIMER_0;
    motor_second_channel.intr_type              = LEDC_INTR_DISABLE;
    motor_second_channel.gpio_num               = (pinIn2); // output pin
    motor_second_channel.duty                   = 0; // Set duty to 0%
    motor_second_channel.hpoint                 = 0;
    motor_second_channel.flags.output_invert    = 1;
    ESP_ERROR_CHECK(ledc_channel_config(&motor_second_channel));
}
MotorController::~MotorController()
{
    xTimerStop(this->_updater, 0);
}
void MotorController::update()
{
    // regulator pid bazuje na prędkości silnika
    // prędkosc kola jest przeliczana

    float setSpeed  = this->getEngineRadialSpeed();
    int pidToPwm    = 0;
    int pwmCh       = 0; // kanal ledc na ktory ma isc pwm

    if(abs(setSpeed) - 1 > 0) // jezeli predkosc jest wieksza to pid jak nie to hamulec bo pwm=0
    {
        pidToPwm = (int) this->calcOutput(abs(this->getCalculatedEngineRadialSpeed()), abs(setSpeed));
        if(setSpeed < 0)
        {
            // pidToPwm *= -1; // odwrocenie nastaw pid jezeli predkosc ujemna 
            pwmCh = 1;
        }
        else
            pwmCh = 0;

        pidToPwm += ledc_get_duty(LEDC_LOW_SPEED_MODE, this->_chanels[pwmCh]);
        pidToPwm = std::min(pidToPwm, 8192);
        pidToPwm = std::max(pidToPwm, 0);
    }

    // ustawienie wyjsc
    ledc_set_duty(LEDC_LOW_SPEED_MODE, this->_chanels[pwmCh], (int)pidToPwm);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, this->_chanels[pwmCh]);

    // ustawienie drugiego kanalu
    ledc_set_duty(LEDC_LOW_SPEED_MODE, this->_chanels[(pwmCh==0)?1:0], 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, this->_chanels[(pwmCh==0)?1:0]);
}
void MotorController::encoderInteruptHandler(void* _this)
{
    MotorController *t = (MotorController*) _this;
    t->_encoderTicks++;
    t->_absoluteEncoderTicks++;
}
void MotorController::clearEncoderTimer(TimerHandle_t xTimer)
{
    MotorController* t = (MotorController*) pvTimerGetTimerID(xTimer);
    float omega = (float)(2 * M_PI * t->_encoderTicks) / (float)(TICKS_PER_ROTATION * ((float)(TIMER_CLEAR)/1000)); 
    
    // czyszczenie licznika enkodera
    t->clearEncoder();
    
    if(t->_engineRadialSpeed < 0) omega*=-1; // jezeli nastawa na - to silnik obraca sie przeciwnie
    t->_currentRadialEngineSpeed = omega;

    // aktulizacja nastaw silnika
    t->update();

}
void MotorController::clearEncoder()
{
    this->_encoderTicks = 0;
}
void MotorController::setEngineRadialSpeed(float speed)
{
    this->_engineRadialSpeed = speed;
}
void MotorController::setWheelRadialSpeed(float speed)
{
    this->_engineRadialSpeed = speed/MotorController::GEAR_RATIO;
}
int MotorController::getEncoder() const 
{
    return this->_encoderTicks;
}
int MotorController::getAbsoluteEncoder() const 
{
    return this->_absoluteEncoderTicks;
}
float MotorController::getEngineRadialSpeed() const
{
    return this->_engineRadialSpeed;
}
float MotorController::getCalculatedEngineRadialSpeed() const
{
    // rad / s
    return this->_currentRadialEngineSpeed;
}
float MotorController::getWheelRadialSpeed() const
{
    // cm / s
    return this->_currentRadialEngineSpeed / MotorController::GEAR_RATIO;
}
float MotorController::getCalculatedWheelRadialSpeed() const
{
    // cm / s
    return this->_currentRadialEngineSpeed / MotorController::GEAR_RATIO;
}
float MotorController::getCurrentLinearDistance() const
{
    return (float)this->_currentRadialEngineSpeed / ((float)TIMER_CLEAR * 0.001);
}
float MotorController::getAbsoluteLinearDistance() const
{
    return ((float)this->_absoluteEncoderTicks / (float)MotorController::TICKS_PER_ROTATION_LARGE) * MotorController::TIRE_RADIUS;
}