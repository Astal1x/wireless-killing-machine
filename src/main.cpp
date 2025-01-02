#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

// Настройка модуля nRF24L01
RF24 radio(9, 10);  // CE, CSN
const byte address[6] = "R1337";  // Адрес
const int channel = 108;          // Уникальный канал

// Настройка моторов (L298N)
const int enA = 6;
const int enB = 5;
const int in1 = 7;
const int in2 = 8;
const int in3 = 4;
const int in4 = 2;


Servo servo;
const int servoPin = 3;

struct DataPacket {
  int motorX;  
  int motorY;  
  int servoY;  
};
DataPacket receivedData;


const int deadzone = 50;  

void setup() {
  Serial.begin(9600);

  // Инициализация nRF24L01
  if (!radio.begin()) {
    Serial.println("Ошибка: модуль nRF24L01 не инициализирован");
    while (1);
  }
  radio.openReadingPipe(1, address);  // Адрес
  radio.setPALevel(RF24_PA_HIGH);     // Мощность
  radio.setDataRate(RF24_1MBPS);      // Скорость
  radio.setChannel(channel);          // Уникальный канал
  radio.startListening();             // Режим приема

  // Инициализация моторов
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  
  servo.attach(servoPin);
  servo.write(90);
}

void loop() {
  if (radio.available()) {
    
    radio.read(&receivedData, sizeof(receivedData));

    
    int motorSpeedX = map(abs(receivedData.motorX - 512), 0, 511, 0, 255);
    int motorSpeedY = map(abs(receivedData.motorY - 512), 0, 511, 0, 255);

    if (receivedData.motorY > 600) {  // Вперед
      analogWrite(enA, motorSpeedY);
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
    } else if (receivedData.motorY < 400) {  // Назад
      analogWrite(enA, motorSpeedY);
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
    } else {  
      analogWrite(enA, 0);
    }

    if (receivedData.motorX > 600) {  // Вправо
      analogWrite(enB, motorSpeedX);
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
    } else if (receivedData.motorX < 400) {  // Влево
      analogWrite(enB, motorSpeedX);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
    } else {  // Остановка по оси X
      analogWrite(enB, 0);
    }

    int servoAngle = map(receivedData.servoY, 0, 1023, 0, 180);
    servo.write(servoAngle);

    Serial.print("Motor X:");
    Serial.print(receivedData.motorX);
    Serial.print(" Motor Y:");
    Serial.print(receivedData.motorY);
    Serial.print(" Servo Y:");
    Serial.println(receivedData.servoY);
  }
}
