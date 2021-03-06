#include <FastLED.h>

/*
    Yuriy Movchan 15/08/2020

    Main process
*/

unsigned long now;          // timing variables to update data at a regular interval
unsigned long rc_update;
unsigned long headlight_update;
const int channels = 6;     // specify the number of receiver channels
float RC_in[channels];      // an array to store the calibrated input from receiver

// LEFT MOTOR STOP PWM [1492:1479], Range 14, Middle: 1485.5
// RIGHT MOTOR STOP PWM [1487:1473], Rnage 15, Middle: 1480

// Assign servo indexes (PWM output channels indexes)
#define RC_SERVO_LEFT_IDX  0
#define RC_SERVO_RIGHT_IDX 1
#define RC_CAM_SERVO1_IDX  2
#define RC_CAM_SERVO2_IDX  3
#define RC_FRAME_SPACE_IDX 4

// Assign channels (PWM output pins)
#define RC_SERVO_LEFT_PIN 3  // ESC
#define RC_SERVO_RIGHT_PIN 5 // ESC
#define RC_CAM_SERVO1_PIN 2  // Servo 1
#define RC_CAM_SERVO2_PIN 4  // Servo 2

// Last PWM index output frame space. Needed to send PWN with 50HZ frequency
#define RC_FRAME_SPACE_DELAY (RC_FRAME_SPACE_IDX + 1) * 2000 // Maximum wait time for framespace

// Define used hardware
#define PWM_LEFT_MOTOR_ENABLE 1
#define PWM_RIGHT_MOTOR_ENABLE 1

// Define special modes
//#define PWM_MOTOR_SETUP 1

void setup()  {
  Serial.begin(115200);

  // Configure headlight
  configureHeadlight();

  // Configure RC reciever code to read signal from A0..A5
  setup_pwmRead();

  // Attach servo objects, these will generate the correct pulses for driving ESC, servos or other devices
#ifdef PWM_LEFT_MOTOR_ENABLE
  attach_pwmMotor(RC_SERVO_LEFT_IDX, RC_SERVO_LEFT_PIN);
#endif
#ifdef PWM_RIGHT_MOTOR_ENABLE
  attach_pwmMotor(RC_SERVO_RIGHT_IDX, RC_SERVO_RIGHT_PIN);
#endif
  attach_pwmMotor(RC_CAM_SERVO1_IDX, RC_CAM_SERVO1_PIN);
  attach_pwmMotor(RC_CAM_SERVO2_IDX, RC_CAM_SERVO2_PIN);

  // Lets set a standard rate of 50 Hz by setting a frame space of 10 * 2000 = 2 ESCs + 2 Servos + 6 times 2000
  setFrameSpaceA_pwmMotor(RC_FRAME_SPACE_IDX, RC_FRAME_SPACE_DELAY);

#ifdef PWM_MOTOR_SETUP
  // Enable ESC control via console commands
  setup_PwmMotorSetup();
#endif

  // Enable signal PWM generator on D2..D4 pins
  begin_pwmMotor();
}


void loop()  {
  int unThrottleIn = 0;
  int unThrottlePercentIn = 0;
  int unLeftRightIn = 0;
  int unColorIn = 0;

  now = millis();

#ifndef PWM_MOTOR_SETUP
  if (RC_avail() || (now - rc_update > 30)) { // if RC data is available or 25ms has passed since last update (adjust to suit frame rate of receiver)
    rc_update = now;

/*
    print_RCpwm();          // uncommment to print raw data from receiver to serial
    for (int i = 0; i < channels; i++) {    // run through each RC channel
      int CH = i + 1;

      RC_in[i] = RC_decode(CH);             // decode receiver channel and apply failsafe

      print_decimal2percentage(RC_in[i]);   // uncomment to print calibrated receiver input (+-100%) to serial
    }
*/
    if (PWM_read(1)) {
      unLeftRightIn = PWM();
    }
    if (PWM_read(3)) {
      unThrottleIn = PWM();
    }
    if (PWM_read(5)) {
      unThrottlePercentIn = PWM();
    }

    // Call ESC update methods only if all params are ready
    if ((unThrottleIn != 0) && (unLeftRightIn != 0) && (unThrottlePercentIn != 0)) {
      caluclateEscThrotle(unThrottleIn, unLeftRightIn, unThrottlePercentIn);
      setEscThrotle();
    }

    // Update color only when we read PWM signal and are generating last FRAME_SPACE
    if ((now - headlight_update > 50) && PWM_read(6) && (getCurrentOutputChannelA() == RC_FRAME_SPACE_IDX)) {
      unColorIn = PWM();
      calculateHeadlightColor(unColorIn);

      if (getCurrentOutputChannelA() == RC_FRAME_SPACE_IDX) {
        setCalculatedHeadlightColor();
        headlight_update = now;
      }
    }
  }

#else
  loop_PwmMotorSetup();
#endif
}
