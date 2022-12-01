struct config {
    int spinSpeeds [3]; //spinSpeeds[0] default
    int spinSpinReverseTime [3]; //If Spin both directions then how long in ms for each direction
    int spinAccell; 

    int verticalTopPosition;  //Step position of top.
    int verticalMidPosition;  //Step position of Mid.
    int verticalBottomPosition;  //Step position of Bottom.
    int verticalSpeed;
    int verticalAccell; 

    int horizontalWash;  //Step position of Wash Jar.
    int horizontalRinse;  //Step position Rinse Jar.
    int horizontalFinalRinse;  //Step position Final Rinse Jar.
    int horizontalDryer;  //Step position Dryer.
    int horizontalSpeed;
    int horizontalAccell;

    int pinStep;
    int pinDirection;
    int pinMotorSpinEnable;
    int pinMotorVerticalEnable;
    int pinMotorHorzontalEnable;

    int pinTopLimitSwitch;
    int pinLeftLimitSwitch;
    int pinDryer;
    int pin24vOn;

} ;

enum directions {clockwise, anticlockwise, bothwise};

