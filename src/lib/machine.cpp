#include <Arduino.h>

#ifndef STRUCTS
#define STRUCTS
#include <./lib/structs.cpp>
#endif

#include <./lib/machine.H>
#include <./config/pindefaults.h>

#pragma region PublicMethods

void Machine::Action10SetSpinClockwise() { _spinSelectedDirection = clockwise; };
void Machine::Action11SetSpinAntiClockwise() { _spinSelectedDirection = anticlockwise; };
void Machine::Action12SetSpinBothwise() { _spinSelectedDirection = bothwise; };
#pragma endregion

#pragma region SetterGetters
config Machine::getConfig() { return _config; }

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

    _config.spinSpeeds[0] = 5000; // In Hz
    _config.spinSpeeds[1] = 10000;
    _config.spinSpeeds[2] = 15000;
    _config.spinSpeeds[3] = 20000;
    _config.spinAccell = 3000; // In Hz/s/s

    _config.spinSpinReverseTime[0] = 10000; // In ms
    _config.spinSpinReverseTime[1] = 30000;
    _config.spinSpinReverseTime[2] = 60000;
    _config.spinSpinReverseTime[3] = 90000;

    _config.verticalTopPosition = 0;
    _config.verticalMidPosition = 5000;
    _config.verticalBottomPosition = 12000;
    _config.verticalSpeed = 5000;
    _config.verticalAccell = 5000;

    _config.horizontalWash = 0;
    _config.horizontalRinse = 0;
    _config.horizontalFinalRinse = 0;
    _config.horizontalDryer = 0;
    _config.horizontalSpeed = 5000;
    _config.horizontalAccell = 5000;
}

#pragma endregion