/*
   Yuriy Movchan 15/08/2020

   Based on indea implemented in RCArduinoFastLib by DuaneB is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
   http://rcarduino.blogspot.com
*/
// The first bank of servos uses OC1A - this will disable PWM on digital pin 9 - a small price for 10 fast and smooth servos
// The second bank of servos uses OC1B - this will disable PWM on digital pin 10 - a small price for 10 more fast and smooth servos

// The library blindly pulses all ten servos one and after another
// If you change the RC_CHANNEL_OUT_COUNT to 4 servos, the library will pulse them more frequently than
// it can ten -
// 10 servos at 1500us = 15ms = 66Hz
// 4 Servos at 1500us = 6ms = 166Hz
// if you wanted to go even higher, run two servos on each timer
// 2 Servos at 1500us = 3ms = 333Hz
//
// You might not want a high refresh rate though, so the setFrameSpace function is provided for you to
// add a pause before the library begins its next run through the servos
// for 50 hz, the pause should be to (20,000 - (RC_CHANNEL_OUT_COUNT * 2000))

// Change to set the number of servos/ESCs
#define RC_CHANNEL_OUT_COUNT 5

// Minimum and Maximum servo pulse widths, you could change these,
// Check the servo library and use that range if you prefer
#define RCARDUINO_SERIAL_SERVO_MIN 1000
#define RCARDUINO_SERIAL_SERVO_MAX 2000
#define RCARDUINO_SERIAL_SERVO_DEFAULT 1500

#define RC_CHANNELS_NOPORT 0
#define RC_CHANNELS_NOPIN 255

#define RCARDUINO_MAX_SERVOS (RC_CHANNEL_OUT_COUNT)

class PortPin {
  public:
    volatile unsigned char *port;
    uint8_t pinMask;
    uint16_t pulseWidth;
};

// Records the current output channel values in timer ticks
// Manually set by calling writeMicroseconds_pwmMotor, the function adjusts from user supplied micro seconds to timer ticks
volatile static PortPin pwmMotorOut[RC_CHANNEL_OUT_COUNT];

// Current output channel, used by the timer ISR to track which channel is being generated
volatile static uint8_t currentOutputChannelA;

void setup_pwmMotor() {
  currentOutputChannelA = 0;
  while (currentOutputChannelA < RC_CHANNEL_OUT_COUNT) {
    pwmMotorOut[currentOutputChannelA].pulseWidth = microsecondsToTicks_pwmMotor(RCARDUINO_SERIAL_SERVO_DEFAULT);

    currentOutputChannelA++;
  }
}

void begin_pwmMotor() {
  TCNT1 = 0;               // clear the timer count

  // Initilialise Timer1
  TCCR1A = 0;              // normal counting mode
  TCCR1B = 2;              // set prescaler of 8 = 1 tick = 0.5us (see ATmega328 datasheet pgs. 134-135)

  // ENABLE TIMER1 OCR1A INTERRUPT to enabled the first bank (A) of ten servos
  TIFR1 |= _BV(OCF1A);     // clear any pending interrupts;
  TIMSK1 |=  _BV(OCIE1A) ; // enable the output compare interrupt

  OCR1A = TCNT1 + 4000;    // Start in two milli seconds
}

void attach_pwmMotor(uint8_t channel, uint8_t pin) {
  if (channel >= RCARDUINO_MAX_SERVOS)
    return;

  // disable interrupts while we update the multi byte value output value
  uint8_t sreg = SREG;
  cli();

  pwmMotorOut[channel].pulseWidth = microsecondsToTicks_pwmMotor(RCARDUINO_SERIAL_SERVO_DEFAULT);
  pwmMotorOut[channel].port = getPortFromPin(pin);
  pwmMotorOut[channel].pinMask = getPortPinMaskFromPin(pin);

  // enable interrupts
  SREG = sreg;

  // TODO: Ty to replace with DDRD = DDRD | pwmMotorOut[channel].pinMask;
  pinMode(pin, OUTPUT);
}

