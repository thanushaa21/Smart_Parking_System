// ── SLOT SENSORS & LEDs ──
// Member 1 - Hardware Engineer
// Function: Reads IR sensors for 3 parking slots,
// controls green/red LEDs, updates LCD slot status

// ── PIN DEFINITIONS — Slot Sensors ────────────────────────
#define IR_SLOT1 32
#define IR_SLOT2 33
#define IR_SLOT3 25

#define GLED1 14
#define GLED2 12
#define GLED3 4

#define RLED1 15
#define RLED2 2
#define RLED3 16

bool slotOccupied[3] = {false, false, false};

// ── COUNT AVAILABLE SLOTS ─────────────────────────────────
int countAvailable() {
  int count = 0;
  for (int i = 0; i < 3; i++) {
    if (!slotOccupied[i]) count++;
  }
  return count;
}

// ── UPDATE LEDs ───────────────────────────────────────────
void updateLEDs() {
  digitalWrite(GLED1, slotOccupied[0] ? LOW : HIGH);
  digitalWrite(RLED1, slotOccupied[0] ? HIGH : LOW);
  digitalWrite(GLED2, slotOccupied[1] ? LOW : HIGH);
  digitalWrite(RLED2, slotOccupied[1] ? HIGH : LOW);
  digitalWrite(GLED3, slotOccupied[2] ? LOW : HIGH);
  digitalWrite(RLED3, slotOccupied[2] ? HIGH : LOW);
}

// ── UPDATE LCD ────────────────────────────────────────────
void updateLCD() {
  int available = countAvailable();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Available: ");
  lcd.print(available);
  lcd.print("/3");
  lcd.setCursor(0, 1);
  lcd.print("S1:");
  lcd.print(slotOccupied[0] ? "X" : "O");
  lcd.print(" S2:");
  lcd.print(slotOccupied[1] ? "X" : "O");
  lcd.print(" S3:");
  lcd.print(slotOccupied[2] ? "X" : "O");
}
