#include <Arduino.h>


//-------- help var ----------
double minBelichtungszeit = 20; // in ms
unsigned long ContiniousStartTime;
unsigned long OneShotStartTime;
unsigned long loopStartTime;
int pixelsClassifiedAsFaser;
//------- help funcs --------------

void sendData(String SensorId, double SensorValue) {
  Serial.print(SensorId);
  Serial.print(",");
  Serial.println(SensorValue);
}

void sendTime(double timeStamp) {
  Serial.print("TimeStamp,");
  Serial.println(timeStamp); // '3' specifies three decimal places
}

void sendCurrentTime() {
  double timeStamp = millis() / 1000.0;
  sendTime(timeStamp);
}
