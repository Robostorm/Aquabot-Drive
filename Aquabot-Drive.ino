#include <Servo.h>

#define ALIE 0
#define ELEV 1
#define BUTTON 13

#define LEFTMOTOR 5
#define RIGHTMOTOR 6

#define ALIEZERO 1486
#define ALIEMAXIN 1912
#define ALIEMAX 2000
#define ALIEMININ 1084
#define ALIEMIN 1000
#define ALIEDEAD 50

#define ELEVZERO 1486
#define ELEVMAXIN 1912
#define ELEVMAX 2000
#define ELEVMININ 1092
#define ELEVMIN 1000
#define ELEVDEAD 50

#define STARTDELAY 2000

#define LED1 4
#define LED2 7

boolean enabled = false;
unsigned long stillTime = 0;
unsigned long time = 0;
unsigned long printTime = 0;
int alieIn = ALIEZERO;
int elevIn = ELEVZERO;
int alie = ALIEZERO;
int elev = ELEVZERO;
int oldAlie = 0;
int oldElev = 0;
int leftMotor = 0;
int rightMotor = 0;

boolean led1 = false;
boolean led2 = false;

double trunReduct = 0.5;

Servo leftServo;
Servo rightServo;

void setup() {
  // Attach Servos
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

  // Attach interrupts
  attachInterrupt(ALIE, alieRising, RISING);
  attachInterrupt(ELEV, elevRising, RISING);

  // Calibrate sticks
  //alieZero = alie;
  //elevZero = elev;

  // Write to center pos, delay to make sure they are calibrated on motor controller
  leftServo.writeMicroseconds(ALIEZERO);
  rightServo.writeMicroseconds(ELEVZERO);
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
	Serial.print("Alei In: ");
	Serial.print(alieIn);
	Serial.print(" Alei Out: ");
	Serial.print(alie);
	Serial.print(" Elev In: ");
	Serial.print(elevIn);
	Serial.print(" Elev Out: ");
    Serial.print(elev);
    Serial.print(" Btn: ");
    Serial.print(pulseIn(BUTTON, HIGH));
	Serial.println("\r");
	
    printTime = time;
  }

  //motors
  if(enabled) {
    if (alieIn < ALIEZERO - ALIEDEAD || alieIn > ALIEZERO + ALIEDEAD) {
	  if(alieIn > ALIEDEAD ){
        //alie -= ALIEDEAD ;
		alie = map(alieIn, ALIEZERO + ALIEDEAD, ALIEMAXIN, ALIEZERO, ALIEMAX);
		alie = (alie <= 2000) ? alie : 2000;
      }else{
        //alie += ALIEDEAD ;
		alie = map(alieIn, ALIEZERO - ALIEDEAD, ALIEMININ, ALIEZERO, ALIEMIN);
		alie = (alie >= 1000) ? alie : 1000;
      }
      int turn = (double)(alie - ALIEZERO) * trunReduct;
      leftServo.writeMicroseconds(turn + ALIEZERO);
      stillTime = time;
      led2 = true;
    } else {
      leftServo.writeMicroseconds(ALIEZERO);
      led2 = false;
    }
    if (elevIn < ELEVZERO - ELEVDEAD || elevIn > ELEVZERO + ELEVDEAD) {
	  if(elevIn > ELEVZERO){
        //elev -= ELEVDEAD;
		elev = map(elevIn, ELEVZERO + ELEVDEAD, ELEVMAXIN, ELEVZERO, ELEVMAX);
		elev = (elev <= 2000) ? elev : 2000;
      }else{
        //elev += ELEVDEAD;
		elev = map(elevIn, ELEVZERO + ELEVDEAD, ELEVMININ, ELEVZERO, ELEVMIN);
		elev = (elev >= 1000) ? elev : 1000;
      }
      rightServo.writeMicroseconds(elev);
	    stillTime = time;
      led2 = true;
    }  else {
      rightServo.writeMicroseconds(ELEVZERO);
      led2 = false;
    }
  }

  led1 = enabled;

  digitalWrite(LED1, led1);
  digitalWrite(LED2, led2);

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
  alieIn = micros() - oldAlie;
}

void elevRising() {
  attachInterrupt(ELEV, elevFalling, FALLING);
  oldElev = micros();
}

void elevFalling() {
  attachInterrupt(ELEV, elevRising, RISING);
  elevIn = micros() - oldElev;
}
