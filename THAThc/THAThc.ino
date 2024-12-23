#include "THAThc.h"
#define BAUD_RATE             250000

void setup() {
  Serial.begin(BAUD_RATE);
  THAThc.begin();
}

void loop() {
  THAThc.shell();
}
