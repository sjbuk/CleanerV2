#include <Arduino.h>

#include <./lib/structs.cpp>

#include <./lib/machine.H>
#include <./config/pindefaults.h>

#pragma region Constructors

Machine::Machine()
{
    _engine.init();
    _setDefaultConfig();
    Serial.println("Set Config");
    _initPins();
    _stepper = _engine.stepperConnectToPin(_config.pinStep);

    Serial.println("Move to home");
}

#pragma endregion

#pragma region PublicMethods

void Machine::ActionInitialise()
{
    ActionMoveVertToHome();
};

/// @brief /////////////////////
// Move vertical to home home position via limit switch.
////////////////////////////////
void Machine::ActionMoveVertToHome()
{
    _SetActiveMotor(MOTOR::vertical);
    _stepper->setSpeedInHz(3000);
    _stepper->setAcceleration(50000);

    // If Top Limit is already pressed move down
    Serial.print("TopLimit:");
    Serial.println(digitalRead(_config.pinTopLimitSwitch));
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
            Serial.printf("Home timed out");
        }
        delay(10);
    }

    _stepper->forceStopAndNewPosition(0);
    _state.verticalPosition = VERTICALPOSITION::home;
};

void Machine::ActionSetSpinDirection(SPINDIRECTION SpinDirection) { _state.spinDirectiom = SpinDirection; };
void Machine::ActionSetSpinSpeedRPM(int SpinSpeedRPM) { _state.spinSpeedRPM = SpinSpeedRPM; };
void Machine::ActionSetAltSpinDurationMs(int DurationMs) { _state.spinReverseTimeMs = DurationMs; };
void Machine::ActionSpin(int Duration)
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
MACHINESTATE Machine::getState()
{
    return _state;
}

MACHINECONFIG Machine::getConfig()
{
    return _config;
}

String Machine::GetMotorName(MOTOR motor)
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
String Machine::GetVerticalName(VERTICALPOSITION position){
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
String Machine::GetHorizontalName(HORIZONTALPOSITION position){
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

void Machine::setSpinAccelleration(int Accelleration)
{
    _config.spinAccell = Accelleration;
}
int Machine::getSpinSpinAccelleration()
{
    return _config.spinAccell;
}

///////////////////////////////////////
// Configure the positions for vertical
//////////////
void Machine::ConfigVerticalPositions(int Top, int Middle, int Bottom)
{
    _config.verticalTopStepValue = Top;
    _config.verticalMidStepValue = Middle;
    _config.verticalBottomStepValue = Bottom;
}
void Machine::ConfigHorizontalPositions(int Wash, int Rinse, int FinalRinse, int Dryer)
{
    _config.horizontalWashStepValue = Wash;
    _config.horizontalRinseStepValue = Rinse;
    _config.horizontalFinalRinseStepValue = FinalRinse;
    _config.horizontalDryerStepValue = Dryer;
}
#pragma endregion

#pragma region PrivateMethods

void Machine::_SetActiveMotor(MOTOR Motor)
{
    // Stop current motor if moving
    // TODO: Will cause current position to be incorrect.
    _stepper->stopMove();
    while (_stepper->isRunning())
    {
        delay(50);
    }
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
        _state.selectedMotor = Motor;
        break;
    case MOTOR::horizontal:
        _stepper->setDirectionPin(_config.pinDirection, false);
        _stepper->setEnablePin(_config.pinMotorHorzontalEnable);
        _state.selectedMotor = Motor;
        _stepper->setCurrentPosition(_state.horizontalCurrentStep);
        break;

    default:
        break;
    }
    _stepper->setAutoEnable(true);
};

void Machine::ActionMoveVerticalTo(VERTICALPOSITION VerticalPosition)
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

void Machine::_initPins()
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
void Machine::_setDefaultConfig()
{
    _config.pinStep = pinStep;
    _config.pinDirection = pinDirection;
    _config.pinMotorSpinEnable = pinSpinMotorEnable;
    _config.pinMotorVerticalEnable = pinVerticalMotorEnable;
    _config.pinMotorHorzontalEnable = pinHorizontalMotorEnable;
    _config.pinTopLimitSwitch = pinTopLimitSwitch;
    _config.pinLeftLimitSwitch = pinLeftLimitSwitch;
    _config.pinDryer = pinDryer;
    _config.pin24vOn = pin24v;

    _config.spinAccell = 3000; // In Hz/s/s
    _config.RPMtoStepsRatio = 27;

    _config.verticalTopStepValue = 2000;
    _config.verticalMidStepValue = 20000;
    _config.verticalBottomStepValue = 40000;
    _config.verticalSpeed = 8000;
    _config.verticalAccell = 8000;

    _config.horizontalWashStepValue = 0;
    _config.horizontalRinseStepValue = 0;
    _config.horizontalFinalRinseStepValue = 0;
    _config.horizontalDryerStepValue = 0;
    _config.horizontalSpeed = 5000;
    _config.horizontalAccell = 5000;
}

#pragma endregion