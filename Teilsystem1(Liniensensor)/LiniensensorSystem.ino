

#include "LiniensensorSystem.h"
#include "LineScan.h"

// Initialize LINIENSENSOR
LineScan linescan1(53, 52, A0, 1000, 15, 14, "A"); //(int clkPIN (53 -> C), int siPIN (52 -> B), int aoPIN (A0 -> A), sampleRateHz, edgeIntervall für Automated Fasserklassifizierung, midIntervall für Automated Fasserklassifizierung, Sensorschlüssel)

// setup the system
void setup() {
  
  Serial.begin(500000); //baudrate -> darauf achten diese auch in der konsole zu wählen
  while (!Serial) {
    ; // Warte auf die serielle Verbindung
  }
  Serial.println("--------------------------------------------------------------------"); // Signalisiert das System startet
  linescan1.setup(); //setup für linescan1
}


//main loop der immer läuft
void loop() {
  loopStartTime = micros();

  UseContiniousMode();
  //UseOneShotMode();

   //-> verlangsamerung des loops für debugging oder kleinere Abtastrate
  while (micros() - loopStartTime < 0) { // 2000000 us = 2000 ms
    // Keine Operation, um die Zeit bis zur nächsten Loop-Iteration zu warten
  } 
  
}

//-------- ContiniousMode ----------
void UseContiniousMode(){

  ContiniousStartTime = millis();
  pixelsClassifiedAsFaser = linescan1.Continious();
  //sendData(linescan1.getSensorId(), pixelsClassifiedAsFaser); ///deactivate when using useSim()
  //warten bis mindeste Bleichtungszeit erreicht ist
  while (micros() - ContiniousStartTime < minBelichtungszeit) { // 2000000 us = 2000 ms
    // Keine Operation, um die Zeit bis zur nächsten Loop-Iteration zu warten
  } 
  linescan1.useSim();// -> für Live Python Evaluation

}

//-------- OneShotMode ----------
void UseOneShotMode(){

  OneShotStartTime = millis();
  pixelsClassifiedAsFaser = linescan1.OneShot();
  sendData(linescan1.getSensorId(), pixelsClassifiedAsFaser);

}
