unsigned long now;          // timing variables to update data at a regular interval
unsigned long rc_update;
const int channels = 6;     // specify the number of receiver channels
float RC_in[channels];      // an array to store the calibrated input from receiver

// Assign servo indexes
#define RC_SERVO_LEFT_IDX  0
#define RC_SERVO_RIGHT_IDX 1
#define RC_CAM_SERVO1_IDX  2
#define RC_CAM_SERVO2_IDX  3
#define RC_FRAME_SPACE_IDX 4

// Assign channels out pins
#define RC_SERVO_LEFT_PIN 3  // ESC
#define RC_SERVO_RIGHT_PIN 5 // ESC
#define RC_CAM_SERVO1_PIN 2  // Servo 1
#define RC_CAM_SERVO2_PIN 4  // Servo 2

#define RC_FRAME_SPACE_DELAY (RC_FRAME_SPACE_IDX + 1) * 2000 // Maximum wait time for framespace

void setup()  {
  setup_pwmRead();

  Serial.begin(115200);

  // Attach servo objects, these will generate the correct pulses for driving Electronic speed controllers, servos or other devices
  // designed to interface directly with RC Receivers
  attach_pwmMotor(RC_SERVO_LEFT_IDX, RC_SERVO_LEFT_PIN);
  attach_pwmMotor(RC_SERVO_RIGHT_IDX, RC_SERVO_RIGHT_PIN);
  attach_pwmMotor(RC_CAM_SERVO1_IDX, RC_CAM_SERVO1_PIN);
  attach_pwmMotor(RC_CAM_SERVO2_IDX, RC_CAM_SERVO2_PIN);

  // Lets set a standard rate of 50 Hz by setting a frame space of 10 * 2000 = 4 Servos + 6 times 2000
  setFrameSpaceA_pwmMotor(RC_FRAME_SPACE_IDX, RC_FRAME_SPACE_DELAY);

  begin_pwmMotor();
}

void loop()  {
  now = millis();
  static uint16_t unThrottleIn;

  if (RC_avail() || now - rc_update > 25) { // if RC data is available or 25ms has passed since last update (adjust to suit frame rate of receiver)

    rc_update = now;

//    print_RCpwm();          // uncommment to print raw data from receiver to serial
    if (PWM_read(2)) {
      unThrottleIn = PWM();
    writeMicroseconds_pwmMotor(RC_SERVO_LEFT_IDX,unThrottleIn);
    writeMicroseconds_pwmMotor(RC_SERVO_RIGHT_IDX,unThrottleIn-5);
    Serial.println(unThrottleIn);
    }
    /*
          for (int i = 0; i<channels; i++){       // run through each RC channel
            int CH = i+1;

            RC_in[i] = RC_decode(CH);             // decode receiver channel and apply failsafe

            print_decimal2percentage(RC_in[i]);   // uncomment to print calibrated receiver input (+-100%) to serial
          }
    */
    Serial.println();         // uncomment when printing calibrated receiver input to serial.
  }
}
