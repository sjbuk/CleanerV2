#ifndef STRUCTS
#define STRUCTS
#include <./lib/structs.cpp>
#endif

#ifndef MACHINE_H
#define MACHINE_H

#include "FastAccelStepper.h"

class Machine
{
private:
    config _config;
    directions _spinSelectedDirection = clockwise;
    int _spinSelectedSpeed = 0;
    int _spinSelectedSpindBothDuration = 0;
    int _verticalPoistion = 0;
    void _setDefaultConfig();
    bool _pause = false;
    bool _stop = true;
    bool _emergencyStop = false;
    void _initPins();
    void _MoveVerticalToPosition (int Position);
    void _SetActiveMotor (motor Motor);    
    FastAccelStepperEngine _engine = FastAccelStepperEngine();
    FastAccelStepper *_stepper = NULL;

public:
    Machine();
    void Action00EmergencyStop();
    void Action05Initialise();
    void Action10SetSpinClockwise();
    void Action11SetSpinAntiClockwise();
    void Action12SetSpinBothwise();
    void Action13SetSpinSpeedTo1();
    void Action14SetSpinSpeedTo2();
    void Action15SetSpinSpeedTo3();
    void Action16SetSpinSpeedTo4();
    void Action17SetAltSpinDurationTo1();
    void Action18SetAltSpinDurationTo2();
    void Action19SetAltSpinDurationTo3();
    void Action20SetAltSpinDurationTo4();
    void Action30MoveVertToHome();
    void Action31MoveVertToTop();
    void Action32MoveVertToMid();
    void Action33MoveVertToBottom();
    void ActionSpin(int Duration);

    // TODO:Complete Actions
    config getConfig();
    void setSpeeds(int Speed0, int Speed1, int Speed2, int Speed3);
    void setVerticalPositions(int Top, int Middle, int Bottom);
    void setHorizontalPositions(int Wash, int Rinse, int FinalRinse, int Dryer);
    void setSpinAccelleration(int Accelleration);
    int getSpinSpinAccelleration();
};

#endif