/*
  Проверка связи Arduino Mega 2560 <-> ESP8266
  ---------------------------------------------
  Просто отправляет "AT" и смотрит, ответит ли модуль "OK".
  Ничего не настраивает и никуда не подключается —
  только диагностика самого канала связи (UART).

  Если через ~10 секунд связи нет:
   - проверьте положение переключателя на плате (режим ESP, не программирования)
   - попробуйте поменять ESP_PORT ниже (Serial1 / Serial2 / Serial3)
   - попробуйте другую скорость (частые варианты: 115200, 9600, 74880)
*/

#define espSerial Serial1     // на каком Serial висит ESP8266
#define ESP_BAUD 115200       // скорость ESP

const unsigned long CHECK_INTERVAL = 3000; // проверка раз в 3 сек
unsigned long lastCheck = 0;

const int LED_PIN = 13; // встроенный светодиод как индикатор

void setup() {
  Serial.begin(115200);
  espSerial.begin(ESP_BAUD);
  pinMode(LED_PIN, OUTPUT);

  Serial.println(F("=== Проверка связи с ESP8266 ==="));
}

void loop() {
  if (millis() - lastCheck >= CHECK_INTERVAL) {
    lastCheck = millis();
    checkConnection();
  }
}

void checkConnection() {
  // очищаем то, что могло накопиться в буфере до этого
  while (espSerial.available()) {
    espSerial.read();
  }

  espSerial.println("AT");

  String response = "";
  unsigned long start = millis();
  while (millis() - start < 1500) {   // ждём ответ до 1.5 сек
    while (espSerial.available()) {
      response += (char)espSerial.read();
    }
    if (response.indexOf("OK") != -1) break;
  }

  bool connected = response.indexOf("OK") != -1;

  if (connected) {
    Serial.println(F("[OK] ESP8266 отвечает, связь есть"));
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println(F("[FAIL] Нет ответа от ESP8266"));
    if (response.length() > 0) {
      Serial.print(F("  Получено (не 'OK'): "));
      Serial.println(response);
    } else {
      Serial.println(F("  Модуль вообще молчит — проверьте порт/скорость/переключатель"));
    }
    digitalWrite(LED_PIN, LOW);
  }
}
