#include <Arduino.h>
#include "esp_log.h"
#include <./lib/structs.cpp>

#include <./lib/machine.H>

#pragma region Constructors


MACHINECONFIG _config;
MACHINESTATE _state;
FastAccelStepper *_stepper;
FastAccelStepperEngine _engine = FastAccelStepperEngine();
SPINDIRECTION _spinSelectedDirection = SPINDIRECTION::clockwise;
TaskHandle_t _taskActionProcessor = NULL;




void MachineStart()
{
    _setDefaultConfig();
    _initPins();
    _engine.init();
    ESP_LOGI("Machine","Step Pin:%d",_config.pinStep);
    _stepper = _engine.stepperConnectToPin(_config.pinStep);
    Serial.println("Set Config");
    xTaskCreate(_ActionProcessor,"Action Processor",4096,NULL,10,NULL);

}

#pragma endregion

#pragma region PublicMethods
void _ActionProcessor(void *pvParameter)
{
    struct msgCommand msg;
    for (;;)
    {
         if (qCommands !=0){
             if (xQueueReceive(qCommands, &msg,(TickType_t)10)){
                 ESP_LOGI ("Async Action","Command: %d, Value: %d",msg.action,msg.value);
                 switch (msg.action)
                 {
                 case ACTIONS::Initialise:
                    ActionInitialise();
                    break;
                 case ACTIONS::VerticalMoveByStep:
                    ActionMoveByStep(MOTOR::vertical,msg.value);
                    break;
                 case ACTIONS::HoricontalMoveByStep:
                    ActionMoveByStep(MOTOR::horizontal,msg.value);
                    break;
                 case ACTIONS::ResetConfig:
                    preferences.clear();
                 default:
                    break;
                 }
             };
         };
        vTaskDelay(100);
    };
}

    void ActionProcessor(msgCommand Command){
        xQueueSend(qCommands,&Command,10);
    }


void ActionInitialise()
{
    ActionMoveVertToHome();
};

/// @brief /////////////////////
// Move vertical to home home position via limit switch.
////////////////////////////////
void ActionMoveVertToHome()
{
    ESP_LOGI("Machine","Running VertToHome");
    _state.busy = true;
    _SetActiveMotor(MOTOR::vertical);
    _stepper->setSpeedInHz(3000);
    _stepper->setAcceleration(3000);

    // If Top Limit is already pressed move down
    ESP_LOGI("Machine","TopLimit:%d",digitalRead(_config.pinTopLimitSwitch));
    if (digitalRead(_config.pinTopLimitSwitch) == LOW)
    {
        _stepper->move(200);
    }
    // Wait till done
    while (_stepper->isRunning())
    {
        delay(100);
    }

    unsigned long startMs = millis();
    bool timeout = false;
    _stepper->runBackward();
    while (digitalRead(_config.pinTopLimitSwitch) == HIGH and timeout == false)
    {
        if (millis() - startMs > _config.verticalLimitTimeoutMs)
        {
            timeout = true;
            ESP_LOGE("Machine","Home timed out");
        }
        delay(10);
    }

    _stepper->forceStopAndNewPosition(0);
    _state.verticalPosition = VERTICALPOSITION::home;
    _state.verticalTargetPosition = VERTICALPOSITION::home;
    _state.busy = false;
};

void ActionMoveByStep(MOTOR Motor, int Steps){
    ESP_LOGI("Machine","Move by: %d",Steps);
    _state.busy = true;
    _SetActiveMotor(Motor);
    _stepper->move(Steps,true);
    SaveCurrentMotorCurrentStep();
    _state.busy = false;
}
void ActionSetSpinDirection(SPINDIRECTION SpinDirection) { _state.spinDirectiom = SpinDirection; };
void ActionSetSpinSpeedRPM(int SpinSpeedRPM) { _state.spinSpeedRPM = SpinSpeedRPM; };
void ActionSetAltSpinDurationMs(int DurationMs) { _state.spinReverseTimeMs = DurationMs; };
void ActionSpin(int Duration)
{
    _SetActiveMotor(MOTOR::spin);
    _stepper->setSpeedInHz(_state.spinSpeedRPM * _config.RPMtoStepsRatio);
    _stepper->setAcceleration(_config.spinAccell);
    int start = millis();
    while (millis() - start <= Duration)
    {
        switch (_spinSelectedDirection)
        {
        case SPINDIRECTION::bothwise:
            _stepper->runForward();
            delay(_state.spinReverseTimeMs);
            _stepper->runBackward();
            delay(_state.spinReverseTimeMs);
            break;
        case SPINDIRECTION::clockwise:
            _stepper->runForward();
            delay(Duration);
        case SPINDIRECTION::anticlockwise:
            _stepper->runBackward();
            delay(Duration);

        default:
            break;
        }
    }
    _stepper->stopMove();
    while (_stepper->isRunning())
    {
        delay(50);
    }
};