// Timer1 Output Compare A interrupt service routine
ISR(TIMER1_COMPA_vect) {
  // if the channel number is >= 10, we need to reset the counter and start again from zero.
  // to do this we pulse the reset pin of the counter this sets output 0 of the counter high, effectivley
  // starting the first pulse of our first channel
  if (currentOutputChannelA >= RC_CHANNEL_OUT_COUNT) {
    // reset our current servo/output channel to 0
    currentOutputChannelA = 0;

    setChannelPinLowA(RC_CHANNEL_OUT_COUNT - 1);
  } else {
    setChannelPinLowA(currentOutputChannelA - 1);
  }

  setCurrentChannelPinHighA();

  // set the duration of the output pulse
  setOutputTimerForPulseDurationA();

  // done with this channel so move on.
  currentOutputChannelA++;
}

void setChannelPinLowA(uint8_t channel) {
  volatile PortPin *portPin = pwmMotorOut + channel;

  if (portPin->pinMask)
    *portPin->port ^= portPin->pinMask;
}

void setCurrentChannelPinHighA() {
  volatile PortPin *portPin = pwmMotorOut + currentOutputChannelA;

  if (portPin->pinMask)
    *portPin->port |= portPin->pinMask;
}

// After we set an output pin high, we need to set the timer to comeback for the end of the pulse
void setOutputTimerForPulseDurationA() {
  OCR1A = TCNT1 + pwmMotorOut[currentOutputChannelA].pulseWidth;
}

// Updates a channel to a new value, the class will continue to pulse the channel
// with this value for the lifetime of the sketch or until writeChannel is called
// again to update the value
void writeMicroseconds_pwmMotor(uint8_t nChannel, uint16_t unMicroseconds) {
  // dont allow a write to a non existent channel
  if (nChannel > RCARDUINO_MAX_SERVOS)
    return;

  // constraint the value just in case
  unMicroseconds = constrain(unMicroseconds, RCARDUINO_SERIAL_SERVO_MIN, RCARDUINO_SERIAL_SERVO_MAX);

  unMicroseconds = microsecondsToTicks_pwmMotor(unMicroseconds);

  // disable interrupts while we update the multi byte value output value
  uint8_t sreg = SREG;
  cli();

  pwmMotorOut[nChannel].pulseWidth = unMicroseconds;

  // enable interrupts
  SREG = sreg;
}

// Specify end frame space. This allows us to run different refresh frequencies on channel A, for example servos at a 70Hz rate on A and ESCs at 250Hz on B
void setFrameSpaceA_pwmMotor(uint8_t channel, uint16_t unMicroseconds) {
  // disable interrupts while we update the multi byte value output value
  uint8_t sreg = SREG;
  cli();

  pwmMotorOut[channel].pulseWidth = microsecondsToTicks_pwmMotor(unMicroseconds);
  pwmMotorOut[channel].port = 0;
  pwmMotorOut[channel].pinMask = 0;

  // enable interrupts
  SREG = sreg;
}
uint16_t ticksToMicroseconds_pwmMotor(uint16_t unTicks) {
  return unTicks / 2;
}

uint16_t microsecondsToTicks_pwmMotor(uint16_t unMicroseconds) {
  return unMicroseconds * 2;
}

// Allow to check if the current channel is framespace to execute long routines
uint8_t getcurrentOutputChannelA() {
  return currentOutputChannelA - 1;
}

volatile uint8_t* getPortFromPin(uint8_t pin) {
  volatile uint8_t* port = RC_CHANNELS_NOPORT;

  if (pin <= 7) {
    port = &PORTD;
  } else if (pin <= 13) {
    port = &PORTB;
  } else if (pin <= A5) { // analog input pin 5
    port = &PORTC;
  }

  return port;
}

uint8_t getPortPinMaskFromPin(uint8_t pin) {
  uint8_t portPinMask = RC_CHANNELS_NOPIN;

  if (pin <= A5) {
    if (pin <= 7) {
      portPinMask = (1 << pin);
    } else if (pin <= 13) {
      pin -= 8;
      portPinMask = (1 << pin);
    } else if (pin <= A5) {
      pin -= A0;
      portPinMask = (1 << pin);
    }
  }

  return portPinMask;
}
