#include <Arduino.h>


//Hier befinden sich Hilfsvariablen und Hilfsfunktionen für das Sensorgesamtsystem

//------------help-vars------------

//State Machine Variables
unsigned long resetTimer;
unsigned long inconsistencyStartTime = 0;
unsigned long lastPlausibilityCheckTime = 0;
bool inconsistencyDetected = false;
const unsigned long toleranceDuration = 10000; // Toleranzdauer für das Tolerieren unterschiedlicher Sensorstates
int stateEncoder1 = 0, stateEncoder2 = 0, stateLightsensor1 = 0, stateLightsensor2 = 0, stateLinescan1 = 0;
int ErrorState1_encoder1 = 0, ErrorState1_encoder2 = 0, ErrorState1_lightsensor1 = 0, ErrorState2_lightsensor1 = 0, ErrorState1_lightsensor2 = 0, ErrorState2_lightsensor2 = 0, ErrorState2_linescan1 = 0, ErrorState3_linescan1 = 0;
bool ErrorState4_encoder1 = false, ErrorState4_lightsensor1 = false, ErrorState4_encoder2 = false, ErrorState4_lightsensor2 = false;
int StateMachine = 0;


// Array von Zeigsern auf die Zustände der relevanten Sensoren
int* statesPL[] = {&stateEncoder1, &stateLightsensor1, &stateEncoder2, &stateLightsensor2};
int* statesSM[] = {&stateEncoder1, &stateLightsensor1, &stateEncoder2, &stateLightsensor2, &stateLinescan1};

bool allDefault;
bool allActive;

unsigned long timeAfterSendCurrentTime;
unsigned long timeAfterUpdateEncoder;
unsigned long timeAfterUpdateLightSensor;
unsigned long timeAfterUpdateLineScan;
unsigned long timeAfterCheckPlausibility;
unsigned long timeAfterStateMachine;
unsigned long timeAfterSendSM;
unsigned long timeAfterSendErrorCode;
unsigned long timeAfterResetErrorStates;
unsigned long ZyklusStart;
unsigned long finishedLastZyklus = 0;

int periode = 0;

String errorCode;

double loopStartTime = 0;
unsigned long loopDuration = 0;


double timer;
double timeUsed;



//------- hel funcs --------------

void sendData(String SensorId, double SensorValue) {
  Serial.print(SensorId);
  Serial.print(",");
  Serial.println(SensorValue);
}

void sendSM() {
  Serial.print("StateMachine,");
  Serial.println(StateMachine);
  timeAfterSendSM = micros();
}

void printErrorCode(){
  Serial.print("Err,");
  Serial.println(errorCode);
}

void sendTime(double timeStamp) {
  Serial.print("TimeStamp,");
  Serial.println(timeStamp); // '3' specifies three decimal places
}

void sendCurrentTime() {
  double timeStamp = millis() / 1000.0;
  sendTime(timeStamp);
  timeAfterSendCurrentTime = micros();
}

//consistency check und error state 4
void plausibilityCheck() {
  allDefault = true;
  allActive = true;
  int defaultCount = 0;
  int activeCount = 0;

  for (int i = 0; i < 2; i++) { // Nur die relevanten Sensoren (Encoder und Lightsensor)
    
    if (*statesPL[i] == 0) {
      defaultCount++;
    } else if (*statesPL[i] == 1) {
      activeCount++;
    }

    if (*statesPL[i] != 0) {
      allDefault = false;
    }
    if (*statesPL[i] != 1) {
      allActive = false;
    }
  }

  if (defaultCount == activeCount) {
    if (!inconsistencyDetected) {
      inconsistencyDetected = true;
      inconsistencyStartTime = millis();
    } else {
      if (millis() - inconsistencyStartTime > toleranceDuration) {
        // Alle relevanten Sensoren in den Error State 2 versetzen
        if (stateEncoder1 != 2) {
          stateEncoder1 = 2;
          ErrorState4_encoder1 = true;
        }
        if (stateEncoder2 != 2) {
          stateEncoder2 = 2;
          ErrorState4_encoder2 = true;
        }
        if (stateLightsensor1 != 2) {
          stateLightsensor1 = 2;
          ErrorState4_lightsensor1 = true;
        }
        if (stateLightsensor2 != 2) {
          stateLightsensor2 = 2;
          ErrorState4_lightsensor2 = true;
        }
        
        StateMachine = 2;
      }
    }

  } else if (!allDefault && !allActive) {
    if (!inconsistencyDetected) {
      inconsistencyDetected = true;
      inconsistencyStartTime = millis();
    } else {
      if (millis() - inconsistencyStartTime > toleranceDuration) {
        // Nur den aus der Reihe tanzenden Sensor in den Error State 2 versetzen
        if (defaultCount == 1) {
          if (stateEncoder1 == 0) {
            stateEncoder1 = 2;
            ErrorState4_encoder1 = true;
          } else if (stateEncoder2 == 0) {
            stateEncoder2 = 2;
            ErrorState4_encoder2 = true;
          } else if (stateLightsensor1 == 0) {
            stateLightsensor1 = 2;
            ErrorState4_lightsensor1 = true;
          } else if (stateLightsensor2 == 0) {
            stateLightsensor2 = 2;
            ErrorState4_lightsensor2 = true;
          }

        } else if (activeCount == 1) {
          if (stateEncoder1 == 1) {
            stateEncoder1 = 2;
            ErrorState4_encoder1 = true;
          } else if (stateEncoder2 == 1) {
            stateEncoder2 = 2;
            ErrorState4_encoder2 = true;
          } else if (stateLightsensor1 == 1) {
            stateLightsensor1 = 2;
            ErrorState4_lightsensor1 = true;
          } else if (stateLightsensor2 == 1) {
            stateLightsensor2 = 2;
            ErrorState4_lightsensor2 = true;
          }
        }
        StateMachine = 2;
      }
    }
  } else {
    inconsistencyDetected = false;
  }
}

