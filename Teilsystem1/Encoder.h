#ifndef ENCODER_H
#define ENCODER_H
#include <Arduino.h>

class Encoder {
  private:
    volatile long impulse;
    unsigned long lastInterruptTime;
    int interruptPinA;
    int interruptPinB;
    int resolution;
    double drehzahl;
    String encoderId; // String-ID für jeden Encoder
    double calcRate_n = 30; // in ms -> wie oft n berechnet werden soll
    static Encoder* instance0;
    static Encoder* instance1;
    bool ErrorState1 = false;
    int State = 0;
    double maxPlausibleRPM;

  public:
    Encoder(int pinA, int pinB, int res, String id, double settedMaxPlausibleRPM);
    void setup(int instance);
    double update();
    String getSensorId();
    void checkPlausibleRPM();
    bool getErrorState1();
    int getState();
    void resetErrors();

    static void isrA1();
    static void isrB1();
    static void isrA2();
    static void isrB2();

  private:
    void handleIsrA1();
    void handleIsrB1();
    void handleIsrA2();
    void handleIsrB2();
};

#endif // ENCODER_H
