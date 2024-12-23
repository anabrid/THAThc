#include "THAThc.h"
#define BAUD_RATE             250000
#define SHELL                 // Runs a shell if defined

void setup() {
  Serial.begin(BAUD_RATE);
  THAThc.begin();
}

void loop() {
#ifdef SHELL
  THAThc.shell();
#else
  THAThc.enable();            // Configure the hybrid controller
  THAThc.set_ic_time(1);
  THAThc.set_op_time(2);
  THAThc.set_channels(1);

  char result[STRING_LENGTH]; // Print current settings
  THAThc.get_status(result);
  Serial.print(String(result) + "\n");
  
  // Perform repetitive runs and sample result at the end of each run
  for (unsigned long i = 0; i < 20000; i++) {
    THAThc.single_run();
    THAThc.block();

    // We are only interested in the last value of the time integration
    float result;
    THAThc.sample_adc(&result);
    Serial.print(String(i) + "\t");
    Serial.print(result, 3);
    Serial.print("\n");
  }

  for( ; ; delay(1000));      // Stop
#endif
}
