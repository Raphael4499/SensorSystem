

#include "SensorSystem.h"
#include "Lightsensor.h"
#include "LineScan.h"
#include "Encoder.h"

// Initialize ENCODER
Encoder encoder2(20, 21, 1024, "E", 10000); //(int pinA (A/yellow -> 20), int pinB (B/Green -> 21), Sensor resolution, Sensorschlüssel, Max PlausibleRPM) - cable meaning: (+V->Brown, GND->White, A->Yellow, B->Green, Z->Gray)
Encoder encoder1(2, 3, 1024, "B", 10000);   //(int pinA (A/yellow -> 2), int pinB (B/Green -> 3)  , Sensor resolution, Sensorschlüssel, Max PlausibleRPM) - cable meaning: (+V->Brown, GND->White, A->Yellow, B->Green, Z->Gray)


// Initialize LIGHTSENSOR
Lightsensor lightsensor1(A1, 110, 2, "C", 1000,10000, 70); // (int pin, int manualThreshold, CalibrationMode, Sensorschlüssel, unsigned int sampleRateHz,  Max PlausibleRPM, Messintervalllänge t_min)
Lightsensor lightsensor2(A0, 500, 2, "D", 1000,10000, 100); // (int pin, int manualThreshold, CalibrationMode, Sensorschlüssel, unsigned int sampleRateHz,  Max PlausibleRPM, Messintervalllänge t_min)
//tmin > 5 für geringere Messfehler. Wird mittels Moving average in 10 mal t_min teile unterteilt
//CalibrationMode: 0 ->manual Threshold ; 1 -> MinMax Init Threshold ; 2 -> Vereinfachter Signifikanz Init Threshold;

// Initialize LINIENSENSOR
LineScan linescan1(53, 52, A0, 1000, 15, 14, "A"); //(int clkPIN (53 -> C), int siPIN (52 -> B), int aoPIN (A0 -> A), sampleRateHz, edgeIntervall für Automated Fasserklassifizierung, midIntervall für Automated Fasserklassifizierung, Sensorschlüssel)

// setup the system
void setup() {
  
  Serial.begin(500000); //baudrate -> darauf achten diese auch in der konsole zu wählen
  while (!Serial) {
    ; // Warte auf die serielle Verbindung
  }

  setUpLampe(); //schaltet Rotorlampe ein

  Serial.println("--------------------------------------------------------------------"); // Signalisiert das System startet
  
  encoder1.setup(0); // setup für encoder1
  encoder2.setup(1); // setup für encoder2

  delay(1500); //delay damit lichtsensoren genug Licht bekommen

  linescan1.setup(); //setup für linescan1
  lightsensor1.setup(); // setup für lightsensor1
  lightsensor2.setup(); // setup für lightsensor2    
}

// FKT die Rotorlampe einschaltet
void setUpLampe(){
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
}

// Funktion dient als Alternative für das ZYKLUSkonzept, wird in der finalen Software nicht verwendet -> führt alle x HZ eine Aktion aus wenn verwendet
void setupTimer4() {
  noInterrupts();
  // Clear registers
  TCCR4A = 0;
  TCCR4B = 0;
  TCNT4 = 0;

  // 1 Hz (16000000/((15624+1)*1024))
  OCR4A = 15624; //ändern auf 1,3ms wenn man methode verwenden will (muss umgerechnet werden)
  // CTC
  TCCR4B |= (1 << WGM42);
  // Prescaler 1024
  TCCR4B |= (1 << CS42) | (1 << CS40);
  // Output Compare Match A Interrupt Enable
  TIMSK4 |= (1 << OCIE4A);
  interrupts();
}

//diese fkt wird aufgerufen, wenn Timer4 abläuft
/*
ISR(TIMER4_COMPA_vect) {
  updateLightSensor1();
}
*/

//main loop der immer läuft
void loop() {
  //loopStartTime = micros();

  useZyklus();

  /* -> verlangsamerung des loops für debugging
  while (micros() - loopStartTime < 0) { // 2000000 us = 2000 ms
    // Keine Operation, um die Zeit bis zur nächsten Loop-Iteration zu warten
  } 
  */
}


//Set StateMachine Paramter
bool useStateMachine = false; //StateMachine benutzen?
double Zyklusinterval = 250 * 1000; //alle wie viel microS soll der Zyklus aufgerufen werden

// Zyklus softwarearchitektur

void useZyklus(){
  ZyklusStart = micros();

  //Messung zwischen jeder Itteration
  double valueD = lightsensor2.update();
  double valueC = lightsensor1.update();

  //Schleife wird nur geöffnet wenn Zeit seit letztem Zyklusende größer als Zyklusintervall
  //Hier wird ein Zyklus ausgeführt: -> Messen -> Datenverarbeitung Sensor1 -> Messen -> Datenverarbeitung Sensor2
  if (ZyklusStart - finishedLastZyklus > Zyklusinterval){
    periode += 1;
    if (periode == 1){
      double valueB = encoder1.update();
      sendData(encoder1.getSensorId(), valueB);
    }else if (periode == 2){
      sendData(lightsensor1.getSensorId(), valueC);
    }else if (periode == 3){
      sendData(lightsensor2.getSensorId(), valueD);
    }else if (periode == 4){
      double valueE = encoder2.update();
      sendData(encoder2.getSensorId(), valueE);
      if (useStateMachine == false){
        periode = 0;
        finishedLastZyklus = micros();
      }
    } else if (periode == 5){ 
      stateAnalysis();
      checkPlausibility();
      sendErrorCode();
      periode = 0;
      finishedLastZyklus = micros();
    }
  }

  /* ->delay in den Zyklus adden um Messfrequenz zu verrignern
  while (micros() - ZyklusStart < 1000) { // 2000000 us = 2000 ms
    // Keine Operation, um die Zeit bis zur nächsten Loop-Iteration zu warten
  } 
  */
}

//analysiert den Status der Sensoren
void stateAnalysis(){
  stateEncoder1 = encoder1.getState();
  //printSensorState("Encoder", stateEncoder1);
  if (stateEncoder1 == 2) {
    ErrorState1_encoder1 = encoder1.getErrorState1();
  }
    stateEncoder2 = encoder2.getState();
  //printSensorState("Encoder", stateEncoder1);
  if (stateEncoder2 == 2) {
    ErrorState1_encoder2 = encoder2.getErrorState1();
  }
    stateLightsensor1 = lightsensor1.getState();
  //printSensorState("LightSensor", stateLightsensor1);
  if (stateLightsensor1 == 2) {
    ErrorState1_lightsensor1 = lightsensor1.getErrorState1();
    ErrorState2_lightsensor1 = lightsensor1.getErrorState2();
  }
    stateLightsensor2 = lightsensor2.getState();
  //printSensorState("LightSensor", stateLightsensor1);
  if (stateLightsensor2 == 2) {
    ErrorState1_lightsensor2 = lightsensor2.getErrorState1();
    ErrorState2_lightsensor2 = lightsensor2.getErrorState2();
  }
}
