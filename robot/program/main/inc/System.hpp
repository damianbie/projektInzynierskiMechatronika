#ifndef _H_SYSTEM_H_
#define _H_SYSTEM_H_

#include "esp_system.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <FreeRTOS/FreeRTOS.h>
#include <queue>

#include "motorController.hpp"
#include "server.hpp"

enum RobotState 
{
    ROBOT_WAITING_FOR_PATH = 0,
    ROGOT_HAS_FULL_PATH,
    ROBOT_RUNNING_ON_PATH,
    ROBOT_END_PATH,
    ROGOT_ERROR
};
enum RobotMode
{
    ROBOT_MANUAL,
    ROBOT_AUTO
};
enum RobotDirection
{
    NONE,
    DIR_FORWARD,
    DIR_BACKWARD,
    DIR_RIGHT,
    DIR_LEFT
};
struct robotTask
{
    RobotDirection dir;
    int32_t absoluteValue;
};
//singleton
class System
{
public:
    static System& getInstance();

    void initFileSystem();
    void initEngines();
    void initLedCDriver();

    void robotStop();
    void robotForward();
    void robotBackward();
    void robotRotateLeft();
    void robotRotateRight();


    void processAutoMode();
    void addTaskToQueue(robotTask);

    std::string runCommand(returnCommandType cmd, std::shared_ptr<con> cl, server& serv);

    MotorController* getLeftEngine();
    MotorController* getRightEngine();

    RobotMode getRobotMode();

    RobotDirection _manRobotDir;

private:
    System();

    std::string _systemCommands(returnCommandType cmd, std::shared_ptr<con> cl, server& serv);
    std::string _manualModeCommands(returnCommandType cmd, std::shared_ptr<con> cl, server& serv);
    std::string _autoModeCommands(returnCommandType cmd, std::shared_ptr<con> cl, server& serv);


    RobotMode _robotMode;
    MotorController* _leftMotor;
    MotorController* _rightMotor;


    float _maxWheelSpeed;


    std::queue<robotTask> _robotTasks;
};
#endif