unsigned long plausibilityCheckInterval = 0; //alle wie viel ms soll Plausibility gecheckt werden

void checkPlausibility() {
  resetTimer = millis();
  if (resetTimer - lastPlausibilityCheckTime >= plausibilityCheckInterval) {
    plausibilityCheck();
    lastPlausibilityCheckTime = millis();
  }
  timeAfterCheckPlausibility = micros();
}

void resetErrorStates() {
  if (resetTimer - lastPlausibilityCheckTime >= plausibilityCheckInterval) {
      ErrorState4_encoder1 = false;
    ErrorState4_lightsensor1 = false;
  }
  timeAfterResetErrorStates = micros();
}

// berechnet SM value
void stateMachine() {
  bool allDefault = true;
  bool allActive = true;

  for (int i = 0; i < 3; i++) {

    if (*statesSM[i] == 2) {
      StateMachine = 2;
      return;
    }
    if (*statesSM[i] != 0) {
      allDefault = false;
    }
    if (*statesSM[i] != 1) {
      allActive = false;
    }
  }

  if (allDefault) {
    StateMachine = 0;
  } else if (allActive) {
    StateMachine = 1;
  }
  timeAfterStateMachine = micros();
}

void sendErrorCode() {
  errorCode = "";
  if (StateMachine == 2) {

    // Encoder 1
    if (stateEncoder1 == 2) {
      if (ErrorState1_encoder1) {
        errorCode += "B1";      
      }
      if (ErrorState4_encoder1) {
        errorCode += "B4";
      }
    }

    // Encoder 2
    if (stateEncoder2 == 2) {
      if (ErrorState1_encoder2) {
        errorCode += "E1";      
      }
      if (ErrorState4_encoder2) {
        errorCode += "E4";
      }
    }

    // LightSensor1
    if (stateLightsensor1 == 2) {
      if (ErrorState1_lightsensor1) {
        errorCode += "C1";     
      }
      if (ErrorState2_lightsensor1) { 
        errorCode += "C2";
      }
      if (ErrorState4_lightsensor1) { 
        errorCode += "C4";
      }
    }

    // LightSensor2
    if (stateLightsensor2 == 2) {
      if (ErrorState1_lightsensor2) {
        errorCode += "D1";     
      }
      if (ErrorState2_lightsensor2) { 
        errorCode += "D2";
      }
      if (ErrorState4_lightsensor2) { 
        errorCode += "D4";
      }
    }

    // LineSensor
    if (stateLinescan1 == 2) {
      if (ErrorState2_linescan1) {
        errorCode += "E2";       
      }
      if (ErrorState3_linescan1) {
        errorCode += "E3";
      }
    }
  } else if (StateMachine == 0) {
    errorCode = "0";
  } else if (StateMachine == 1) {
    errorCode = "1";
  }
  printErrorCode();
  timeAfterSendErrorCode = micros();
}

void printSensorState(const String& sensorName, int state) {
  Serial.print(sensorName);
  Serial.print(" state: ");
  Serial.println(state);
}
