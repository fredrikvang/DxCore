/***********************************************************************|
| Configurable Custom Logic library                                     |
| Developed in 2019 by MCUdude. https://github.com/MCUdude/             |
| Example by Spence Konde                                               |
|                                                                       |
| LatchNoSeq.ino - Getting "RS Latch" like behavior with a single even  |
| numbered LUT.                                                         |
|                                                                       |
| In this example we use the configurable logic peripherals in AVR      |
| to act as a "latch" WITHOUT using both LUTs and the sequencer         |
| For the even-numbered logic block(s) we can simply use the feedback   |
| input. Otherwise we need to use the event system.                     |
|                                           3-input     truth table:    |
| We use CCL LUT event as our "feedback",   |PA2|PA1|CCL| Y |           |
| PA1 is RESET and PA2 is SET, both         |---|---|---|---|           |
| active low                                | 0 | 0 | 0 | 0 |           |
| Connect a button between those and Gnd    | 0 | 0 | 1 | 1 |           |
| Pressing button on PA2 will set output    | 0 | 1 | 0 | 1 |           |
| HIGH and pressing button on PA1 will set  | 0 | 1 | 1 | 1 |           |
| output LOW, and pressing neither will do  | 1 | 0 | 0 | 0 |           |
| nothing.                                  | 1 | 0 | 1 | 0 |           |
| We could even then fire an interrupt from | 1 | 1 | 0 | 0 |           |
| that pin                                  | 1 | 1 | 1 | 1 |           |
|                                                                       |
| The sky (well, and the number of LUTs) is the limit!!                 |
|                                                                       |
| Warning: This involves EVSYS, which is very different on tiny0/1 vs   |
| everything else. Hence, many #ifdefs.                                 |
|***********************************************************************/

#include <Logic.h>

void setup() {
  // Initialize logic block 0
  // Logic block 0 has three inputs, PA0, PA1 and PA2.
  // Because PA0 is shared with the UPDI pin on tinyAVR parts
  // we use the other two as our button inputs.
  // It outputs on the LUT0 OUT pin - PA4 (alt. PB4) on ATtiny
  // or PA3 (alt PA6) everywhere else.

  Logic0.enable = true;                   // Enable logic block 0
  Logic0.input0 = in::feedback;
  Logic0.input1 = in::input_pullup;       // PA1 as input1 (RESET)
  Logic0.input2 = in::input_pullup;       // PA2 as input2 (SET)
  // Logic0.output_swap = out::pin_swap;   // Uncomment this line to route the output to alternate location, if available.
  Logic0.output = out::enable;            // Enable logic block 0 output pin (see pinout chart)
  Logic0.filter = filter::disable;        // No output filter enabled
  Logic0.truth = 0x8E;                    // Set truth table - HIGH only if both high
  Logic0.init();                          // Initialize logic block 0

  // Example for odd-number block where we can't use feedback to get it's own output
  #ifdef EVSYS_CHANNEL0                             // means it's not a 0/1-series
  EVSYS.CHANNEL0 = EVSYS_CHANNEL0_CCL_LUT1_gc;
  EVSYS.USERCCLLUT1A = EVSYS_USER_CHANNEL0_gc;
  #else                                             // it's a tinyAVR 0/1 - EVSYS is weird there
  EVSYS.ASYNCCH0 = EVSYS_ASYNCCH0_CCL_LUT1_gc;      // Use CCL LUT1 as event generator
  EVSYS.ASYNCUSER4 = EVSYS_ASYNCUSER2_ASYNCCH0_gc;  // ASYNCUSER4 is LUT1 event 0
  #endif

  Logic1.input0 = in::event_a;          // same distribution of inputs (though there isn't the tinyAVR PA0 issue forcing it here)
  Logic1.input1 = in::input_pullup;     // get it from LUT 1 event a or event 0 (tinyAVR 0/1 documentation calls them 0 and 1 - Logic.h accepts both for all parts)
  Logic1.input2 = in::input_pullup;
  // Logic0.output_swap = out::pin_swap; // Uncomment this line to route the output to alternate location, if available.
  Logic1.output = out::enable;          // Enable logic block 1 output pin (see pinout chart)
  Logic1.filter = filter::disable;      // No output filter enabled
  Logic1.truth = 0x8E;                  // Set truth table - HIGH only if both high
  Logic0.init();                        // Initialize logic block 0
  Logic::start();                       // Start the AVR logic hardware
}

void loop() {
  // When using configurable custom logic the CPU isn't doing anything
}
