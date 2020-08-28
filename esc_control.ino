/*
    Yuriy Movchan 17/08/2020

    ESC throtle calculator and motor control
*/

// #define ESC_CONTROL_DEBUG 1

#define RC_THROTLE_MIN 1088                      // Minimum throtle PWM
#define RC_THROTLE_MID 1508                      // Middle throtle PWM
#define RC_THROTLE_MAX 1928                      // Maximum throtle PWM

#define RC_THROTLE_DEADZONE 2000                 // Signal lost marker

#define ESC_THROTLE_LEFT_ADJUST_TO_ZERO -23       // Value need to add to RC PWM to allow adjust with middle ESC
#define ESC_THROTLE_RIGHT_ADJUST_TO_ZERO -28      // Value need to add to RC PWM to allow adjust with ESC

// Define ESC motor control ration to define balance between left and right. The value can be between [1..2]
#define ESC_CONTROL_RATION 1.5

// Define maximum allowed ESC speed in percent
#define ESC_CONTROL_MAX_SPEED_PERCENT 50

// Default values to stop motors
const int escLeftStopPwm = RC_THROTLE_MID + ESC_THROTLE_LEFT_ADJUST_TO_ZERO;
const int escRightStopPwm = RC_THROTLE_MID + ESC_THROTLE_RIGHT_ADJUST_TO_ZERO;

// Current values
int escLeftPwm = escLeftStopPwm;
int escRightPwm = escRightStopPwm;

void caluclateEscThrotle(int inputThrotle, int inputBalance, int inputThrotleRange) {
  unsigned int currentEscLeftPwm, currentEscRightPwm;
  unsigned balancePwmChange;
  int inputBalancePercent;
  unsigned int throtlePercent;
  unsigned int rcThrotleRangeMin, rcThrotleRangeMax;

  // Check if receiver lost signal
  if (inputThrotle >= RC_THROTLE_DEADZONE) {
    // Stop motors
    escLeftPwm = escLeftStopPwm;
    escRightPwm = escRightStopPwm;
    return;
  }

  // Make sure that all intput value belong to right ranges
  inputThrotle = constrain(inputThrotle, RC_THROTLE_MIN, RC_THROTLE_MAX);
  inputThrotleRange = constrain(inputThrotleRange, RC_THROTLE_MIN, RC_THROTLE_MAX);
  inputBalance = constrain(inputBalance, RC_THROTLE_MIN, RC_THROTLE_MAX);

  // Convert balance to percent from PWM
  inputBalancePercent = map(inputBalance, RC_THROTLE_MIN, RC_THROTLE_MAX, -100, 100);

  // Set same speed
  currentEscLeftPwm = inputThrotle;
  currentEscRightPwm = inputThrotle;

  if (inputThrotle > RC_THROTLE_MID) { // Reverse driving
    // Update according to balance
    if (inputBalancePercent > 0) { // Stick to right
      // Reduce right track speed
      balancePwmChange = (currentEscRightPwm - RC_THROTLE_MID) * inputBalancePercent / 100;
      currentEscRightPwm = currentEscRightPwm - balancePwmChange * ESC_CONTROL_RATION;
    } else { // Stick to left
      // Reduce left track speed
      balancePwmChange = (RC_THROTLE_MID - currentEscLeftPwm) * inputBalancePercent / 100;
      currentEscLeftPwm = currentEscLeftPwm - balancePwmChange * ESC_CONTROL_RATION;
    }
  } else if (inputThrotle < RC_THROTLE_MID) { // Forward driving
    // Update according to balance
    if (inputBalancePercent > 0) { // Stick to right
      // Reduce right track speed
      balancePwmChange = (RC_THROTLE_MID - currentEscRightPwm) * inputBalancePercent / 100;
      currentEscRightPwm = currentEscRightPwm + balancePwmChange * ESC_CONTROL_RATION;
    } else { // Stick to left
      // Reduce left track speed
      balancePwmChange = (currentEscLeftPwm - RC_THROTLE_MID) * inputBalancePercent / 100;
      currentEscLeftPwm = currentEscLeftPwm + balancePwmChange * ESC_CONTROL_RATION;
    }
  }

  // Calculate current speed percent based on one of the 3 stick positions
  if (inputThrotleRange > RC_THROTLE_MID  + 100) {
    // 1/3 of maximum throtle
    throtlePercent = ESC_CONTROL_MAX_SPEED_PERCENT / 3;
  } else if (inputThrotleRange < RC_THROTLE_MID  - 100) {
    // Max allowed throtle
    throtlePercent = ESC_CONTROL_MAX_SPEED_PERCENT;
  } else {
    // 2/3 of maximum throtle
    throtlePercent = ESC_CONTROL_MAX_SPEED_PERCENT * 2 / 3;
  }

  // Apply throtle percent
  rcThrotleRangeMin = RC_THROTLE_MIN + (RC_THROTLE_MID - RC_THROTLE_MIN) * (100 - throtlePercent) / 100;
  rcThrotleRangeMax = RC_THROTLE_MAX - (RC_THROTLE_MAX - RC_THROTLE_MID) * (100 - throtlePercent) / 100;

  currentEscLeftPwm = map(currentEscLeftPwm, RC_THROTLE_MIN, RC_THROTLE_MAX, rcThrotleRangeMin, rcThrotleRangeMax);
  currentEscRightPwm = map(currentEscRightPwm, RC_THROTLE_MIN, RC_THROTLE_MAX, rcThrotleRangeMin, rcThrotleRangeMax);


  // Adjust PWM reciever and ESC controllers
  currentEscLeftPwm += ESC_THROTLE_LEFT_ADJUST_TO_ZERO;
  currentEscRightPwm += ESC_THROTLE_RIGHT_ADJUST_TO_ZERO;

  escLeftPwm = currentEscLeftPwm;
  escRightPwm = currentEscRightPwm;

#ifdef ESC_CONTROL_DEBUG
  Serial.print(escLeftPwm);
  Serial.print(" = ");
  Serial.print(escRightPwm);
  Serial.print("\n");
#endif
}

void setEscThrotle() {
#ifdef PWM_LEFT_MOTOR_ENABLE
  writeMicroseconds_pwmMotor(RC_SERVO_LEFT_IDX, escLeftPwm);
#endif
#ifdef PWM_RIGHT_MOTOR_ENABLE
  writeMicroseconds_pwmMotor(RC_SERVO_RIGHT_IDX, escRightPwm);
#endif
}
