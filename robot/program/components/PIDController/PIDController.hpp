#ifndef _H_PIDCONTROLLER_
#define _H_PIDCONTROLLER_


class PIDController
{
public:
    PIDController();
    PIDController(float p, float i, float d, int timeStep = 0);

    float calcOutput(float current, float set);

    void setP(float p);
    void setI(float i);
    void setD(float d);
    void setTimeStep(int t);

    float getP() const;
    float getI() const;
    float getD() const;
    int getTimeStep() const;

private:
    float _p, _i, _d;
    float _lastValue;
    int _timeStep;
};
#endif