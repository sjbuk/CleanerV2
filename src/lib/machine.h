#ifndef STRUCTS
#define STRUCTS
#include <./lib/structs.cpp>
#endif

#ifndef MACHINE_H
#define MACHINE_H

class Machine{
private:
    config _config;
    directions _spinSelectedDirection = clockwise;
    int _spinSelectedSpeed = 0;
    int _spinSelectedSpindBothDuration = 0;
    void _setDefaultConfig();
    bool _pause = false;
    bool _stop = true;
    bool _emergencyStop = false;
    void _initPins ();

    

public:
    Machine (unsigned int actions[100]);
    Machine ();
    void Action00EmergencyStop();
    void Action10SetSpinClockwise();
    void Action11SetSpinAntiClockwise();
    void Action12SetSpinBothwise();

    config getConfig();
    void setSpeeds (int Speed0, int Speed1, int Speed2, int Speed3);
    void setVerticalPositions (int Top, int Middle, int Bottom);
    void setHorizontalPositions (int Wash, int Rinse, int FinalRinse, int Dryer);
    void setSpinAccelleration(int Accelleration);
    int getSpinSpinAccelleration();
};

#endif