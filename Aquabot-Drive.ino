#include <Servo.h>

/*
 *
 *  ██████  ██████  ███    ██ ███████ ████████  █████  ███    ██ ████████ ███████
 * ██      ██    ██ ████   ██ ██         ██    ██   ██ ████   ██    ██    ██
 * ██      ██    ██ ██ ██  ██ ███████    ██    ███████ ██ ██  ██    ██    ███████
 * ██      ██    ██ ██  ██ ██      ██    ██    ██   ██ ██  ██ ██    ██         ██
 *  ██████  ██████  ██   ████ ███████    ██    ██   ██ ██   ████    ██    ███████
 *
 */

// Uncomment to enable debug messages in the serial console
// Prints input and motor values
#define SERIAL_DEBUG

#define SERIAL_BAUD 9600

#define STEER_IN_PIN 0 // Steering reciver input (Alieron)
#define DRIVE_IN_PIN 1 // Drive reciver input (Elevator)
#define ENABLE_PIN 13 // Enable button reciver input (Bind button)
#define LED_ENABLE_PIN 4
#define LED_DRIVE_PIN 7

#define DRIVE_OUT_PIN 6
#define STEER_OUT_PIN 5

#define STEER_IN_MAX 1912
#define STEER_IN_MIN 1100
#define STEER_IN_MID_MAX 1518
#define STEER_IN_MID_MIN 1490
#define STEER_IN_MID (STEER_IN_MID_MAX + STEER_IN_MID_MIN)/2

#define DRIVE_IN_MAX 1900
#define DRIVE_IN_MIN 1080
#define DRIVE_IN_MID_MAX 1494
#define DRIVE_IN_MID_MIN 1475
#define DRIVE_IN_MID (DRIVE_IN_MID_MAX + DRIVE_IN_MID_MIN)/2

#define STEER_OUT_MAX 2000
#define STEER_OUT_MIN 1000
#define STEER_OUT_MID_MAX 1575
#define STEER_OUT_MID_MIN 1442
#define STEER_OUT_MID (STEER_OUT_MID_MAX + STEER_OUT_MID_MIN)/2

#define DRIVE_OUT_MAX 2000
#define DRIVE_OUT_MIN 1000
#define DRIVE_OUT_MID_MAX 1600
#define DRIVE_OUT_MID_MIN 1455
#define DRIVE_OUT_MID (DRIVE_OUT_MID_MAX + DRIVE_OUT_MID_MIN)/2

#define ENABLE_MID 1500

#define STEER_RATE 0.7
#define DRIVE_RATE 1

#define ENABLE_TIMEOUT 5000

#define DELAY_STAGGER 10

#define PRINT_DELAY 50
#define MOTOR_DELAY 50
#define ENABLE_DELAY 50
#define LED_DELAY 50

/*
 *
 * ██    ██  █████  ██████  ██  █████  ██████  ██      ███████ ███████
 * ██    ██ ██   ██ ██   ██ ██ ██   ██ ██   ██ ██      ██      ██
 * ██    ██ ███████ ██████  ██ ███████ ██████  ██      █████   ███████
 *  ██  ██  ██   ██ ██   ██ ██ ██   ██ ██   ██ ██      ██           ██
 *   ████   ██   ██ ██   ██ ██ ██   ██ ██████  ███████ ███████ ███████
 *
 */

int steer_in = STEER_IN_MID;
int drive_in = DRIVE_IN_MID;

int steer_out = STEER_OUT_MID;
int drive_out = DRIVE_OUT_MID;

bool enable = false;

bool enabled = false;
bool driving = false;

bool led_enable = false;
bool led_drive = false;

Servo steer_servo;
Servo drive_servo;

int steer_rise_micros = 0;
int drive_rise_micros = 0;
int enable_rise_micros = 0;

long stillTime = 0;

long now = 0;
long printTime = 0;
long motorTime = 0;
long enableTime = 0;
long ledTime = 0;

/*
 *
 * ███████ ███████ ████████ ██    ██ ██████
 * ██      ██         ██    ██    ██ ██   ██
 * ███████ █████      ██    ██    ██ ██████
 *     ██  ██         ██    ██    ██ ██
 * ███████ ███████    ██     ██████  ██
 *
 */

