#include "LineScan.h"

LineScan::LineScan(int clk, int si, int ao, unsigned int sampleRateHz, int edgeInt, int midInt, String id) {
  CLKpin = clk;
  SIpin = si;
  AOpin = ao;
  edgeInterval = edgeInt;
  midInterval = midInt;
  width = 0;
  faserBandLength = 0;
  faserCounter = 0;
  enable_dataProcessing = true; // Standardwert
  pixelLength = 0.04; // Standardwert
  minInRow = 2; // Standardwert
  sensorId = id;
  sampleInterval = 1000 / sampleRateHz; // Berechne das Abtastintervall in Millisekunden
  exposureTime = 5; // Setze die Belichtungszeit
}

//SETUP
void LineScan::setup() {
  pinMode(CLKpin, OUTPUT);
  pinMode(SIpin, OUTPUT);
  Initialize();
}

//-----------Update ContiniousMode (DateSheet)---------
//individual sampleInterval can be added by using uncommented codelines below
double LineScan::Continious() { 
  resetErrors();
  automateTresholdCalculation(); 
  unsigned long currentTime = millis(); 
  //if (currentTime - lastSampleTime >= sampleInterval) {
    ReadAnalog();
    calculateFaserValues(); 
    checkWidth();
    //lastSampleTime = currentTime; // Aktualisiere den Zeitstempel der letzten Abtastung
  //}
  return faserCounter;
}

//-------Update OneShotMode (DateSheet)---------
//individual sampleInterval can be added by using uncommented codelines below
double LineScan::OneShot() {
  resetErrors();
  automateTresholdCalculation();
  unsigned long currentTime = millis();
  //if (currentTime - lastSampleTime >= sampleInterval) { 
    GarbageClockOut();
    unsigned long waitTime = 15000; // gewünschte Belichtungszeit in Mikrosekunden
    unsigned long targetTime = micros() + waitTime;
    while (micros() < targetTime) {
      // Warte, bis die Zielzeit erreicht ist
    }   
    ReadAnalog();
    calculateFaserValues();  
    checkWidth();
    //lastSampleTime = currentTime; // Aktualisiere den Zeitstempel der letzten Abtastung
  //}
  return faserCounter;
}

//----------Help functions----------

//GarbageClockout
void LineScan::GarbageClockOut(){
  digitalWrite(SIpin, HIGH);
  ClockPulse();
  digitalWrite(SIpin, LOW);
  for (int i = 1; i < 128; i++) {
    ClockPulse();
  }
}

void LineScan::resetErrors(){
  ErrorState2 = false;
  ErrorState3 = false;
  State = 0;
}

String LineScan::getSensorId() {
  return sensorId;
}

void LineScan::ClockPulse() {
  delayMicroseconds(1); // Mindestverzögerung zwischen Taktimpulsen
  digitalWrite(CLKpin, HIGH);
  digitalWrite(CLKpin, LOW);
  //PORTB |= _BV(PB0);  // oder PORTB |= (1 << PB0);
  //PORTB &= ~_BV(PB0); // oder PORTB &= ~(1 << PB0);
}

//---Read Sensor Value---
// Process can be accelerated if using direct access to ports
void LineScan::ReadAnalog() {
  digitalWrite(SIpin, HIGH);
  ClockPulse();
  digitalWrite(SIpin, LOW);
  //PORTB |= _BV(PB1);  // oder PORTB |= (1 << PB1);
  //ClockPulse();
  //PORTB &= ~_BV(PB1); // oder PORTB &= ~(1 << PB1);
  dataD1[0] = analogRead(AOpin);
  for (int i = 1; i < 128; i++) {
    //delayMicroseconds(1); // Mindestverzögerung zwischen den Abtastungen
    ClockPulse();
    double t3 = micros();
    dataD1[i] = analogRead(AOpin);
    double t4 = micros();
    //checkPixelValue(dataD1[i]); checks pixelvalue but takes a lot of time. -> not recommended enabling
  }
}

