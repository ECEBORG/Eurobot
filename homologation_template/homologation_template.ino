#define SHARP A0

#define M1 8
#define D1 9

#define M2 10
#define D2 11

#define START 2

#define POLLING_RATE 100

#define MIN_DIST 400

#define DEF_SPEED 50

int time=0;

void fwd(int mil){
  move(DEF_SPEED,HIGH,DEF_SPEED,HIGH,mil);
}

void bckw(int mil){
  move(DEF_SPEED,LOW,DEF_SPEED,LOW,mil);
}

void turnR(int mil){
  move(DEF_SPEED+17,HIGH,DEF_SPEED+17,LOW,mil);
}

void turnL(int mil){
  move(DEF_SPEED,LOW,DEF_SPEED,HIGH,mil);
}

void move (int M1_SPEED, int M1_DIR, int M2_SPEED, int M2_DIR, int time_delay){
  for (int i=0; i<time_delay/POLLING_RATE; i++){
    if(analogRead(SHARP) > MIN_DIST){
    digitalWrite(M1, 0);
    digitalWrite(M2, 0);

    while(analogRead(SHARP) > MIN_DIST)
      delay(POLLING_RATE);
    }

    analogWrite(M1, M1_SPEED);
    analogWrite(M2, M2_SPEED);
    digitalWrite(D1, M1_DIR);
    digitalWrite(D2, M2_DIR);

    delay(POLLING_RATE);
  }
  
  analogWrite(M1, 0);
  analogWrite(M2, 0);
}

void setup()
{
  pinMode(SHARP, INPUT);
  pinMode(START, INPUT_PULLUP);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  while (!digitalRead(START));
}

void loop()
{
  fwd(4000);
  turnR(4000);
  fwd(6500);
  while(1);
}
