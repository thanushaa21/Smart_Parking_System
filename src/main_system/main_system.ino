#define BLYNK_TEMPLATE_ID "TMPL62-Wbv9g6"
#define BLYNK_TEMPLATE_NAME "Smart Parking System"
#define BLYNK_AUTH_TOKEN "KsFOHqME9oh8UhGTYhNYzwnTu6e2IqSc"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <MPU6050.h>

// ── WiFi credentials ──────────────────────────────────────
char ssid[] = "sha";
char pass[] = "12345678";

LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

// ── UPDATE BLYNK ──────────────────────────────────────────
void updateBlynk() {
  Blynk.virtualWrite(V0, slotOccupied[0] ? 1 : 0);
  Blynk.virtualWrite(V1, slotOccupied[1] ? 1 : 0);
  Blynk.virtualWrite(V2, slotOccupied[2] ? 1 : 0);
  Blynk.virtualWrite(V3, gateOpen ? 1 : 0);
  Blynk.virtualWrite(V4, countAvailable());
}

// ── SETUP ─────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);
  delay(100);

  pinMode(IR_SLOT1, INPUT);
  pinMode(IR_SLOT2, INPUT);
  pinMode(IR_SLOT3, INPUT);
  pinMode(IR_EXIT,  INPUT);

  pinMode(GLED1, OUTPUT); pinMode(GLED2, OUTPUT); pinMode(GLED3, OUTPUT);
  pinMode(RLED1, OUTPUT); pinMode(RLED2, OUTPUT); pinMode(RLED3, OUTPUT);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  gateServo.attach(SERVO_PIN);
  gateServo.write(0);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Parking");
  lcd.setCursor(0, 1);
  lcd.print("System Ready!");
  delay(2000);
  lcd.clear();

  SPI.begin(18, 19, 23, RFID_SS);
  rfid.PCD_Init();

  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 ready!");
  } else {
    Serial.println("MPU6050 failed!");
  }

  updateLEDs();
  updateLCD();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(2000L, updateBlynk);

  Serial.println("System Initialized!");
}

// ── MAIN LOOP ─────────────────────────────────────────────
void loop() {
  Blynk.run();
  timer.run();

  bool current[3];
  current[0] = (digitalRead(IR_SLOT1) == LOW);
  current[1] = (digitalRead(IR_SLOT2) == LOW);
  current[2] = (digitalRead(IR_SLOT3) == LOW);

  bool changed = false;
  for (int i = 0; i < 3; i++) {
    if (current[i] != slotOccupied[i]) {
      slotOccupied[i] = current[i];
      changed = true;
      Serial.print("Slot ");
      Serial.print(i + 1);
      Serial.println(slotOccupied[i] ? " OCCUPIED" : " EMPTY");
    }
  }
  if (changed) {
    updateLEDs();
    if (!gateOpen) updateLCD();
    updateBlynk();
  }

  bool exitCooldownOver = (millis() - entryTime > ENTRY_COOLDOWN);
  if (digitalRead(IR_EXIT) == LOW && !gateOpen && exitCooldownOver) {
    Serial.println("Exit detected — opening gate");
    openGate();
  }

  if (millis() - lastRFIDTime > 3000) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      lastRFIDTime = millis();

      Serial.print("Card UID: ");
      for (byte i = 0; i < 4; i++) {
        Serial.print(rfid.uid.uidByte[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      bool authorized = uidMatches(authorizedCard) || uidMatches(authorizedFob);

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();

      if (authorized) {
        if (countAvailable() > 0) {
          openGate();
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Parking FULL!");
          lcd.setCursor(0, 1);
          lcd.print("Sorry :(");
          digitalWrite(BUZZER, HIGH);
          delay(500);
          digitalWrite(BUZZER, LOW);
          delay(2000);
          updateLCD();
        }
      } else {
        triggerAlarm("Unauthorized Card !!!");
      }
    }
  }

  if (gateOpen && (millis() - gateTimer > GATE_DELAY)) {
    closeGate();
  }

  bool alarmCooledDown = (millis() - lastAlarmTime > ALARM_COOLDOWN);
  if (alarmCooledDown) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    int totalAccel = abs(ax) + abs(ay) + abs(az);

    if (totalAccel > VIBRATION_THRESHOLD) {
      triggerAlarm("Tampering!");
    }
  }

  delay(100);
}
