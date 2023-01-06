#include "PIDController.hpp"


PIDController::PIDController()
    :PIDController(0, 0, 0)
{}
PIDController::PIDController(float p, float i, float d, int timeStep)
{
    this->setP(p);
    this->setI(i);
    this->setD(d);
    this->setTimeStep(timeStep);
}

float PIDController::calcOutput(float current, float set)
{
    float error = set - current;
    float der = (error - this->_lastValue)/(_timeStep* 0.001);
 
    float output =  (this->_p * error) + 
                    (this->_i * error * this->_timeStep * 0.001) + 
                    (this->_d * der);

    this->_lastValue = error;
    return output;
}


void PIDController::setP(float p)
{
    this->_p = p;
}
void PIDController::setI(float i)
{
    this->_i = i;
}
void PIDController::setD(float d)
{
    this->_d = d;
}
void PIDController::setTimeStep(int t)
{
    this->_timeStep = t;
}
float PIDController::getP() const
{
    return this->_p;
}
float PIDController::getD() const
{
    return this->_d;
}
float PIDController::getI() const
{
    return this->_i;
}
int PIDController::getTimeStep() const
{
    return this->_timeStep;
}