void setup(){

  // Enable serial if serial debugging is enabled
  #ifdef SERIAL_DEBUG
  Serial.begin(SERIAL_BAUD);
  #endif // SERIAL_DEBUG

  // Setup input pins
  pinMode(STEER_IN_PIN, INPUT);
  pinMode(DRIVE_IN_PIN, INPUT);
  pinMode(ENABLE_PIN, INPUT);

  // Setup output LED pins
  pinMode(LED_ENABLE_PIN, OUTPUT);
  pinMode(LED_DRIVE_PIN, OUTPUT);

  steer_servo.attach(STEER_OUT_PIN);
  drive_servo.attach(DRIVE_OUT_PIN);

  // Attatch interupts
  attachInterrupt(STEER_IN_PIN, steerRising, RISING);
  attachInterrupt(DRIVE_IN_PIN, driveRising, RISING);



  // Write to center pos, delay to make sure they are calibrated on motor controller
  // ONLY NEEDED IF MOTOR CONTROLLER IS IN RC MODE

  //steer_servo.writeMicroseconds(steer_out;
  //drive_servo.writeMicroseconds(drive_out;
  //delay(START_DELAY);

  // Blink LEDs
  digitalWrite(LED_ENABLE_PIN, HIGH);
  digitalWrite(LED_DRIVE_PIN, HIGH);
  delay(500);
  digitalWrite(LED_ENABLE_PIN, LOW);
  digitalWrite(LED_DRIVE_PIN, LOW);
  delay(500);
  digitalWrite(LED_ENABLE_PIN, HIGH);
  digitalWrite(LED_DRIVE_PIN, HIGH);
  delay(500);
  digitalWrite(LED_ENABLE_PIN, LOW);
  digitalWrite(LED_DRIVE_PIN, LOW);

  now = millis();
  printTime = now;
  motorTime = now + DELAY_STAGGER;
  enableTime = now + DELAY_STAGGER*2;
  ledTime = now + DELAY_STAGGER*3;
}

/*
 *
 * ██       ██████   ██████  ██████
 * ██      ██    ██ ██    ██ ██   ██
 * ██      ██    ██ ██    ██ ██████
 * ██      ██    ██ ██    ██ ██
 * ███████  ██████   ██████  ██
 *
 */