#pragma endregion

#pragma region SetterGetters
MACHINESTATE getState()
{
    return _state;
}

MACHINECONFIG getConfig()
{
    return _config;
}

String GetMotorName(MOTOR motor)
{
    // Text name of motor
    switch (motor)
    {
    case MOTOR::none:
        return "None";
        break;
    case MOTOR::spin:
        return "Spin";
        break;
    case MOTOR::horizontal:
        return "Horizontal";
        break;
    case MOTOR::vertical:
        return "Vertical";
        break;
    default:
        return "Unknown";
        break;
    }
};
String GetVerticalName(VERTICALPOSITION position)
{
    switch (position)
    {
    case VERTICALPOSITION::home:
        return "Home";
        break;
    case VERTICALPOSITION::middle:
        return "Middle";
        break;
    case VERTICALPOSITION::bottom:
        return "Bottom";
        break;
    case VERTICALPOSITION::top:
        return "Top";
        break;
    case VERTICALPOSITION::unknown:
        return "Unknown";
        break;
    default:
        return "Error";
        break;
    }
};
String GetHorizontalName(HORIZONTALPOSITION position)
{
    switch (position)
    {
    case HORIZONTALPOSITION::home:
        return "Home";
        break;
    case HORIZONTALPOSITION::wash:
        return "Wash";
        break;
    case HORIZONTALPOSITION::rinse:
        return "Rinse";
        break;
    case HORIZONTALPOSITION::finalRinse:
        return "Final Rinse";
        break;
    case HORIZONTALPOSITION::dryer:
        return "Dryer";
        break;
    case HORIZONTALPOSITION::unknown:
        return "Unknown";
        break;
    default:
        return "Error";
        break;
    }
};

void setSpinAccelleration(int Accelleration)
{
    _config.spinAccell = Accelleration;
}
int getSpinSpinAccelleration()
{
    return _config.spinAccell;
}

///////////////////////////////////////
// Configure the positions for vertical
//////////////
void ConfigVerticalPositions(int Top, int Middle, int Bottom)
{
    _config.verticalTopStepValue = Top;
    _config.verticalMidStepValue = Middle;
    _config.verticalBottomStepValue = Bottom;
}
void ConfigHorizontalPositions(int Wash, int Rinse, int FinalRinse, int Dryer)
{
    _config.horizontalWashStepValue = Wash;
    _config.horizontalRinseStepValue = Rinse;
    _config.horizontalFinalRinseStepValue = FinalRinse;
    _config.horizontalDryerStepValue = Dryer;
}
#pragma endregion

#pragma region PrivateMethods

void SaveCurrentMotorCurrentStep(){
        //Save position of current Motor
    switch (_state.selectedMotor)
    {
    case MOTOR::horizontal:
        ESP_LOGI("Machine","Set H Position: %d",_stepper->getCurrentPosition());
        _state.horizontalCurrentStep = _stepper->getCurrentPosition();
        break;
    case MOTOR::vertical:
        ESP_LOGI("Machine","Set V Position: %d",_stepper->getCurrentPosition());
        _state.verticalCurrentStep = _stepper->getCurrentPosition();
        break;
    default:
        break;
    }
}

void _SetActiveMotor(MOTOR Motor)
{
    // Stop current motor if moving
    // TODO: Will cause current position to be incorrect.
    _stepper->stopMove();
    
    while (_stepper->isRunning())
    {
        delay(50);
    }
    SaveCurrentMotorCurrentStep();


    switch (Motor)
    {
    case MOTOR::spin:
        _stepper->setDirectionPin(_config.pinDirection);
        _stepper->setEnablePin(_config.pinMotorSpinEnable);
        _stepper->setCurrentPosition(0);
        _state.selectedMotor = Motor;

        break;
    case MOTOR::vertical:
        _stepper->setDirectionPin(_config.pinDirection, false);
        _stepper->setEnablePin(_config.pinMotorVerticalEnable);
        _stepper->setCurrentPosition(_state.verticalCurrentStep);
        _stepper->setSpeedInHz(_config.verticalSpeed);
        _stepper->setAcceleration(_config.verticalAccell);
        _state.selectedMotor = Motor;
        break;
    case MOTOR::horizontal:
        _stepper->setDirectionPin(_config.pinDirection, false);
        _stepper->setEnablePin(_config.pinMotorHorzontalEnable);
        _state.selectedMotor = Motor;
        _stepper->setSpeedInHz(_config.horizontalSpeed);
        _stepper->setAcceleration(_config.horizontalAccell);
        _stepper->setCurrentPosition(_state.horizontalCurrentStep);
        break;

    default:
        break;
    }
    _stepper->setAutoEnable(true);
};

