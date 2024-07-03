#include "Lightsensor.h"
#include <numeric>
#include <list>


// Define the instance
Lightsensor* Lightsensor::instance = nullptr;

// Constructor
Lightsensor::Lightsensor(int pin, int settedThreshold, int settedCalibrationMode, String id, unsigned int sampleRateHz, double settedMaxPlausibleRPM, unsigned long setted_tmin) {
  sensorPin = pin;
  threshold = settedThreshold;
  calibrationMode = settedCalibrationMode;
  sensorId = id;
  sampleInterval = 1000 / sampleRateHz; // Calculate sample interval in ms
  maxPlausibleRPM = settedMaxPlausibleRPM;
  t_min = setted_tmin;
}

//Lichtsensor Setup
void Lightsensor::setup() {
  instance = this;  
  pinMode(sensorPin, INPUT);

  // Wichtig: hier wird geprüft welcher calibration mode verwendet wird
  if (calibrationMode == 1) {
    calibration1();
  } else if (calibrationMode == 2){
    calibration2();
  }
  //calibrationMode 0 -> manual Threshold

  readSensorValue();
  lastStateWhite = (lichtwert > threshold);
  lastStateChangeTime = millis();
}


// Calibration function 1 -> Min/Max Init Threshold calibration
void Lightsensor::calibration1() {
  double startTime = millis();
  int highestLW = 0;
  int lowestLW = 1023;
  while (millis() - startTime < calibrationTime) {
    lichtwert = analogRead(sensorPin);
    if (lichtwert > highestLW) {
      highestLW = lichtwert;
    }
    if (lichtwert < lowestLW) {
      lowestLW = lichtwert;
    }
  }
  //prüft ob signifikanter Unterschied zwischen weiß und schwarz. Faktor kann angepasst werden
  if (highestLW > 1.09 * lowestLW) {
    threshold = (highestLW + lowestLW) / 2;
  } else {
    // recalibrate
    // can be implemented with using hardware timers
  }
}

// Calibration function 2 -> Vereinfachte Signifikanzklassifizierung Init Threshold calibration
void Lightsensor::calibration2() {
  double startTime = millis();
  while (millis() - startTime < calibrationTime) {
    lichtwert = analogRead(sensorPin);
    Set.insert(lichtwert);
  }

  calculateAverages(Set);
  //prüft ob signifikanter Unterschied zwischen weiß und schwarz. Faktor kann angepasst werden
  if (secondHalfAverage > 1.02 * firstHalfAverage) {
    threshold = (firstHalfAverage + secondHalfAverage) / 2;
  } else {
    // recalibrate
    // can be implemented with using hardware timers
  }
  LastAvg = threshold;

  /* -> debugging tools
  Serial.print("initial TH : ");
  Serial.println(threshold);
  Serial.println(secondHalfAverage);
  Serial.println(firstHalfAverage);
  Serial.println("tbd");
  */
}

//help function for calibration2() -> calculates Averages of Sethalfs
void Lightsensor::calculateAverages(const std::set<int>& mySet) {
  std::vector<int> sortedValues(mySet.begin(), mySet.end());
  size_t totalSize = sortedValues.size();
  size_t midIndex = totalSize / 2;

  if (midIndex > 0) {
    firstHalfAverage = std::accumulate(sortedValues.begin(), sortedValues.begin() + midIndex, 0.0) / midIndex;
  }

  if (totalSize - midIndex > 0) {
    secondHalfAverage = std::accumulate(sortedValues.begin() + midIndex, sortedValues.end(), 0.0) / (totalSize - midIndex);
  }
}

double minTime1 = 500;
double timer1 = 0;

//measure and update Lightsensor
//if fixed samplingRate preferred use structure below that is uncommented
double Lightsensor::update() {
  resetErrors();
  //if (currentTime - lastSampleTime >= sampleInterval) {
    readSensorValue();
    checkForErrors();
    rpm = updateState();
    checkPlausibleRPM();   
    //lastSampleTime = currentTime; // Aktualisiere den Zeitstempel der letzten Abtastung
  //}
  return rpm; // bei Maschine durch rpm ersetzen
}

