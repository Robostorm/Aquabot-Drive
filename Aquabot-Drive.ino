#include <Servo.h>

#define ALIE 0
#define ELEV 1
#define BUTTON 13

#define LEFTMOTOR 5
#define RIGHTMOTOR 6

#define ALIEZERO 1486
#define ALIEMAX 1000
#define ALIEMIN 2000
#define ALIEDEAD 50

#define ELEVZERO 1486
#define ELEVMAX 1000
#define ELEVZMIN 2000
#define ELEVDEAD 50

#define STARTDELAY 2000

#define LED1 4
#define LED2 7

boolean enabled = false;
int stillTime = 0;
int time = 0;
int printTime = 0;
int alieZero = ALIEZERO;
int elevZero = ELEVZERO;
int alie = 0;
int elev = 0;
int oldAlie = 0;
int oldElev = 0;
int leftMotor = 0;
int rightMotor = 0;

double trunReduct = 0.5;

Servo leftServo;
Servo rightServo;

void setup() {
  // Attatch Servos
  leftServo.attach(LEFTMOTOR);
  rightServo.attach(RIGHTMOTOR);

  // Set pin modes
  pinMode(ALIE, INPUT);
  pinMode(ELEV, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUTTON, INPUT);

  // Start Serial
  Serial.begin(9600);

  // Attatch interupts
  attachInterrupt(ALIE, alieRising, RISING);
  attachInterrupt(ELEV, elevRising, RISING);

  // Calibrate sticks
  //alieZero = alie;
  //elevZero = elev;

  // Write to center pos, delay to make sure they are calibrated on motor controller
  leftServo.writeMicroseconds(alieZero);
  rightServo.writeMicroseconds(elevZero);
  delay(STARTDELAY);

  // Blink LEDs
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  delay(500);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  // Set the time
  time = millis();
  printTime = time;
}

void loop() {
  time = millis();
  if (time - printTime >= 100) {
    Serial.print(alie);
    Serial.print(" : ");
    Serial.print(elev);
    Serial.print(" : ");
    Serial.println(pulseIn(BUTTON, HIGH));
    printTime = time;
  }

  //motors
  if(enabled) {
    if (alie < alieZero - ALIEDEAD || alie > alieZero + ALIEDEAD) {
      int turn = (double)(alie - alieZero)*trunReduct;
      leftServo.writeMicroseconds(turn+alieZero);
      stillTime = time;
    } else {
      leftServo.writeMicroseconds(alieZero);
    }
    if (elev < elevZero - ELEVDEAD || elev > elevZero + ELEVDEAD) {
      rightServo.writeMicroseconds(elev);
	    stillTime = time;
    }  else {
      rightServo.writeMicroseconds(elevZero);
    }
  }

  //safety
  if(time - stillTime >= 5000) {
    enabled = false;
    leftServo.write(90);
    rightServo.write(90);
  }

  if(pulseIn(BUTTON, HIGH) > 1500) {
    enabled = true;
	stillTime = time;
  }
}

void alieRising() {
  attachInterrupt(ALIE, alieFalling, FALLING);
  oldAlie = micros();
}

void alieFalling() {
  attachInterrupt(ALIE, alieRising, RISING);
  alie = micros() - oldAlie;
}

void elevRising() {
  attachInterrupt(ELEV, elevFalling, FALLING);
  oldElev = micros();
}

void elevFalling() {
  attachInterrupt(ELEV, elevRising, RISING);
  elev = micros() - oldElev;
}
