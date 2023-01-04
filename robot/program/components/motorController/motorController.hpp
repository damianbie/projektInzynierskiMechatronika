#include <iostream>
#include <stdio.h>
#include <cstring>
#include <math.h>

#include <freertos/freertos.h>
#include <freertos/timers.h>
#include <driver/gpio.h>
#include "driver/ledc.h"
#include "esp_log.h"

#include "PIDController.hpp"

/*
    units:
        for ratations   -> rad / s
        for linera      -> cm / s

*/
class MotorController :protected PIDController
{
public:
    MotorController(gpio_num_t pinIn1, gpio_num_t pinIn2,
    gpio_num_t encoerPin, ledc_channel_t ledcChanel1, 
    ledc_channel_t ledcChanel2);

    ~MotorController();

    void clearEncoder();
    void setEngineRadialSpeed(float speed);
    void setWheelRadialSpeed(float speed);

    int getEncoder()                        const;
    int getAbsoluteEncoder()                const;

    float getEngineRadialSpeed()            const;
    float getCalculatedEngineRadialSpeed()  const;

    float getWheelRadialSpeed()             const;
    float getCalculatedWheelRadialSpeed()   const;

    float getCurrentLinearDistance()        const;
    float getAbsoluteLinearDistance()       const;

    static void encoderInteruptHandler(void*);
    static void clearEncoderTimer(TimerHandle_t);

    static const int MIN_PWM_VAL                = 0;
    static const int MAX_PWM_VAL                = 8192;

    static const int TIMER_CLEAR                = 100;       //ms
    static const int TICKS_PER_ROTATION         = 14;       
    static const int TICKS_PER_ROTATION_LARGE   = 700;
    static constexpr float TIRE_RADIUS         = (6.4/2);   // cm
    static constexpr float GEAR_RATIO          = 0.02;
protected:
    void update();

    int _encoderTicks, _absoluteEncoderTicks;
    float _engineRadialSpeed, _currentRadialEngineSpeed;
    
    TimerHandle_t _updater;
    ledc_channel_t _chanels[2];
};