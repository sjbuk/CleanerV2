#ifndef STRUCTS
#define STRUCTS

enum class SPINDIRECTION
{
    clockwise = 1,
    anticlockwise = 2,
    bothwise = 3
};
enum class MOTOR
{
    none = 0,
    spin = 1,
    vertical = 2,
    horizontal = 3
};

enum class HORIZONTALPOSITION{
    unknown = -1,
    home = 0,
    wash = 1,
    rinse = 2,
    finalRinse = 3,
    dryer = 4
};
enum class VERTICALPOSITION
{
    unknown = -1,
    home = 0,
    top = 1,
    middle = 2,
    bottom = 3
};
enum class ACTIONS{
    EmergencyStop = 0,              //No value
    StopMotors = 1,                 //No value
    Pause = 2,                      //No value need Continue to restart
    Continue = 3,                   //No Value
    Clear = 4,                      //No Value
    StartSpinMotor = 5,             //Vale = duration in ms
    StartVerticalMotor = 6,         //Value = Destination VERTICALPOSITION
    StartHorizontalMotor = 7,       //Value = Destination HORIZONTALPOSITION
    SetSpinDirection = 8,           //Value = SPINDIRECTION
    SetSpinSpeedRPM = 9,            //Value = RPM
    DryerOn = 10,                   //Value = Duration in MS
    DryerOff = 11,                  //NoValue
    VerticalStep = 12,              //Vaue +- Steps to move.
    HoricontalStep = 13,             //Vaue +- Steps to move.
    Initialise = 14
};

struct LOGFILTER{
    bool Errors = true;
    bool Warnings = true;
    bool Information = true;
    bool Debug = false;
    bool Verbose = false;
};

enum EVENTS{
    StateUpdated = 1
};

struct msgCommand{
    ACTIONS action;
    int value;
};


struct MACHINESTATE
{
    MOTOR selectedMotor = MOTOR::none;
    bool motorRunning = false;
    VERTICALPOSITION verticalPosition = VERTICALPOSITION::unknown;
    VERTICALPOSITION verticalTargetPosition = VERTICALPOSITION::unknown;
    int verticalCurrentStep;
    HORIZONTALPOSITION horizontalPosition = HORIZONTALPOSITION::unknown;
    HORIZONTALPOSITION horizontalTargetPosition = HORIZONTALPOSITION::unknown;
    int horizontalCurrentStep;

    int spinSpeedRPM;          // spinSpeeds[0] default
    int spinReverseTimeMs; // If Spin both directions then how long in ms for each direction
    SPINDIRECTION spinDirectiom;

};


struct MACHINECONFIG
{
    int RPMtoStepsRatio;

    int verticalTopStepValue;    // Step position of top.
    int verticalMidStepValue;    // Step position of Mid.
    int verticalBottomStepValue; // Step position of Bottom.
    int verticalSpeed;
    int verticalAccell;
    unsigned long verticalLimitTimeoutMs;

    int horizontalWashStepValue;       // Step position of Wash Jar.
    int horizontalRinseStepValue;      // Step position Rinse Jar.
    int horizontalFinalRinseStepValue; // Step position Final Rinse Jar.
    int horizontalDryerStepValue;      // Step position Dryer.
    int horizontalSpeed;
    int horizontalAccell;

    int pinStep;
    int pinDirection;
    int pinMotorSpinEnable;
    int pinMotorVerticalEnable;
    int pinMotorHorzontalEnable;
    int spinAccell;

    int pinTopLimitSwitch;
    int pinLeftLimitSwitch;
    int pinDryer;
    int pin24vOn;
};
#endif
