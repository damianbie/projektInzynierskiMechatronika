#include "System.hpp"
#include <freertos/timers.h>
#include <driver/gpio.h>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include "esp_log.h"

System::System()
    :_robotMode(RobotMode::ROBOT_MANUAL), _leftMotor(nullptr), _rightMotor(nullptr)
{
    _maxWheelSpeed = 6;
    this->_robotMode = RobotMode::ROBOT_MANUAL;
    this->_manRobotDir = RobotDirection::NONE;
}
System& System::getInstance()
{
    static System _instance;
    return _instance;
}

void System::initFileSystem()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
        }
    ESP_ERROR_CHECK(ret);
}
void System::initEngines()
{
    this->_leftMotor = new MotorController(
        gpio_num_t::GPIO_NUM_22, gpio_num_t::GPIO_NUM_21,
        gpio_num_t::GPIO_NUM_23, 
        LEDC_CHANNEL_0, LEDC_CHANNEL_1);

    this->_rightMotor = new MotorController(
        gpio_num_t::GPIO_NUM_18, gpio_num_t::GPIO_NUM_19,
        gpio_num_t::GPIO_NUM_4, 
        LEDC_CHANNEL_2, LEDC_CHANNEL_3);
}

void System::initLedCDriver()
{
    ledc_timer_config_t ledc_timer;
    ledc_timer.speed_mode       = LEDC_LOW_SPEED_MODE,
    ledc_timer.timer_num        = LEDC_TIMER_0,
    ledc_timer.duty_resolution  = LEDC_TIMER_13_BIT ,
    ledc_timer.freq_hz          = 5000,  // Set output frequency at 5 kHz
    ledc_timer.clk_cfg          = LEDC_AUTO_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
}
MotorController* System::getLeftEngine()
{
    return this->_leftMotor;
}
MotorController* System::getRightEngine()
{
    return this->_rightMotor;
}
std::string System::runCommand(returnCommandType cmd, std::shared_ptr<con> cl, server& serv)
{
    std::string result;
    auto& args = cmd.second;
   
    if(this->_robotMode == RobotMode::ROBOT_AUTO)
    {
        result = this->_autoModeCommands(cmd, cl, serv);
    }
    else if(this->_robotMode == RobotMode::ROBOT_MANUAL)
    {
        result = this->_manualModeCommands(cmd, cl, serv);
    }
    
    if(!result.length())
        result = this->_systemCommands(cmd, cl, serv);

    return result;
}
std::string System::_systemCommands(returnCommandType cmd, std::shared_ptr<con> cl, server& serv)
{
    std::string result = "ok";
    auto& args = cmd.second;

    if(!cmd.first.compare("close"))
    {
        serv.close(cl);
    }
    else if(!cmd.first.compare("ping"))
        result = "pong";
    else if(!cmd.first.compare("getRadialEngineSpeed"))
    {
        char buff[1024];
        float s1 = System::getInstance().getLeftEngine()->getCalculatedEngineRadialSpeed();
        float s2 = System::getInstance().getRightEngine()->getCalculatedEngineRadialSpeed();
        sprintf(buff, "%f %f", s1, s2);
        result = buff;
    }
    else if(!cmd.first.compare("setMode") && args.size())
    {
        if(!args[0].compare("manual"))
        {
            this->_robotMode = RobotMode::ROBOT_MANUAL;
        }
        else if(!args[0].compare("auto"))
        {
            this->_robotMode = RobotMode::ROBOT_AUTO;
        }
    }
    else if(!cmd.first.compare("setWheelRadialSpeed") && args.size())
    {
        this->_maxWheelSpeed = atof(args[0].c_str());
    }
    else 
        result = "Unknow command";

    return result;
}
std::string System::_manualModeCommands(returnCommandType cmd, std::shared_ptr<con> cl, server& serv)
{
    std::string result;
    auto& args = cmd.second;

    if(!cmd.first.compare("forward"))
    {
        this->_manRobotDir = RobotDirection::DIR_FORWARD;
        result = "ok";
    }
    else if(!cmd.first.compare("backwards"))
    {
        this->_manRobotDir = RobotDirection::DIR_BACKWARD;
        result = "ok";
    }
    else if(!cmd.first.compare("turnLeft"))
    {
        this->_manRobotDir = RobotDirection::DIR_LEFT;
        result = "ok";
    }
    else if(!cmd.first.compare("turnRight"))
    {
        this->_manRobotDir = RobotDirection::DIR_RIGHT;
        result = "ok";
    }
    else if(!cmd.first.compare("noMovment"))
    {
        this->_manRobotDir = RobotDirection::NONE;
        result = "ok";
    }
    return result;
}
std::string System::_autoModeCommands(returnCommandType cmd, std::shared_ptr<con> cl, server& serv)
{
    std::string result;
    auto& args = cmd.second;
    robotTask task;

    if(!cmd.first.compare("addForwardPath") && args.size())
    {
        task.dir = RobotDirection::DIR_FORWARD;


        this->addTaskToQueue(task);
        result = "ok";
    }
    else if(!cmd.first.compare("addBackwardPath") && args.size())
    {
        task.dir = RobotDirection::DIR_BACKWARD;

        this->addTaskToQueue(task);
        result = "ok";
    }
    else if(!cmd.first.compare("addTurnLeftPath") && args.size())
    {
        task.dir = RobotDirection::DIR_LEFT;

        this->addTaskToQueue(task);
        result = "ok";
    }
    else if(!cmd.first.compare("addTurnRightPath") && args.size())
    {
        task.dir = RobotDirection::DIR_RIGHT;

        this->addTaskToQueue(task);
        result = "ok";
    }
    return result;
}

RobotMode System::getRobotMode()
{
    return this->_robotMode;
}


void System::robotForward()
{
    this->_leftMotor->setWheelRadialSpeed(0.8 * _maxWheelSpeed);
    this->_rightMotor->setWheelRadialSpeed(0.8 * _maxWheelSpeed);
}
void System::robotBackward()
{
    this->_leftMotor->setWheelRadialSpeed(-0.8 * _maxWheelSpeed);
    this->_rightMotor->setWheelRadialSpeed(-0.8 * _maxWheelSpeed);
}
void System::robotStop()
{
    this->_leftMotor->setWheelRadialSpeed(0);
    this->_rightMotor->setWheelRadialSpeed(0);
}
void System::robotRotateLeft()
{
    this->_leftMotor->setWheelRadialSpeed(0.8 * _maxWheelSpeed);
    this->_rightMotor->setWheelRadialSpeed(-0.8 * _maxWheelSpeed);
}  
void System::robotRotateRight()
{
    this->_leftMotor->setWheelRadialSpeed(-0.8 * _maxWheelSpeed);
    this->_rightMotor->setWheelRadialSpeed(0.8 * _maxWheelSpeed);
}
void System::addTaskToQueue(robotTask task)
{
    this->_robotTasks.push(task);
}
void System::processAutoMode()
{
    static PIDController controller(1, 0.01, 0.01);
    static float leftSpeed = 0;
    static float rightSpeed = 0;

    float distanceLeft  = _leftMotor->getCurrentLinearDistance();
    float distanceRight = _rightMotor->getCurrentLinearDistance();



    // leftSpeed += controller.calcOutput(distanceLeft, 100);
    // rightSpeed += controller.calcOutput(distanceright, 100);


}