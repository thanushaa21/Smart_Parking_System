// ── PIN DEFINITIONS — Gate Control ───
#define IR_EXIT  26
#define RFID_SS  5
#define RFID_RST 27
#define SERVO_PIN 13

MFRC522 rfid(RFID_SS, RFID_RST);
Servo gateServo;

// ── AUTHORIZED UIDs ───
byte authorizedCard[] = {0x29, 0xB7, 0x61, 0x11};
byte authorizedFob[]  = {0x66, 0x9D, 0x22, 0xB8};

bool gateOpen = false;
unsigned long gateTimer = 0;
const unsigned long GATE_DELAY = 3000;
unsigned long lastRFIDTime = 0;

bool entryInProgress = false;
unsigned long entryTime = 0;
const unsigned long ENTRY_COOLDOWN = 5000;

// ── CHECK IF UID MATCHES ───
bool uidMatches(byte* authorized) {
  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != authorized[i]) return false;
  }
  return true;
}

// ── OPEN GATE ────
void openGate() {
  gateServo.write(90);
  gateOpen = true;
  gateTimer = millis();

 entryInProgress = true;
  entryTime = millis();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Granted!");
  lcd.setCursor(0, 1);
  lcd.print("Gate Opening...");
  Serial.println("Gate OPEN");
  updateBlynk();
}

// ── CLOSE GATE ────
void closeGate() {
  gateServo.write(0);
  gateOpen = false;
  entryInProgress = false;
  Serial.println("Gate CLOSED");
  updateLCD();
  updateBlynk();
}

