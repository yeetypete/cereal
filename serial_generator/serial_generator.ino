#include <math.h>
#define PRECISION 6
#define FREQUENCY 100 // Hz

void generateSerialSignal(int, int, int);

void setup() {
  Serial.begin(9600);
}

void loop() {
  generateSerialSignal(4, 10, 100);
}

void generateSerialSignal(int signals, int min, int max) {
  int rand_delim = random(4);
  double phase_offset = random(PI * pow(10, PRECISION)) / PRECISION;
  char delim;
    if (rand_delim == 0)
      delim = '\t';
    else if (rand_delim == 1)
      delim = ';';
    else if (rand_delim == 2)
      delim = ',';
    else
      delim = ' ';
    unsigned long time = millis();
    double noise = random(min * pow(10, PRECISION), max * pow(10, PRECISION)) /
    pow(10, PRECISION);
    for (int i = 0; i < signals; i++) {
      double signal = noise * sin(time / FREQUENCY + phase_offset);
      Serial.print(signal);
      Serial.print(delim);
    }
    Serial.println();
}