void Lightsensor::readSensorValue() {
  lichtwert = analogRead(sensorPin);
}

void Lightsensor::resetErrors() {
  ErrorState1 = false;
  ErrorState2 = false;
  State = 0;
}

// Plausibilitätskriterium 1
void Lightsensor::checkForErrors() {
  if (lichtwert < 0 || lichtwert > 1023) {
    ErrorState1 = true;
    State = 2;
  }
}

//hier wird nach Zustabdsänderung geprüft, rpm berchnet und adaptive Threshold
double Lightsensor::updateState() {

  //State schwarz oder weiß prüfen
  currentStateWhite = (lichtwert > threshold);
  //var für adaptive TH
  Sum += lichtwert;
  values += 1;
  cnt += 1;
  //Serial.println("lichtwert");
  //Serial.println("threshold");
  
  //wenn Zustandsänderung
  if (currentStateWhite != lastStateWhite) {
    StateChanges += 1;
    //Adaptive TH Algo
    Avg = Sum / values;
    weightedThresh = (Avg + LastAvg) / 2;
    threshold = (1.2 * threshold + 0.8 * weightedThresh) / 2;  //Faktoren der TH anpassbar, jenachdem ob man alten Messungen oder neuer Messung vertraut. Faktoren sollten zusammen = 2 ergeben
    values = 0;
    Sum = 0;
    LastAvg = Avg;
    //Serial.print("Abtastungen pro StateChange");
    //Serial.println(cnt);
    cnt = 0;
    timeDifference = millis() - lastStateChangeTime;

    //RPM Berechnung
    if (timeDifference > t_min) {
      //Serial.println()
      //Serial.print("nhalbe ");
      //Serial.println(StateChanges);


      //rpm = 60000.0 * StateChanges / (timeDifference * 2); rpm without moving average

      //Moving average with 10 intervals of t_min
      if (SM.size() < 10){
        SM.push_back(StateChanges);
        Int.push_back(timeDifference);
      }else {
        SM.push_back(StateChanges);
        Int.push_back(timeDifference);
        SM.pop_front();
        Int.pop_front();
      }
      SumIntt = sumList(Int);
      SumSM = sumList1(SM);

      //moving average rpm
      rpm_multi_Cont = 60000.0 * (SumSM) / (SumIntt * 2);
  
      lastStateChangeTime = millis();
      StateChanges = 0;
    }
  }

  lastStateWhite = currentStateWhite;
  return rpm_multi_Cont;
}

// helpfunc
double Lightsensor::sumList(const std::list<double>& myList) {
    double summinho = 0;
    for (double valueX : myList) {
        summinho += valueX;
    }
    return summinho;
}
// helpfunc
int Lightsensor::sumList1(const std::list<int>& myList) {
    int ksum = 0;
    for (int valuet : myList) {
        ksum += valuet;
    }
    return ksum;
}

// Plausibilitätskriterium 2
void Lightsensor::checkPlausibleRPM() {
  if (State != 2) {
    if (rpm == 0) {
      State = 0;
    } else {
      State = 1;
    }
  }
  if (rpm < 0 || rpm > maxPlausibleRPM) {
    ErrorState2 = true;
    State = 2;
  }
}

//resetsRpm if no update for some time
void Lightsensor::resetRPMIfStalled(unsigned long currentTime) {
  if ((currentTime - lastStateChangeTime) > timeUntilNequals0) {
    rpm = 0;
  }
}

//-----------get methoden----------

String Lightsensor::getSensorId() {
  return sensorId;
}

bool Lightsensor::getErrorState1() {
  return ErrorState1;
}

bool Lightsensor::getErrorState2() {
  return ErrorState2;
}

int Lightsensor::getState() {
  return State;
}
