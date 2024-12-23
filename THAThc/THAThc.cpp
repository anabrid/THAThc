#include <Arduino.h>
#include "THAThc.h"

// Convert an ADC raw value to float:
#define RAW2FLOAT(i) (2. * (i - _adc_minus) / (_adc_plus - _adc_minus) - 1.)

/*
** Local variant of strtok, just better. :-) The first call expects the string to be tokenized as its first argument.
** All subsequent calls only require the second argument to be set. If there is nothing left to be tokenized, a zero pointer
** will be returned. In contrast to strtok this routine will not alter the string to be tokenized since it
** operates on a local copy of this string.
*/
char *tokenize(char *string, char *delimiters) {
  static char local_copy[STRING_LENGTH], *position;
  char *token;

  if (string) { /* Initial call, create a copy of the string pointer */
    strcpy(local_copy, string);
    position = local_copy;
  } else { /* Subsequent call, scan local copy until a delimiter character will be found */
    while (*position && strchr(delimiters, *position)) /* Skip delimiters if there are any at the beginning of the string */
      position++;

    token = position; /* Now we are at the beginning of a token (or the end of the string :-) ) */

    if (*position == '\'') { /* Special case: Strings delimited by single quotes won't be split! */
      position++;
      while (*position && *position != '\'')
        position++;
    }

    while (*position) {
      position++;
      if (!*position || strchr(delimiters, *position)) { /* Delimiter found */
        if (*position)
          *position++ = (char) 0; /* Split string copy */
        return token;
      }
    }
  }

  return NULL;
}

THAThc_ &THAThc_::getInstance() {
  static THAThc_ instance;
  return instance;
}

void THAThc_::begin() {                                            // Constructor
  _op_time   = DEFAULT_OP_TIME;
  _ic_time   = DEFAULT_IC_TIME;
  _state     = STATE_IDLE;
  _repop     = FALSE;
  _channels  = DEFAULT_CHANNELS;
  _interval  = MIN_INTERVAL;
  _armed     = FALSE;
  _adc_plus  = ADC_PLUS;
  _adc_minus = ADC_MINUS;
  _shell     = FALSE;

  pinMode(ENABLE, OUTPUT);                                    // Enable control pin
  digitalWrite(ENABLE, HIGH);                                 // Disable the hybrid controller

  pinMode(MIC, OUTPUT);                                       // Mode-IC control pin
  pinMode(MOP, OUTPUT);                                       // Mode-OP control pin

  Timer3.stop();
  Timer5.stop();

  Timer3.attachInterrupt(THAThc_::_state_machine);
  Timer5.attachInterrupt(THAThc_::_data_logger);
}

THAThc_ &THAThc = THAThc.getInstance();

void set_mode(int mode) {
  if (mode == IC) {                                           // Switch the analog computer to IC mode
    digitalWrite(MIC, LOW);
    digitalWrite(MOP, HIGH);
  } else if (mode == OP) {                                    // Switch the analog computer to OP mode
    digitalWrite(MIC, HIGH);
    digitalWrite(MOP, LOW);
  } else if (mode == HALT) {                                  // Switch the analog computer to HALT mode
    digitalWrite(MIC, HIGH);
    digitalWrite(MOP, HIGH);
  }
}

void THAThc_::state_machine() {                                // This implements the central state machine for run/rep-mode.
  unsigned int redo;

  if (_state == STATE_IDLE) {
    Timer3.stop();
    _repop = FALSE;
  }

  do {
    redo = FALSE;

    if (_state == STATE_SR_START) {
      _state = STATE_SR_IC;
      set_mode(IC);
      Timer3.initialize(1000 * (long) _ic_time);
    } else if (_state == STATE_SR_IC) {
      Timer3.stop();
      _state = STATE_SR_OP;
      set_mode(OP);

      if (!_repop && _armed)                                  // Enable data logging if we are in single run mode and if the logger has been _armed.
        Timer5.initialize(_interval);

      Timer3.initialize(1000 * (long) _op_time);              // Set Timer3 to the duration of the OP phase.
    } else if (_state == STATE_SR_OP) {
      if (!_repop) {
        Timer3.stop();
        Timer5.stop();
        _armed = FALSE;
        set_mode(HALT);
        _state = STATE_IDLE;
        if(_shell)
          Serial.print("Single run ended.\n");
      } else {
        _state = STATE_SR_START;
        redo = TRUE;
      }
    }
  } while (redo);
}

