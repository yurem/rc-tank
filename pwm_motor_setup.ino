/*
 *  Yuriy Movchan 16/08/2020
 *
 *  Utility mehods to help determine when bidirectional motor stop/strt, etc...
 */

// ---------------------------------------------------------------------------
// Customize here pulse lengths as needed
#define MIN_PULSE_LENGTH 1000 // Minimum pulse length in µs
#define MID_PULSE_LENGTH 1500 // Middle pulse length in µs
#define MAX_PULSE_LENGTH 2000 // Maximum pulse length in µs
// ---------------------------------------------------------------------------

uint16_t pwmMotorSetupLeftPWM, pwmMotorSetupRightPWM;

/**
   Initialisation routine
*/
void setup_PwmMotorSetup() {
  setPWM_PwmMotorSetup(MID_PULSE_LENGTH, MID_PULSE_LENGTH);

  displayInstructions_PwmMotorSetup();
}

/**
   Main function
*/
void loop_PwmMotorSetup() {
  if (Serial.available()) {

    switch (Serial.read()) {
      case 'q' : Serial.println("Increasing left PWM");
        setPWM_PwmMotorSetup(++pwmMotorSetupLeftPWM, pwmMotorSetupRightPWM);
        break;

      case 'a' : Serial.println("Decreasing left PWM");
        setPWM_PwmMotorSetup(--pwmMotorSetupLeftPWM, pwmMotorSetupRightPWM);
        break;

      case 'w' : Serial.println("Increasing right PWM");
        setPWM_PwmMotorSetup(pwmMotorSetupLeftPWM, ++pwmMotorSetupRightPWM);
        break;

      case 's' : Serial.println("Decreasing right PWM");
        setPWM_PwmMotorSetup(pwmMotorSetupLeftPWM, --pwmMotorSetupRightPWM);
        break;

      case '0' : Serial.println("Sending minimum throttle");
        setPWM_PwmMotorSetup(MIN_PULSE_LENGTH, MIN_PULSE_LENGTH);
        break;

      case '`' : Serial.println("Sending middle throttle");
        setPWM_PwmMotorSetup(MID_PULSE_LENGTH, MID_PULSE_LENGTH);
        break;

      case '2' : Serial.println("Sending maximum throttle");
        setPWM_PwmMotorSetup(MAX_PULSE_LENGTH, MAX_PULSE_LENGTH);
        break;

      case '3' : Serial.print("Running test in 3");
        delay(1000);
        Serial.print(" 2");
        delay(1000);
        Serial.println(" 1...");
        delay(1000);
        testNormal_PwmMotorSetup();
        break;

      case '4' : Serial.print("Running test in 3");
        delay(1000);
        Serial.print(" 2");
        delay(1000);
        Serial.println(" 1...");
        delay(1000);
        testBidirectionalDown_PwmMotorSetup();
        break;

      case '5' : Serial.print("Running test in 3");
        delay(1000);
        Serial.print(" 2");
        delay(1000);
        Serial.println(" 1...");
        delay(1000);
        testBidirectionalUp_PwmMotorSetup();
        break;
    }
  }


}

/**
   Test function: send min throttle to max throttle to each ESC.
*/
void testNormal_PwmMotorSetup() {
  for (int i = MIN_PULSE_LENGTH; i <= MAX_PULSE_LENGTH; i += 5) {
    Serial.print("Pulse length = ");
    Serial.println(i);

    setPWM_PwmMotorSetup(i, i);

    delay(200);
  }

  Serial.println("STOP");
  setPWM_PwmMotorSetup(MIN_PULSE_LENGTH, MIN_PULSE_LENGTH);
}

/**
   Test function: send min throttle to max throttle to each ESC.
*/
void testBidirectionalDown_PwmMotorSetup() {
  for (int i = MID_PULSE_LENGTH; i >= MIN_PULSE_LENGTH; i -= 5) {
    Serial.print("Pulse length = ");
    Serial.println(i);

    setPWM_PwmMotorSetup(i, i);

    delay(200);
  }

  Serial.println("STOP");
  setPWM_PwmMotorSetup(MID_PULSE_LENGTH, MID_PULSE_LENGTH);
}

/**
   Test function: send min throttle to max throttle to each ESC.
*/
void testBidirectionalUp_PwmMotorSetup() {
  for (int i = MID_PULSE_LENGTH; i <= MAX_PULSE_LENGTH; i += 5) {
    Serial.print("Pulse length = ");
    Serial.println(i);

    setPWM_PwmMotorSetup(i, i);

    delay(200);
  }

  Serial.println("STOP");
  setPWM_PwmMotorSetup(MID_PULSE_LENGTH, MID_PULSE_LENGTH);
}

void setPWM_PwmMotorSetup(uint16_t newPwmMotorSetupLeftPWM, uint16_t newPwmMotorSetupRightPWM) {
  pwmMotorSetupLeftPWM = newPwmMotorSetupLeftPWM;
  pwmMotorSetupRightPWM = newPwmMotorSetupRightPWM;

  Serial.print("Left PWM: ");
  Serial.println(pwmMotorSetupLeftPWM);

  Serial.print("Right PWM: ");
  Serial.println(pwmMotorSetupRightPWM);

#ifdef PWM_LEFT_MOTOR_ENABLE
  writeMicroseconds_pwmMotor(RC_SERVO_LEFT_IDX, pwmMotorSetupLeftPWM);
#endif
#ifdef PWM_RIGHT_MOTOR_ENABLE
  writeMicroseconds_pwmMotor(RC_SERVO_RIGHT_IDX, pwmMotorSetupRightPWM);
#endif
}

/**
   Displays instructions to user
*/
void displayInstructions_PwmMotorSetup() {
  Serial.println("READY - PLEASE SEND INSTRUCTIONS AS FOLLOWING :");
  Serial.println("\tq : Increase left motor PWM");
  Serial.println("\ta : Decrease left motor PWM");
  Serial.println("\tw : Increase right motor PWM");
  Serial.println("\ts : Decrease right motor PWM");
  Serial.println("\t0 : Send min throttle");
  Serial.println("\t1 : Send mid throttle");
  Serial.println("\t2 : Send max throttle");
  Serial.println("\t3 : Run test function from 1000 to 2000 PWM");
  Serial.println("\t4 : Run test function from 1500 to 1000 PWM");
  Serial.println("\t5 : Run test function from 1500 to 2000 PWM");
}
