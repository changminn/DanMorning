#include <Servo.h>
#include "DHT.h" 
#include <SoftwareSerial.h>

Servo myservo;
Servo Tapservo;
int force_sensor1 = A0;
int force_sensor2 = A1;
int water = A3;

//블루투스 모듈
#define blueTX 5  // BT_TXD를 5로 설정합니다.
#define blueRX 13  // BT_RXD를 13으로 설정합니다.

SoftwareSerial mySerial(blueRX, blueTX);
//5, 13번 data pin사용

int ledPin_bath= 2;

int dhtPin = 3;

int servoPin = 4;

int servoTap = 6;

int ledPin_room = 7;

int Relaypin = 9;

int count = 0;
int alarm = -1;
int ButtonSwitch =0;

//초음파 센서
#define echoPin 12                    // echoPin을 12로 설정합니다.
#define trigPin 11                   // trigPin을 13으로 설정합니다.
//12~13 data pin사용

long duration, distance;

DHT dht(dhtPin,DHT11);

int old_force1=0;
int val = 0;

int angle = 0;

void setup() {
  // put your setup code here, to run once:
   
  Serial.begin(9600);

  mySerial.begin(9600);

  myservo.attach(servoPin);
  Tapservo.attach(servoTap);

  pinMode(Relaypin, OUTPUT);
  
  pinMode(ledPin_bath, OUTPUT);
  pinMode(ledPin_room, OUTPUT);
  
  pinMode(force_sensor1, INPUT);
  pinMode(force_sensor2, INPUT);
  
  pinMode(trigPin, OUTPUT);   // trigPin 핀을 출력핀으로 설정합니다.
  pinMode(echoPin, INPUT);    // echoPin 핀을 입력핀으로 설정합니다.
}

void loop() {

  // put your main code here, to run repeatedly:
  int level = analogRead(water);  // 수분센서의 신호를 측정합니다.


  int force_val1 = analogRead(force_sensor1); 
  int force_val2 = analogRead(force_sensor2);

  
 int angle1 = 0;

//블루투스 모듈 (begin)
  if (mySerial.available()) {
    Serial.write(mySerial.read());  //블루투스측 내용을 시리얼모니터에 출력
    alarm = 1;
    ButtonSwitch++;
    if(ButtonSwitch>1){
      ButtonSwitch=0;
    }
    else if(ButtonSwitch==0){
      alarm=-1;
    }
    if(alarm==1){
      Serial.println("All Sensors ON");
      digitalWrite(ledPin_room, HIGH);
      digitalWrite(ledPin_bath, HIGH);
      digitalWrite(Relaypin, HIGH);
      for(angle1 = 180; angle1 > 0; angle1--) 
      { 
        Tapservo.write(angle1); 
        delay(5); 
      }
    }
    else{
    Serial.println("All Sensors OFF");
    }
  }
  if(Serial.available()){
    mySerial.write(Serial.read());
  }
//블루투스 모듈 (end)




//릴레이1채널 + 진동
/*
  if(val == 1){
    digitalWrite(Relaypin, HIGH);
  }
  else{
    digitalWrite(Relaypin, LOW)
  };*/
//
 
//물 센서
  Serial.println(String("water level :")+level);
//  delay(100);
  if(level >500 && count == 0){
    for(angle1 = 0; angle1 < 180; angle1++) 
    { 
      Tapservo.write(angle1); 
      delay(5); 
      count++;
    }
    angle1 = 0; 
  }

  if(level < 500){
    count = 0;
  }

//압력
  Serial.println("first : "+String(force_val1));
//  delay(100);
  Serial.println("second : "+String(force_val2));
//  delay(100);


//초음파 센서 (begin)
  digitalWrite(trigPin, LOW);                 // trigPin에 LOW를 출력하고
  delayMicroseconds(2);                    // 2 마이크로초가 지나면
  digitalWrite(trigPin, HIGH);                // trigPin에 HIGH를 출력합니다.
  delayMicroseconds(10);                  // trigPin을 10마이크로초 동안 기다렸다가
  digitalWrite(trigPin, LOW);                // trigPin에 LOW를 출력합니다.
  duration = pulseIn(echoPin, HIGH);   // echoPin핀에서 펄스값을 받아옵니다.

  distance = duration * 17 / 1000;          //  duration을 연산하여 센싱한 거리값을 distance에 저장합니다.

  if (distance >= 200 || distance <= 0)       // 거리가 200cm가 넘거나 0보다 작으면
  {
    Serial.println("거리를 측정할 수 없음");   // 에러를 출력합니다.
  }
  else if(distance >= 0 && distance <=7){
    digitalWrite(ledPin_bath, LOW);
    digitalWrite(ledPin_room, LOW);
    Serial.print(distance);
    Serial.println(" cm");
    Serial.println("Passed");    
  }
    else                                               // 거리가 200cm가 넘지 않거나 0보다 작지 않으면
  {
    Serial.print(distance);                        // distance를 시리얼 모니터에 출력합니다.
    Serial.println(" cm");                         // cm를 출력하고 줄을 넘깁니다.
                                                     // distance가 10이면 10 cm로 출력됩니다.
  }
//  delay(100);                                         // 0.5초동안 기다립니다.
//초음파센서 (end)



//압력 값 조건 되면 
  if(force_val1 < 50 && force_val2 != 0 && old_force1 > force_val1+100){
    digitalWrite(Relaypin, LOW);
  }

  old_force1 = force_val1;

 //온습도 측정 데이터 보여줌
  int humidity = dht.readHumidity();
  int temperature = dht.readTemperature();
  
  Serial.print("Humidity: ");  // 문자열 Humidity: 를 출력한다.
  Serial.print(humidity);  // 변수 h(습도)를 출력한다.
  Serial.print("%\t");  // %를 출력한다
  Serial.print("Temperature: ");  // 이하생략
  Serial.print(temperature);
  Serial.println(" C");


  
//습도 높으면 환기 모터 돌아감
  if(humidity  > 75){
    for(angle = 0; angle < 180; angle++) 
    { 
      myservo.write(angle); 
      delay(5); 
    } 
    // now scan back from 180 to 0 degrees
    for(angle = 180; angle > 0; angle--) 
    { 
      myservo.write(angle); 
      delay(5); 
    }
  }

  delay(450);
   
}
