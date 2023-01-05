#include <./lib/structs.cpp>
#ifndef MACHINE_H
#define MACHINE_H

#include "FastAccelStepper.h"
#include <Preferences.h>

extern QueueHandle_t qCommands, qEvents;
extern Preferences preferences;
void _setDefaultConfig();
void _initPins();
void _ActionProcessor(void *pvParameter);
void ActionMoveVertToHome();
void _SetActiveMotor(MOTOR Motor);
void MachineStart();
void ActionProcessor(msgCommand Command);
void ActionEmergencyStop();
void ActionInitialise();
void ActionSetSpinDirection(SPINDIRECTION SpinDirection);
void ActionSetSpinSpeedRPM(int SpeedRPM);
void ActionSetAltSpinDurationMs(int DurationMs);
void ActionMoveVerticalTo(VERTICALPOSITION VerticalPosition);
void ActionSpin(int Duration);
void ActionMoveByStep(MOTOR Motor, int Steps);
void SaveCurrentMotorCurrentStep();

MACHINESTATE getState();

// TODO:Complete Actions
MACHINECONFIG getConfig();
// Calibration VerticalPosition
void ConfigVerticalPositions(int Top, int Middle, int Bottom);
void ConfigHorizontalPositions(int Wash, int Rinse, int FinalRinse, int Dryer);
void setSpinAccelleration(int Accelleration);
int getSpinSpinAccelleration();
String GetMotorName(MOTOR motor);
String GetVerticalName(VERTICALPOSITION position);
String GetHorizontalName(HORIZONTALPOSITION position);


#endif