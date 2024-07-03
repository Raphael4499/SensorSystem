#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H

#include <Arduino.h>
#include <StandardCplusplus.h>

#include <set>


class Lightsensor {
  private:
    int sensorPin; // Pin für den Lichtsensor
    int lichtwert = 0; // Variable zum Speichern des Lichtwerts
    
    bool lastStateWhite; // Speichert den letzten Zustand (Weiß/Schwarz)
    unsigned long lastStateChangeTime = 0; // Zeitstempel für den letzten Zustandswechsel
    unsigned long letzteMessung = 0; // Zeitstempel für die letzte Ausgabe
    unsigned long lastSampleTime = 0; // Zeitstempel für die letzte Abtastung
    unsigned long StateChangeTime;
    unsigned long t_min; //angabe in ms -> alle t_min ms wird rpm berechnet
    double timeDifference;
    int rev = 0; // Zähler für die Anzahl der Umdrehungen
    double rpm = 0.0; // Variable zum Speichern der Drehzahl
    int calibrationMode;
    double calibrationTime = 1900; // in ms
    String sensorId; // ID des Sensors
    unsigned int sampleInterval; // Abtastintervall in Millisekunden
    double printTime = 1000;
    bool currentStateWhite;
    double timeUntilNequals0 = 5000; // 1/(5/2) * 60 = 24 // if theoretic rpm < 24 -> n drops to 0
    bool ErrorState1 = false;
    bool ErrorState2 = false;
    double maxPlausibleRPM;
    int State = 0; // 0 = default,  = active, 2 = Ecrror
    int StateChanges;
    double firstHalfAverage = 0;
    double secondHalfAverage = 0;
    long long Sum =0;
    int values = 0;
    double Avg = 0;
    double LastAvg = 0;
    double weightedThresh = 0;
    static Lightsensor* instance;  // Singleton instance for ISR access
    
    
    int LastSC = 0;
    int LastLastSC = 0;
    int LastLastLastSC = 0;
    double LastInt = 0;
    double LastLastInt = 0;
    double LastLastLastInt = 0;
    double SumOfInt;
    double SumOfSC = 0;
    std::set<int> Set;
    int cnt = 0;
    std::list<double> Int;
    std::list<int> SM;

    

  public:
    Lightsensor(int pin, int settedThreshold, int settedCalibrationMode, String id, unsigned int sampleRateHzm, double settedMaxPlausibleRPM, unsigned long setted_tmin);
    void setup();
    double rpmCont;
    double threshold; // Schwellenwert für Weiß/Schwarz-Erkennung
    void calibration1();
    void calibration2();
    double update();
    void readSensorValue();
    void checkForErrors();
    double updateState();
    void resetRPMIfStalled(unsigned long currentTime);
    void printPeriodically(unsigned long currentTime);
    void checkPlausibleRPM();
    void printValues(bool currentStateWhite);
    void calculateAverages(const std::set<int>& mySet);
    String getSensorId();
    bool getErrorState1();
    bool getErrorState2();
    int getState();
    void resetErrors();
    static void startTimer();
    static void stopTimer();
    static void timerISR();
    double sumList(const std::list<double>& myList);
    int sumList1(const std::list<int>& myList); // Deklaration der Methode sumList1
    double rpm_multi_Cont;
    double SumIntt;
    int SumSM;
};

#endif // LIGHTSENSOR_H
