#include <functions.h>
#include <Configuration.h>
#include <StepperDrive.h>
#include <Encoder.h>
#include <Core.h>
#include <UserInterface.h>


TaskHandle_t processorCore2;


// Feed table factory
FeedTableFactory feedTableFactory;

// Control Panel driver
ControlPanel controlPanel;

// Encoder driver
Encoder encoder;

// Stepper driver
StepperDrive stepperDrive;

// Core engine
Core core(&encoder, &stepperDrive);

// User interface
UserInterface userInterface(&controlPanel, &core, &feedTableFactory);

// timer 0
hw_timer_t * timer0 = NULL;
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;
uint32_t cp0_regs0[18]; 

// timer 1
hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;
uint32_t cp0_regs1[18]; 



void IRAM_ATTR onTimer0() {
  portENTER_CRITICAL_ISR(&timerMux0);

  // https://esp32.com/viewtopic.php?t=1292
  // enable FPU
  xthal_set_cpenable(1);
  // Save FPU registers
  xthal_save_cp0(cp0_regs0);

  core.ISR();

  // Restore FPU
  xthal_restore_cp0(cp0_regs0);
  // and turn it back off
  xthal_set_cpenable(0);

  portEXIT_CRITICAL_ISR(&timerMux0);
}


void IRAM_ATTR onTimer1() {
  static Uint64 previous = 0;
  portENTER_CRITICAL_ISR(&timerMux1);

  // https://esp32.com/viewtopic.php?t=1292
  // enable FPU
  xthal_set_cpenable(1);
  // Save FPU registers
  xthal_save_cp0(cp0_regs1);

  Uint64 current = encoder.getPosition();
  Uint64 count = (current > previous) ? current - previous : previous - current;
  encoder.setRPM(count * 60 * RPM_CALC_RATE_HZ / ENCODER_RESOLUTION);
  previous = current;
//  Serial.println(encoder.getRPM());

  // Restore FPU
  xthal_restore_cp0(cp0_regs1);
  // and turn it back off
  xthal_set_cpenable(0);

  portEXIT_CRITICAL_ISR(&timerMux1);
}


void setup() {
  Serial.begin(115200);

  // start second core
 // xTaskCreatePinnedToCore(runOnProcessorCore2, "ProcessorCore2", STACK_SIZE_PROCESSOR_CORE2, NULL, 1, &processorCore2, 0);

  // setup timer
  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &onTimer0, true);
  timerAlarmWrite(timer0, STEPPER_CYCLE_US, true);
  timerAlarmEnable(timer0);

    // setup timer
  timer1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 1000000 / RPM_CALC_RATE_HZ, true);
  timerAlarmEnable(timer1);

  // Initialize peripherals and pins
  stepperDrive.initHardware();
}


void loop() {
  userInterface.loop();
  delayMicroseconds(1000000 / UI_REFRESH_RATE_HZ);
}
