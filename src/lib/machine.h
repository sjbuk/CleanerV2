#include <./lib/structs.cpp>
#ifndef MACHINE_H
#define MACHINE_H

#include "FastAccelStepper.h"

class Machine
{
private:
    MACHINECONFIG _config;
    MACHINESTATE _state;
    SPINDIRECTION _spinSelectedDirection = SPINDIRECTION::clockwise;
    void _setDefaultConfig();
    bool _pause = false;
    bool _stop = true;
    bool _emergencyStop = false;
    void _initPins();
    void ActionMoveVertToHome();
    void _SetActiveMotor (MOTOR Motor);    
    FastAccelStepperEngine _engine = FastAccelStepperEngine();
    FastAccelStepper *_stepper = NULL;

public:
    Machine();
    void ActionEmergencyStop();
    void ActionInitialise();
    void ActionSetSpinDirection(SPINDIRECTION SpinDirection);
    void ActionSetSpinSpeedRPM(int SpeedRPM);
    void ActionSetAltSpinDurationMs(int DurationMs);
    void ActionMoveVerticalTo(VERTICALPOSITION VerticalPosition);
    void ActionSpin(int Duration);
    MACHINESTATE getState ();

    // TODO:Complete Actions
    MACHINECONFIG getConfig();
    //Calibration VerticalPosition
    void ConfigVerticalPositions(int Top, int Middle, int Bottom);
    void ConfigHorizontalPositions(int Wash, int Rinse, int FinalRinse, int Dryer);
    void setSpinAccelleration(int Accelleration);
    int getSpinSpinAccelleration();
    String GetMotorName (MOTOR motor);
    String GetVerticalName (VERTICALPOSITION position);
    String GetHorizontalName (HORIZONTALPOSITION position);

};

#endif