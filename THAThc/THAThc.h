#ifndef THAThc_H
# define THAThc_H
# include <Arduino.h>

/*
 * The two following constants calibrate the ADC for THE ANALOG THING in question. The
 * value ADC_PLUS is determined by connecting the X-jack on the front panel to +1 and
 * reading the first ADC. ADC_MINUS is determined in the same fashion but using -1.
 */
#define ADC_PLUS            545
#define ADC_MINUS           130

#include <TimerThree.h>
#include <TimerFive.h>

#ifndef TRUE
#define TRUE 1
#define FALSE !TRUE
#endif

#define VERSION "1.0"
#define STRING_LENGTH       133
#define SHORT_STRING_LENGTH 41
#define ENABLE              2                 // Hybrid enable control line
#define MIC                 3                 // MIC control line
#define MOP                 4                 // MOP control line
#define IC                  0                 // Mode IC
#define OP                  1                 // Mode OP
#define HALT                2                 // Mode HALT
#define DEFAULT_OP_TIME     1000              // Default OP-time in milliseconds
#define DEFAULT_IC_TIME     10                // Default IC-time in milliseconds
#define DEFAULT_CHANNELS    1                 // Number of _channels per default
#define MAX_CHANNELS        4                 // Maximum number of ADC _channels
#define MIN_INTERVAL        1                 // Minimum sample _interval in milliseconds
#define MIN_SAMPLE_TIME     110               // Minimum sample time of AnalogRead in microseconds
#define MAX_DATA            2048              // Maximum size of data buffer for data logger

#define STATE_IDLE          0                 // States of the control _state machine
#define STATE_SR_START      1
#define STATE_SR_IC         2
#define STATE_SR_OP         3

class THAThc_ {
  public:

    static THAThc_ &getInstance();
    THAThc_(const THAThc_ &) = delete;
    THAThc_ &operator = (const THAThc_ &) = delete;

    static void _state_machine() {
      return getInstance().state_machine();
    }

    static void _data_logger() {
      return getInstance().data_logger();
    }

    void arm(),                               // Arm the data logger
      begin(),
      block(),                                // Block until the current single run has finished
      disable(),                              // Disable the hybrid controller (enables the MODE switch of the THAT)
      enable(),                               // Enable the hybrid controller (disables the MODE switch of the THAT)
      halt(),                                 // Switch the analog computer to HALT mode
      ic(),                                   // Switch the analog computer to IC mode
      op(),                                   // Switch the analog computer to OP mode
      get_status(char *),                     // Get current status of the hybrid controller
      rep(),                                  // Perform repetitive IC/OP cycles with times set by set_ic_time, set_op_time
      single_run(),                           // Perform a single IC/OP cycle with times set by set_ic_time, set_op_time
      shell(),                                // Interactive shell
      state_machine(),                        // Central state machine
      sample_adc(float *),                    // Sample all ADC channels and return results as float values
      sample_adc_raw(unsigned int *),         // Sample all ADC channels and return the raw values
      data_logger(),                          // Data logger (timer controlled during single run if armed
      read_data();                            // Return data from the last single run with armed data logger
    unsigned long set_channels(unsigned int), // Set number of channels to be sampled by each call to sample_adc
      set_ic_time(unsigned long),             // Set IC time in milliseconds
      set_op_time(unsigned long);             // Set OP time in milliseconds
    unsigned int set_adc_minus(unsigned int), // Set value corresponding to a sampled value of -1
      set_adc_plus(unsigned int);             // Set value corresponding to a sampled value of +1
  private:
    THAThc_() = default;                      // Constructor
    unsigned long _op_time,                   // OP time in milliseconds
      _ic_time,                               // IC time in milliseconds
      _interval;                              // Sampling interval in milliseconds
    unsigned int _state,                      // Central state for the state machine
      _repop,                                 // Flag for repetitive operation
      _shell,                                 // TRUE if shell is running
      _channels,                              // Number of channels to be sampled (<= 4)
      _armed,                                 // Flag for arming the data logger
      _adc_plus,                              // Value corresponding to a sampled value of +1
      _adc_minus,                             // Value corresponding to a sampled value of -1
      _data_index,                            // Index into the data logger buffer
      _data[MAX_DATA];                        // Buffer for data logger
};

extern THAThc_ &THAThc;
#endif
