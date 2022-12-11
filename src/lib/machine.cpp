#include <Arduino.h>

#ifndef STRUCTS
#define STRUCTS
#include <./lib/structs.cpp>
#endif

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

void Machine::Action05Initialise()
{
    Action30MoveVertToHome();
};

void Machine::Action30MoveVertToHome()
{
    _SetActiveMotor(motor::vertical);
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
        if (millis() - startMs > _config.verticalLimitTimeoutMs){
            timeout = true;
            Serial.printf("Home timed out");
        }
        delay(10);
    }

    _stepper->forceStopAndNewPosition(uint32_t(0));
    _verticalPoistion = 0;
};

void Machine::Action10SetSpinClockwise() { _spinSelectedDirection = clockwise; };
void Machine::Action11SetSpinAntiClockwise() { _spinSelectedDirection = anticlockwise; };
void Machine::Action12SetSpinBothwise() { _spinSelectedDirection = bothwise; };
void Machine::Action13SetSpinSpeedTo1() { _spinSelectedSpeed = _config.spinSpeeds[0]; };
void Machine::Action14SetSpinSpeedTo2() { _spinSelectedSpeed = _config.spinSpeeds[1]; };
void Machine::Action15SetSpinSpeedTo3() { _spinSelectedSpeed = _config.spinSpeeds[2]; };
void Machine::Action16SetSpinSpeedTo4() { _spinSelectedSpeed = _config.spinSpeeds[3]; };
void Machine::Action17SetAltSpinDurationTo1() { _spinSelectedSpindBothDuration = _config.spinSpinReverseTime[0]; };
void Machine::Action18SetAltSpinDurationTo2() { _spinSelectedSpindBothDuration = _config.spinSpinReverseTime[1]; };
void Machine::Action19SetAltSpinDurationTo3() { _spinSelectedSpindBothDuration = _config.spinSpinReverseTime[2]; };
void Machine::Action20SetAltSpinDurationTo4() { _spinSelectedSpindBothDuration = _config.spinSpinReverseTime[3]; };
void Machine::Action31MoveVertToTop() { _MoveVerticalToPosition(_config.verticalTopPosition); };
void Machine::Action32MoveVertToMid() { _MoveVerticalToPosition(_config.verticalMidPosition); };
void Machine::Action33MoveVertToBottom() { _MoveVerticalToPosition(_config.verticalBottomPosition); };
void Machine::ActionSpin(int Duration)
{
    _SetActiveMotor(motor::spin);
    _stepper->setSpeedInHz(_spinSelectedSpeed);
    _stepper->setAcceleration(_config.spinAccell);
    int start = millis();
    while (millis() - start <= Duration)
    {
        switch (_spinSelectedDirection)
        {
        case bothwise:
            _stepper->runForward();
            delay(_spinSelectedSpindBothDuration);
            _stepper->runBackward();
            delay(_spinSelectedSpindBothDuration);
            break;
        case clockwise:
            _stepper->runForward();
            delay(Duration);
        case anticlockwise:
            _stepper->runBackward();
            delay(Duration);

        default:
            break;
        }
    }
    _stepper->stopMove();
    while (_stepper->isRunning())
    {
        delay (50);
    }

    
};

#pragma endregion

#pragma region SetterGetters
config Machine::getConfig()
{
    return _config;
}

void Machine::setSpeeds(int Speed0, int Speed1, int Speed2, int Speed3)
{
    _config.spinSpeeds[0] = Speed0; // In Hz
    _config.spinSpeeds[1] = Speed1;
    _config.spinSpeeds[2] = Speed2;
    _config.spinSpeeds[3] = Speed3;
}

void Machine::setSpinAccelleration(int Accelleration)
{
    _config.spinAccell = Accelleration;
}
int Machine::getSpinSpinAccelleration()
{
    return _config.spinAccell;
}
void Machine::setVerticalPositions(int Top, int Middle, int Bottom)
{
    _config.verticalTopPosition = Top;
    _config.verticalMidPosition = Middle;
    _config.verticalBottomPosition = Bottom;
}
void Machine::setHorizontalPositions(int Wash, int Rinse, int FinalRinse, int Dryer)
{
    _config.horizontalWash = Wash;
    _config.horizontalRinse = Rinse;
    _config.horizontalFinalRinse = FinalRinse;
    _config.horizontalDryer = Dryer;
}
#pragma endregion

#pragma region PrivateMethods

void Machine::_SetActiveMotor(motor Motor)
{
    _stepper->stopMove();
    while (_stepper->isRunning()){delay(50);}
    switch (Motor)
    {
    case spin:
        _stepper->setDirectionPin(_config.pinDirection);
        _stepper->setEnablePin(_config.pinMotorSpinEnable);
        break;
    case vertical:
        _stepper->setDirectionPin(_config.pinDirection, false);
        _stepper->setEnablePin(_config.pinMotorVerticalEnable);
        _stepper->setCurrentPosition (_verticalPoistion);
        break;
    case horizontal:
        _stepper->setDirectionPin(_config.pinDirection, false);
        _stepper->setEnablePin(_config.pinMotorHorzontalEnable);
        break;

    default:
        break;
    }
    _stepper->setAutoEnable(true);
};

void Machine::_MoveVerticalToPosition(int Position)
{
    _SetActiveMotor(motor::vertical);
    Serial.print ("Destination:");
    Serial.println (Position);
    Serial.print ("Current Position:");
    Serial.println (_stepper->getCurrentPosition());
    _stepper->setAcceleration(_config.verticalAccell);
    _stepper->setSpeedInHz(_config.horizontalSpeed);
    _stepper->moveTo(Position);
    while (_stepper->isRunning())
    {
        delay(50);
    }
    _verticalPoistion = _stepper->getCurrentPosition();
    Serial.print ("Final Position:");
    Serial.println (Position);

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

    _config.spinSpeeds[0] = 10000; // In Hz
    _config.spinSpeeds[1] = 20000;
    _config.spinSpeeds[2] = 30000;
    _config.spinSpeeds[3] = 40000;
    _spinSelectedSpeed = _config.spinSpeeds[0];
    _config.spinAccell = 3000; // In Hz/s/s

    _config.spinSpinReverseTime[0] = 10000; // In ms
    _config.spinSpinReverseTime[1] = 30000;
    _config.spinSpinReverseTime[2] = 60000;
    _config.spinSpinReverseTime[3] = 90000;
    _spinSelectedSpindBothDuration = _config.spinSpinReverseTime[0];

    _config.verticalTopPosition = 2000;
    _config.verticalMidPosition = 20000;
    _config.verticalBottomPosition = 40000;
    _config.verticalSpeed = 8000;
    _config.verticalAccell = 8000;

    _config.horizontalWash = 0;
    _config.horizontalRinse = 0;
    _config.horizontalFinalRinse = 0;
    _config.horizontalDryer = 0;
    _config.horizontalSpeed = 5000;
    _config.horizontalAccell = 5000;
}

#pragma endregion