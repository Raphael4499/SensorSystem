#ifndef LINESCAN_H
#define LINESCAN_H

#include <Arduino.h>
class LineScan {
  private:
    int CLKpin;
    int SIpin;
    int AOpin;
    short dataD1[128];
    int edgeInterval;
    int midInterval;
    bool enable_dataProcessing;
    double width;
    double pixelLength;
    int faserBandLength;
    int faserCounter;
    int minInRow;
    String sensorId;
    unsigned int sampleInterval; // Abtastintervall in Millisekunden
    unsigned long lastSampleTime = 0;
    unsigned int exposureTime; // Belichtungszeit in Mikrosekunden
    double maxPixelValue = 1023.0; // Maximaler Wert des Analog-Digital-Wandlers
    double minPixelValue = 0.0; // Minimaler Wert des Analog-Digital-Wandlers
    bool ErrorState2 = false;
    bool ErrorState3 = false;
    int State = 0; // 0 = default/active, 2 = error
    double faser_treshhold;

  public:
    LineScan(int clk, int si, int ao, unsigned int sampleRateHz, int edgeInt, int midInt, String id);
    void setup();
    double Continious();
    double OneShot();
    String getSensorId();
    bool getErrorState3();
    bool getErrorState2();
    int getState();
    void resetErrors();
    void useSim();
    void automateTresholdCalculation();
    void limitExceptions();
    void GarbageClockOut();

  private:
    void ClockPulse();
    void ReadAnalog();
    void calculateFaserValues();
    void Initialize();
    void checkPixelValue(int pixelValue);
    void checkWidth();

    
};

#endif // LINESCAN_H