void loop(){

  now = millis();

  /*
   *
   * ██████  ██████  ██ ███    ██ ████████
   * ██   ██ ██   ██ ██ ████   ██    ██
   * ██████  ██████  ██ ██ ██  ██    ██
   * ██      ██   ██ ██ ██  ██ ██    ██
   * ██      ██   ██ ██ ██   ████    ██
   *
   */

  #ifdef SERIAL_DEBUG
  if(now - printTime > PRINT_DELAY){
    Serial.print(" SI:");
    Serial.print(steer_in);
    Serial.print(" DI:");
    Serial.print(drive_in);
    Serial.print(" EN:");
    Serial.print(enable);
    Serial.print(" CE:");
    Serial.print(enabled);
    Serial.print(" CD:");
    Serial.print(driving);
    Serial.print(" SO:");
    Serial.print(steer_out);
    Serial.print(" DO:");
    Serial.print(drive_out);
    Serial.println();
    printTime = now;
  }
  #endif // SERIAL_DEBUG

  /*
   *
   * ███████ ███    ██  █████  ██████  ██      ███████
   * ██      ████   ██ ██   ██ ██   ██ ██      ██
   * █████   ██ ██  ██ ███████ ██████  ██      █████
   * ██      ██  ██ ██ ██   ██ ██   ██ ██      ██
   * ███████ ██   ████ ██   ██ ██████  ███████ ███████
   */


  if(now - enableTime > ENABLE_DELAY){
    enable = pulseIn(ENABLE_PIN, HIGH, 0.1) > ENABLE_MID;
    if(enable){
      stillTime = now;
      enabled = true;
    }
    enableTime = now;
  }

  if(now - stillTime > ENABLE_TIMEOUT){
    enabled = false;
  }

  /*
   *
   * ██      ███████ ██████  ███████
   * ██      ██      ██   ██ ██
   * ██      █████   ██   ██ ███████
   * ██      ██      ██   ██      ██
   * ███████ ███████ ██████  ███████
   *
  */

  if(now - ledTime > LED_DELAY){
    digitalWrite(LED_DRIVE_PIN, driving);
    digitalWrite(LED_ENABLE_PIN, enabled);
  }

  /*
   *
   * ███    ███  ██████  ████████  ██████  ██████  ███████
   * ████  ████ ██    ██    ██    ██    ██ ██   ██ ██
   * ██ ████ ██ ██    ██    ██    ██    ██ ██████  ███████
   * ██  ██  ██ ██    ██    ██    ██    ██ ██   ██      ██
   * ██      ██  ██████     ██     ██████  ██   ██ ███████
   *
   */

  if(now - motorTime > MOTOR_DELAY){

    /*
     *
     * ███████ ████████ ███████ ███████ ██████  ██ ███    ██  ██████
     * ██         ██    ██      ██      ██   ██ ██ ████   ██ ██
     * ███████    ██    █████   █████   ██████  ██ ██ ██  ██ ██   ███
     *      ██    ██    ██      ██      ██   ██ ██ ██  ██ ██ ██    ██
     * ███████    ██    ███████ ███████ ██   ██ ██ ██   ████  ██████
     *
     */

    driving = false;

    if(steer_in > STEER_IN_MID_MAX && enabled){
      stillTime = now;
      driving = true;

      // Ensure that the input is no greater than the max possible input. This assures that the output will be no greater than the max output.
      if(steer_in > STEER_IN_MAX){
        steer_in  = STEER_IN_MAX;
      }

      /*
      * In order to apply the steer rate correctly, we need a positive number representing the input, such that 0 is stopped.
      * To get that, we subtract STEER_IN_MID_MAX from the input.
      * This is steer_in - STEER_IN_MID_MAX.
      * That number then needs to be mapped between the max input above the in mid, and the max  output above the out mid.
      * These are STEER_IN_MAX - STEER_IN_MID_MAX and STEER_OUT_MAX - STEER_OUT_MID_MAX, resepectivly.
      * The mapping itself is done by the map(...) function. https://www.arduino.cc/en/Reference/Map
      * The mapped value then needs to be added back to STEER_OUT_MID_MAX to be a valid signal to the motor controller
      */

      steer_out = STEER_OUT_MID_MAX + map(steer_in - STEER_IN_MID_MAX, 0, STEER_IN_MAX - STEER_IN_MID_MAX, 0, STEER_OUT_MAX - STEER_OUT_MID_MAX)*STEER_RATE;

    }else if(steer_in < STEER_IN_MID_MIN && enabled){
      stillTime = now;
      driving = true;

      // Ensure that the input is no less than the minimum possible input. This assures that the output will be no less than the minimum output.
      if(steer_in < STEER_IN_MIN){
        steer_in  = STEER_IN_MIN;
      }

      /*
      * In order to apply the steer rate correctly, we need a positive number representing the input, such that 0 is stopped.
      * To get that, we subtract the input from STEER_IN_MID_MIN.
      * This is STEER_IN_MID_MIN - steer_in.
      * That number then needs to be mapped between the min input mid and the min output mid.
      * These are STEER_IN_MID_MIN - STEER_IN_MIN and STEER_OUT_MID_MIN - STEER_OUT_MIN, resepectivly.
      * The mapping itself is done by the map(...) function. https://www.arduino.cc/en/Reference/Map
      * The mapped value then needs to be substracted back from STEER_OUT_MID_MIN to be a valid signal to the motor controller
      */

      steer_out = STEER_OUT_MID_MIN - map(STEER_IN_MID_MIN - steer_in, 0, STEER_IN_MID_MIN - STEER_IN_MIN, 0, STEER_OUT_MID_MIN - STEER_OUT_MIN)*STEER_RATE;

    }else{
      steer_out = STEER_OUT_MID;
    }

    /*
     *
     * ██████  ██████  ██ ██    ██ ███████
     * ██   ██ ██   ██ ██ ██    ██ ██
     * ██   ██ ██████  ██ ██    ██ █████
     * ██   ██ ██   ██ ██  ██  ██  ██
     * ██████  ██   ██ ██   ████   ███████
     *
     */

    if(drive_in > DRIVE_IN_MID_MAX && enabled){
      stillTime = now;
      driving = true;

      // Ensure that the input is no greater than the max possible input. This assures that the output will be no greater than the max output.
      if(drive_in > DRIVE_IN_MAX){
        drive_in  = DRIVE_IN_MAX;
      }

      /*
      * In order to apply the drive rate correctly, we need a positive number representing the input, such that 0 is stopped.
      * To get that, we subtract DRIVE_IN_MID_MAX from the input.
      * This is drive_in - DRIVE_IN_MID_MAX.
      * That number then needs to be mapped between the max input above the in mid, and the max  output above the out mid.
      * These are DRIVE_IN_MAX - DRIVE_IN_MID_MAX and DRIVE_OUT_MAX - DRIVE_OUT_MID_MAX, resepectivly.
      * The mapping itself is done by the map(...) function. https://www.arduino.cc/en/Reference/Map
      * The mapped value then needs to be added back to DRIVE_OUT_MID_MAX to be a valid signal to the motor controller
      */

      drive_out = DRIVE_OUT_MID_MAX + map(drive_in - DRIVE_IN_MID_MAX, 0, DRIVE_IN_MAX - DRIVE_IN_MID_MAX, 0, DRIVE_OUT_MAX - DRIVE_OUT_MID_MAX)*DRIVE_RATE;

    }else if(drive_in < DRIVE_IN_MID_MIN && enabled){
      stillTime = now;
      driving = true;

      // Ensure that the input is no less than the minimum possible input. This assures that the output will be no less than the minimum output.
      if(drive_in < DRIVE_IN_MIN){
        drive_in  = DRIVE_IN_MIN;
      }

      /*
      * In order to apply the drive rate correctly, we need a positive number representing the input, such that 0 is stopped.
      * Tonabled = enable > ENABLE_MID;-2---  * The mapping itself is done by the map(...) function. https://www.arduino.cc/en/Reference/Map
      * The mapped value then needs to be substracted back from DRIVE_OUT_MID_MIN to be a valid signal to the motor controller
      */

      drive_out = DRIVE_OUT_MID_MIN - map(DRIVE_IN_MID_MIN - drive_in, 0, DRIVE_IN_MID_MIN - DRIVE_IN_MIN, 0, DRIVE_OUT_MID_MIN - DRIVE_OUT_MIN)*DRIVE_RATE;

    }else{
      drive_out = DRIVE_OUT_MID;
    }

    steer_servo.writeMicroseconds(steer_out);
    drive_servo.writeMicroseconds(drive_out);

    motorTime = now;
  }
}

/*
 *
 * ██ ███    ██ ████████ ███████ ██████  ██    ██ ██████  ████████ ███████
 * ██ ████   ██    ██    ██      ██   ██ ██    ██ ██   ██    ██    ██
 * ██ ██ ██  ██    ██    █████   ██████  ██    ██ ██████     ██    ███████
 * ██ ██  ██ ██    ██    ██      ██   ██ ██    ██ ██         ██         ██
 * ██ ██   ████    ██    ███████ ██   ██  ██████  ██         ██    ███████
 *
 */

void steerRising() {
  attachInterrupt(STEER_IN_PIN, steerFalling, FALLING);
  steer_rise_micros = micros();
}

void steerFalling() {
  attachInterrupt(STEER_IN_PIN, steerRising, RISING);
  steer_in = micros() - steer_rise_micros;
}

void driveRising() {
  attachInterrupt(DRIVE_IN_PIN, driveFalling, FALLING);
  drive_rise_micros = micros();
}

void driveFalling() {
  attachInterrupt(DRIVE_IN_PIN, driveRising, RISING);
  drive_in = micros() - drive_rise_micros;
}
