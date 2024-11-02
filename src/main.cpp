  #include <Arduino.h>
  #include <SPI.h>
  #include <nRF24L01.h>
  #include <RF24.h>

  /* 
    Распиновка:
    --------------------------------
    Антенна кабельное тв:
    CE - D10, 
    CSN - D9, 
    VCC - 3.3, 
    GND - GND, 
    MOSI - D11, 
    MISO - D12, 
    SCK - D13
    --------------------------------
    Питание:
    9 Вольт крона для платы
    7,4 Вольт для моторов 18650 с понижайкой до 6 lm2596s
    --------------------------------
    UPD 02.11:
      - Усилен сигнал командой palevel(high)
      - Задержка 50 мс для пульта
      - Тонкая настройка ШИМа моторов с аналоговым подключением и формулами
    
  */

  RF24 radio(10, 9); 

  // Пины для мотора A
  const int PWMA = 2; // PWM для мотора A
  const int AIN1 = 3; // Направление мотора A
  const int AIN2 = 4; // Направление мотора A

  // Пины для мотора B
  const int PWMB = 7; // PWM для мотора B
  const int BIN1 = 5; // Направление мотора B
  const int BIN2 = 6; // Направление мотора B

  // Настройки для работы джойстика
  const int threshold = 50; // Порог для центрирования
  const int centerValue = 512; // Центр осей джойстика 

  void setup() {
    Serial.begin(9600); 
    // 415 база ответье
    if (!radio.begin()) {
      Serial.println("Ошибка: модуль не инициализирован");
      while (1); // 415 база не вышла на связь
    }
    Serial.println("Модуль успешно инициализирован");

    // Настройка приемника деда
    radio.setChannel(76); // 76 канал с мультиками по тв
    radio.setPayloadSize(32); // Полезная нагрузка(качка тренировка)
    radio.setDataRate(RF24_1MBPS); // Скорость передачи данных
    radio.openReadingPipe(1, 0xF0F0F0F0E1LL); // Адрес 
    radio.startListening(); // Слушать музыку
    radio.setPALevel(RF24_PA_HIGH); // Усиление сигнала до high, чтобы все было красиво

    pinMode(PWMA, OUTPUT);
    pinMode(PWMB, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
  }

  void loop() {
    if (radio.available()) {
      int joystickData[2] = {0, 0};
      radio.read(&joystickData, sizeof(joystickData));  

      int xValue = joystickData[0];
      int yValue = joystickData[1];

      // Настройка значения PWM в зависимости от расстояния до центра джойстика
      int pwmValue = map(abs(yValue - centerValue), 0, 512, 100, 255); // Нижний предел для управления скоростью

      // Определение направления движения и управление моторами
      if (yValue > centerValue + threshold) {  
        // Вперед
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
        analogWrite(PWMA, map(yValue, centerValue + threshold, 1023, 100, 255));

        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
        analogWrite(PWMB, map(yValue, centerValue + threshold, 1023, 100, 255));

        Serial.println("Движение: Вперед");

      } else if (yValue < centerValue - threshold) {  
        // Назад
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        analogWrite(PWMA, map(centerValue - yValue, 0, centerValue - threshold, 100, 255));

        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
        analogWrite(PWMB, map(centerValue - yValue, 0, centerValue - threshold, 100, 255));

        Serial.println("Движение: Назад");

      } else if (xValue > centerValue + threshold) {  
        // Вправо
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
        analogWrite(PWMA, map(xValue, centerValue + threshold, 1023, 100, 255));

        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
        analogWrite(PWMB, map(xValue, centerValue + threshold, 1023, 100, 255));

        Serial.println("Движение: Вправо");

      } else if (xValue < centerValue - threshold) {  
        // Влево
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        analogWrite(PWMA, map(centerValue - xValue, 0, centerValue - threshold, 100, 255));

        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
        analogWrite(PWMB, map(centerValue - xValue, 0, centerValue - threshold, 100, 255));

        Serial.println("Движение: Влево");

      } else {  
        // Стоп
        analogWrite(PWMA, 0);
        analogWrite(PWMB, 0);
        Serial.println("Движение: Стоп");
      }

      // Задержка приема сигналов
      delay(50);
    }
  }