void ActionMoveVerticalTo(VERTICALPOSITION VerticalPosition)
{
    int Position;
    _SetActiveMotor(MOTOR::vertical);

    _state.verticalTargetPosition = VerticalPosition;
    _stepper->setAcceleration(_config.verticalAccell);
    _stepper->setSpeedInHz(_state.spinSpeedRPM * _config.RPMtoStepsRatio);

    if (VerticalPosition == VERTICALPOSITION::home || _state.verticalPosition == VERTICALPOSITION::unknown)
    {
        ActionMoveVertToHome();
    }

    switch (VerticalPosition)
    {
    case VERTICALPOSITION::top:
        Position = _config.verticalTopStepValue;
        break;
    case VERTICALPOSITION::middle:
        Position = _config.verticalMidStepValue;
        break;
    case VERTICALPOSITION::bottom:
        Position = _config.verticalBottomStepValue;
        break;
    default:
        break;
    }

    _stepper->moveTo(Position);
    while (_stepper->isRunning())
    {
        delay(50);
    }
    _state.verticalPosition = VerticalPosition;
    Serial.print("Final Position:");
    Serial.println(Position);
};

void _initPins()
{
    // Setup Pins
    pinMode(_config.pinMotorSpinEnable, OUTPUT);
    digitalWrite(_config.pinMotorSpinEnable, HIGH);

    pinMode(_config.pinMotorVerticalEnable, OUTPUT);
    digitalWrite(_config.pinMotorVerticalEnable, HIGH);

    pinMode(_config.pinStep, OUTPUT);
    pinMode(_config.pinDirection, OUTPUT);
    pinMode(_config.pinTopLimitSwitch, INPUT_PULLUP);
}
void _setDefaultConfig()
{

    _config.pinStep = preferences.getInt("pinStep", 32);
    _config.pinDirection = preferences.getInt("pinDirection", 13); //need to chand
    _config.pinMotorSpinEnable = preferences.getInt("pinMotorSpinEnable", 27);
    _config.pinMotorVerticalEnable =preferences.getInt("pinMotorVerticalEnable", 26);
    _config.pinMotorHorzontalEnable = preferences.getInt("pinMotorHorzontalEnable", 33);
    _config.pinTopLimitSwitch = preferences.getInt("pinTopLimitSwitch", 25);
    _config.pinLeftLimitSwitch = preferences.getInt("pinLeftLimitSwitch", 12); //need to chand
    _config.pinDryer = preferences.getInt("pinDryer", 14); //need to chand
    _config.pin24vOn = preferences.getInt("pin24vOn", 35);

    _config.spinAccell = preferences.getInt("spinAccell", 3000); // In Hz/s/s
    _config.RPMtoStepsRatio = preferences.getInt("RPMtoStepsRatio", 27);

    _config.verticalTopStepValue = preferences.getInt("verticalTopStepValue",2000);
    _config.verticalMidStepValue = preferences.getInt("verticalMidStepValue",20000);
    _config.verticalBottomStepValue = preferences.getInt("verticalBottomStepValue",40000);
    _config.verticalSpeed = preferences.getInt("verticalSpeed",8000);
    _config.verticalAccell = preferences.getInt("verticalAccell",8000);
    _config.verticalLimitTimeoutMs=preferences.getInt("verticalLimitTimeoutMs",30000);

    _config.horizontalWashStepValue = preferences.getInt("horizontalWashStepValue",0);
    _config.horizontalRinseStepValue = preferences.getInt("horizontalRinseStepValue",0);
    _config.horizontalFinalRinseStepValue = preferences.getInt("horizontalFinalRinseStepValue",0);
    _config.horizontalDryerStepValue = preferences.getInt("horizontalDryerStepValue",0);
    _config.horizontalSpeed = preferences.getInt("horizontalSpeed",5000);
    _config.horizontalAccell = preferences.getInt("horizontalAccell",5000);
}

#pragma endregion