void THAThc_::arm() {                                          // Arm the data logger function
  _armed = TRUE;
}

void THAThc_::block() {                                        // Block until the end of a single run
  while (_state != STATE_IDLE)
    delay(1);
}

void THAThc_::disable() {                                      // Disable the hybrid controller
  digitalWrite(ENABLE, HIGH);
}

void THAThc_::enable() {                                       // Enable the hybrid contoller
  ic();
  digitalWrite(ENABLE, LOW);
}

void THAThc_::halt() {                                         // HALT the analog computer
  set_mode(HALT);
  _state = STATE_IDLE;
  _armed = FALSE;
  Timer3.stop();
  Timer5.stop();
}

void THAThc_::ic() {                                           // Set the analog computer to initial condition mode
  set_mode(IC);
  _state = STATE_IDLE;
  _armed = FALSE;
  Timer3.stop();
  Timer5.stop();
}

void THAThc_::op() {                                           // Set the analog computer to operate mode
  set_mode(OP);
  _state = STATE_IDLE;
  _armed = FALSE;
  Timer3.stop();
  Timer5.stop();
}

unsigned long THAThc_::set_channels(unsigned int value) {      // Set number of channels for data logging
  if (value < 1 || value > MAX_CHANNELS)
    return -1;
  return _channels = value;
}

unsigned long THAThc_::set_ic_time(unsigned long value) {      // Set time for initial condition mode (single run/repetitive operation)
  return _ic_time = value;
}

unsigned long THAThc_::set_op_time(unsigned long value) {      // Set time for operation mode (single run/repetitive operation)
  return _op_time = value;
}

unsigned int THAThc_::set_adc_plus(unsigned int value) {      // Set the raw ADC value representing +1
  return _adc_plus = value;
}

unsigned int THAThc_::set_adc_minus(unsigned int value) {     // Set the raw ADC value representing -1
  return _adc_minus = value;
}

void THAThc_::single_run() {                                   // Perform a single IC/OP cycle with times set by set_ic_time, set_op_time
  _state = STATE_SR_START;
  _repop = FALSE;
  if (_armed) {
    _data_index = 0;
    _interval = (unsigned long) (_op_time * 1000) / (unsigned long) (MAX_DATA / _channels);
    if (_interval < _channels * MIN_SAMPLE_TIME)
      _interval = _channels * MIN_SAMPLE_TIME;
    if (_shell)
      Serial.print("interval=" + String(_interval) + "\n");
  }
  state_machine();
}

void THAThc_::get_status(char *result) {                       // Return current state
  sprintf(result, "version=%s,optime=%lu,ictime=%lu,channels=%u,armed=%u,interval=%lu,adcminus=%u,adcplus=%u,dataindex=%u",
          VERSION, _op_time, _ic_time, _channels, _armed, _interval, _adc_minus, _adc_plus, _data_index);
}

void THAThc_::rep() {                                          // Perform repetitive runs (IC/OP) with times set by set_ic_time, set_op_time
  _state = STATE_SR_START;
  _repop = TRUE;
  state_machine();
}

void THAThc_::sample_adc(float *result) {                      // Sample the ADCs once and return results as floats
  static unsigned int port[MAX_CHANNELS] = {A0, A1, A2, A3};

  for (unsigned int i = 0; i < _channels; i++) {
    unsigned int v = analogRead(port[i]);
    *result++ = RAW2FLOAT(v);
  }
}

void THAThc_::sample_adc_raw(unsigned int *result) {           // Sample the ADCs once and return results as floats
  static unsigned int port[MAX_CHANNELS] = {A0, A1, A2, A3};

  for (unsigned int i = 0; i < _channels; i++)
    *result++ = analogRead(port[i]);
}

void THAThc_::data_logger() {                                  // This function is executed by Timer5 if _state == STATE_SR_START and _arm == TRUE
  static unsigned int port[MAX_CHANNELS] = {A0, A1, A2, A3};

  for (unsigned int i = 0; i < _channels; i++)
    _data[_data_index + i] = analogRead(port[i]);
  _data_index += _channels;
}

