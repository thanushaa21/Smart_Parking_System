// ── PIN DEFINITIONS — Security ───
#define BUZZER 17

MPU6050 mpu;

const int VIBRATION_THRESHOLD = 55000;
unsigned long lastAlarmTime = 0;
const unsigned long ALARM_COOLDOWN = 10000;

// ── TRIGGER ALARM ───────
void triggerAlarm(String reason) {
  lastAlarmTime = millis();
  Serial.println("ALARM: " + reason);

  Blynk.virtualWrite(V5, reason);
  Blynk.logEvent("alert", reason);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("!! ALERT !!");
  lcd.setCursor(0, 1);
  lcd.print(reason);
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(300);
    digitalWrite(BUZZER, LOW);
    delay(200);
  }
  delay(1000);
  updateLCD();

  Blynk.virtualWrite(V5, "No Alerts");
}
