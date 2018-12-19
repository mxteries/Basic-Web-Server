#include <LiquidCrystal.h>
#define trigPin 4
#define echoPin 5

//for noise
#define buzzer 13
int TOOCLOSE = 4;
int TOOFAR = 400;
long duration;
int distance;
const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  // put your setup code here, to run once:
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(57600);
  lcd.begin(16, 2);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  //in microseconds
  duration = pulseIn(echoPin, HIGH);

  distance = duration*.0343/2;
  //writes to lcd and serial
  lcd.clear();
  lcd.setCursor(0,0);
  Serial.print(distance);
  Serial.println(" cm");
  lcd.print("distance: ");
  lcd.print(distance);
  //if close
  if(distance <= TOOCLOSE)
  {
  Serial.println("SECURITY BREACH");
  lcd.setCursor(0,1);
  lcd.print("SECURITY BREACH");
  digitalWrite(buzzer,HIGH);
  }
  //if far
  else if(distance >= TOOFAR)
  {
  Serial.println("FAR");
  lcd.setCursor(0,1);
  lcd.print("FAR");
  }
  else
  {
    //turn alarm off
    digitalWrite(buzzer,LOW);
  }
  delay(100);
  }  