void THAThc_::read_data() {                                    // Print data gathered by the data logger with one column per channel
  for (unsigned int j = 0; j < _data_index / _channels; j++) {
    for (unsigned int i = 0; i < _channels; i++) {
      unsigned int index = j * _channels + i;
      if (index > _data_index - 1) 
        break;
      Serial.print(RAW2FLOAT(_data[index]), 3);
      if (i < _channels - 1)
        Serial.print(";");
    }
    Serial.print("\n");
  }
}

void THAThc_::shell() {                                        // This is the interactive shell
  char input[STRING_LENGTH], command[SHORT_STRING_LENGTH], value[SHORT_STRING_LENGTH], result[STRING_LENGTH];
  float adc_data[MAX_CHANNELS];

  if (!_shell) {
    Serial.print("Interactive Shell\n-----------------\n\n");
    _shell = TRUE;
  }

  if (Serial.available() > 0) { // Process command
    Serial.readString().toCharArray(input, STRING_LENGTH);

    int last = strlen(input);
    if (input[last - 1] == '\r' or input[last - 1] == '\n')
      input[last - 1] = (char) 0;

    tokenize(input, (char *) 0);
    strcpy(command, tokenize((char *) 0, (char *) " ="));

    if (!strcmp(command, "arm")) {
      arm();
      Serial.print("Armed...\n");
    } else if (!strcmp(command, "channels")) {
      strcpy(value, tokenize((char *) 0, (char *) "="));
      if (set_channels(atoi(value)) < 1)
        Serial.print("Illegal number of _channels!\n");
      else {
        Serial.print("channels=" + String(_channels) + "\n");
      }
    } else if (!strcmp(command, "disable")) { // Disable hybrid mode
      disable();
      Serial.print("Hybrid mode disabled.\n");
    } else if (!strcmp(command, "enable")) {  // Enable hybrid mode
      enable();
      Serial.print("Hybrid mode enabled.\n");
    } else if (!strcmp(command, "halt")) {
      halt();
      Serial.print("HALT\n");
    } else if (!strcmp(command, "help")) {
      Serial.print("\nCommands:\n\
        arm                 Arm the data logger\n\
        channels=<value>    Set the number of _channels to be logged\n\
        disable             Disable the hybrid controller\n\
        enable              Enable the hybrid controller\n\
        halt                Switch THAT into HALT mode\n\
        help                Print this help text\n\
        ic                  Switch THAT to IC mode\n\
        ictime=<value>      Set IC-time to value milliseconds\n\
        op                  Switch THAT to OP mode\n\
        optime=<value>      Set the OP-time to value milliseconds\n\
        read                Read data from last logger operation\n\
        rep                 Start repetitive operation\n\
        run                 Start a single run (with logging if _armed)\n\
        sample              Sample the analog inputs once\n\
        status              Return status information\n\
        \n\Don't forget to 'enable' the hybrid controller!\n\
        \n");
    } else if (!strcmp(command, "ic")) {
      ic();
      Serial.print("IC\n");
    } else if (!strcmp(command, "ictime")) {
      strcpy(value, tokenize((char *) 0, (char *) "="));
      Serial.print("ictime=" + String(set_ic_time(atoi(value))) + "\n");
    } else if (!strcmp(command, "op")) {
      op();
      Serial.print("OP\n");
    } else if (!strcmp(command, "optime")) {
      strcpy(value, tokenize((char *) 0, (char *) "="));
      Serial.print("optime=" + String(set_op_time(atoi(value))) + "\n");
    } else if (!strcmp(command, "read")) {
      read_data();
    } else if (!strcmp(command, "rep")) {
      Serial.print("Repetitive operation...\n");
      rep();
    } else if (!strcmp(command, "run")) {
      Serial.print("Single run...\n");
      single_run();
    } else if (!strcmp(command, "sample")) {
      sample_adc(adc_data);
      for (unsigned int i = 0; i < _channels; i++) {
        Serial.print(adc_data[i], 3);
        if (i < _channels - 1)
          Serial.print(";");
      }
      Serial.print("\n");
    } else if (!strcmp(command, "status")) {
      get_status(result);
      Serial.print(String(result) + "\n");
    } else
      Serial.print("Illegal command >>> " + String(command) + "\n");
  }
}