//checks if measured value is bigger or smaller then TH
void LineScan::calculateFaserValues() {
  faserCounter = 0;
  for (int i = 0; i < 128; i++) {
      if (dataD1[i] > faser_treshhold) {
        faserCounter += 1;
      }
  }
}

void LineScan::automateTresholdCalculation(){
  int edgeSum = 0;
  int midSum = 0;
  for (int i = 0; i < edgeInterval; i++) {
    edgeSum += dataD1[i];  // Summe der ersten 15 Elemente
  }
  for (int i = 128 - edgeInterval; i < 128; i++) { // letzte 15 Elemente (Index 113 bis 127)
    edgeSum += dataD1[i];
  }
  for (int i = 128 / 2 - midInterval / 2; i < 128 / 2 + midInterval / 2; i++) {
    midSum += dataD1[i];
  }
  double averageEdge = edgeSum / (2 * edgeInterval);
  double averageMid = midSum / midInterval;
  faser_treshhold = (averageEdge + averageMid) / 2;
}

//algorithm currently not in use
void LineScan::limitExceptions(){
  int inRowStartCounter = 0;
  int finishCounter = 0;
  bool foundStart = false;
  bool foundEnd = false;
  int startFaserband = 0;
  int endFaserband = 128;

  for (int i = 0; i < 128; i++) {
    if (dataD1[i] > faser_treshhold) {
      finishCounter = 0;
      if (!foundStart) {
        inRowStartCounter += 1;
      }
    } else {
      inRowStartCounter = 0;
      if (foundStart && !foundEnd) {
        finishCounter += 1;
      }
    }
    if (inRowStartCounter > minInRow && !foundStart) {
      startFaserband = i - minInRow;
      foundStart = true;
    }
    if (finishCounter > minInRow && !foundEnd && foundStart) {
      endFaserband = i - minInRow;
      foundEnd = true;
    }
  }
  faserBandLength = endFaserband - startFaserband;
    //width = pixelLength * faserBandLength;
}

void LineScan::Initialize() {
  for (int i = 0; i < 128; i++) {
    ClockPulse();
  }
  digitalWrite(SIpin, HIGH);
  ClockPulse();
  digitalWrite(SIpin, LOW);
  for (int i = 1; i < 128; i++) {
    ClockPulse();
  }
}

//Plausibilitätsprüfung 2
void LineScan::checkPixelValue(int pixelValue) {
  if (pixelValue < minPixelValue || pixelValue > maxPixelValue) {
    ErrorState2 = true;
    State = 2;
  }
}

//Plausibilitätsprüfung 3
void LineScan::checkWidth() {
  if (width < 0 || width > 128) {
    ErrorState3 = true;
    State = 2;
  }

}
bool LineScan::getErrorState2() {
  return ErrorState2;
}
bool LineScan::getErrorState3() {
  return ErrorState3;
}
int LineScan::getState() {
  return State;
}

// interface for Python SIM tool
void LineScan::useSim() {
  Serial.print("Data: ");
  for (int i = 0; i < 127; i++) {  // Schleife bis zum vorletzten Element
    Serial.print(dataD1[i]);
    Serial.print(",");
  }
  Serial.print(dataD1[127]);  // Letztes Element ohne Komma hinzufügen
  Serial.print(" ");
  Serial.print("FaserCounter: ");
  Serial.print(faserCounter);
  Serial.print(" Width: ");
  Serial.println(width);  // Hier wird die Zeile mit einem Newline-Zeichen abgeschlossen
}

/* calibration to convert relative value into absolute value:
take a X 2,3,4,5 cm paper (more than 1 for higher precision)
measure pixelAboveTreshhold
pixelLength = X(cm)/pixelAboveTreshhold
use average pixelLength

width = pixelLength * faserBandLength;
*/
