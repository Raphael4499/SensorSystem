#include "Encoder.h"


//pointer für Drehgeber insanzen
Encoder* Encoder::instance0 = nullptr;
Encoder* Encoder::instance1 = nullptr;

Encoder::Encoder(int pinA, int pinB, int res, String id, double settedMaxPlausibleRPM) {
  interruptPinA = pinA;
  interruptPinB = pinB;
  resolution = res * 2; // Weil wir Rising verwenden -> analog zu verschiedenen Modi (Datasheet)
  impulse = 0;
  lastInterruptTime = 0;
  drehzahl = 0.0;
  encoderId = id; // Setzen der String-ID
  maxPlausibleRPM = settedMaxPlausibleRPM;
}

//SETUP
void Encoder::setup(int instance) {
  pinMode(interruptPinA, INPUT_PULLUP);
  pinMode(interruptPinB, INPUT_PULLUP);
  
  if (instance == 0) {
    instance0 = this;
    //initialize with interrupt
    attachInterrupt(digitalPinToInterrupt(interruptPinA), isrA1, RISING);
    attachInterrupt(digitalPinToInterrupt(interruptPinB), isrB1, RISING);
  } else if (instance == 1) {
    instance1 = this;
    //initialize with interrupt
    attachInterrupt(digitalPinToInterrupt(interruptPinA), isrA2, RISING);
    attachInterrupt(digitalPinToInterrupt(interruptPinB), isrB2, RISING);
  }
}

//updates function for both encoders
double Encoder::update() {
  resetErrors();
  static unsigned long lastTime1 = 0;
  static unsigned long lastTime2 = 0;
  unsigned long currentTime = millis();
  double timeInterval1 = currentTime - lastTime1;
  double timeInterval2 = currentTime - lastTime2;

  //calRate_n defines how often rpm should be calcualted
  if (this == instance0 && timeInterval1 >= calcRate_n) { // Berechnung der Drehzahl für instance0
    drehzahl = (impulse / (double)resolution) * (60000.0 / timeInterval1);
    impulse = 0; // Impulszähler zurücksetzen
    lastTime1 = currentTime; // Zeitstempel aktualisieren
  }
  if (this == instance1 && timeInterval2 >= calcRate_n) { // Berechnung der Drehzahl für instance1
    drehzahl = (impulse / (double)resolution) * (60000.0 / timeInterval2);
    impulse = 0; // Impulszähler zurücksetzen
    lastTime2 = currentTime; // Zeitstempel aktualisieren
  }
  checkPlausibleRPM(); // plausibilitätscheck 
  return drehzahl;
}

void Encoder::resetErrors() {
  ErrorState1 = false;
  State = 0;
}
//set State for Encoder
void Encoder::checkPlausibleRPM() {
  if (drehzahl == 0) {
    State = 0;
  } else {
    State = 1;
  }
  if (drehzahl < 0 || drehzahl > maxPlausibleRPM) {
    ErrorState1 = true;
    State = 2;
  }
}

String Encoder::getSensorId() {
  return encoderId;
}

void Encoder::isrA1() { if (instance0) instance0->handleIsrA1(); }
void Encoder::isrB1() { if (instance0) instance0->handleIsrB1(); }
void Encoder::isrA2() { if (instance1) instance1->handleIsrA2(); }
void Encoder::isrB2() { if (instance1) instance1->handleIsrB2(); }


//---------------------Interrupt Handler---------------
// -> using A and B Signals  to get rotation direction
void Encoder::handleIsrA1() {
  if (digitalRead(interruptPinA) == digitalRead(interruptPinB)) {
    impulse++;
  } else {
    impulse--;
  }
  lastInterruptTime = millis();
}

void Encoder::handleIsrB1() {
  if (digitalRead(interruptPinA) != digitalRead(interruptPinB)) {
    impulse++;
  } else {
    impulse--;
  }
  lastInterruptTime = millis();
}

void Encoder::handleIsrA2() {
  if (digitalRead(interruptPinA) == digitalRead(interruptPinB)) {
    impulse++;
  } else {
    impulse--;
  }
  lastInterruptTime = millis();
}

void Encoder::handleIsrB2() {
  if (digitalRead(interruptPinA) != digitalRead(interruptPinB)) {
    impulse++;
  } else {
    impulse--;
  }
  lastInterruptTime = millis();
}

//-----get methods--------
bool Encoder::getErrorState1() {
  return ErrorState1;
}

int Encoder::getState() {
  return State;
}
