/* Smart Traffic Light - Arduino (Uno)
   - Two-way intersection (Lane A, Lane B)
   - Virtual IR sensors extend green time when vehicles are present
   - Counts cars per green cycle and saves totals to EEPROM
   - Works in Wokwi / Tinkercad / Arduino IDE. Uses built-in EEPROM library.
*/

#include <EEPROM.h>

// Pin mapping (change as needed)
const uint8_t A_RED   = 2;
const uint8_t A_YELLOW= 3;
const uint8_t A_GREEN = 4;
const uint8_t B_RED   = 5;
const uint8_t B_YELLOW= 6;
const uint8_t B_GREEN = 7;

const uint8_t IR_A = 8;   // IR sensor digital output for Lane A
const uint8_t IR_B = 9;   // IR sensor digital output for Lane B

// Timing (ms)
const unsigned long BASE_GREEN_MS  = 5000UL; // base green time
const unsigned long YELLOW_MS      = 2000UL; // yellow time
const unsigned long RED_BUFFER_MS  = 200UL;  // small buffer between phases
const unsigned long EXTEND_STEP_MS = 2000UL; // time added per detection
const unsigned long MAX_GREEN_MS   = 15000UL; // max allowed green

// EEPROM addresses for storing totals (4 bytes each using unsigned long)
const int EE_ADDR_TOTAL_A = 0;   // 4 bytes
const int EE_ADDR_TOTAL_B = 4;   // 4 bytes

// runtime counters (per-cycle)
unsigned int passedThisGreenA = 0;
unsigned int passedThisGreenB = 0;

// helper: write/read unsigned long safely
void eeWriteUL(int addr, unsigned long val){
  EEPROM.put(addr, val);
}
unsigned long eeReadUL(int addr){
  unsigned long v = 0;
  EEPROM.get(addr, v);
  return v;
}

void setupPins(){
  pinMode(A_RED, OUTPUT);
  pinMode(A_YELLOW, OUTPUT);
  pinMode(A_GREEN, OUTPUT);
  pinMode(B_RED, OUTPUT);
  pinMode(B_YELLOW, OUTPUT);
  pinMode(B_GREEN, OUTPUT);

  pinMode(IR_A, INPUT_PULLUP); // assume IR sensor pulls low when detecting ORients vary; use appropriate wiring
  pinMode(IR_B, INPUT_PULLUP);

  digitalWrite(A_RED, LOW);
  digitalWrite(A_YELLOW, LOW);
  digitalWrite(A_GREEN, LOW);
  digitalWrite(B_RED, LOW);
  digitalWrite(B_YELLOW, LOW);
  digitalWrite(B_GREEN, LOW);
}

void setup(){
  Serial.begin(115200);
  setupPins();

  // Initialize EEPROM if first run (optional)
  // (We won't overwrite existing totals; keep preloaded)
  Serial.println("Smart Traffic Light starting...");
  unsigned long totA = eeReadUL(EE_ADDR_TOTAL_A);
  unsigned long totB = eeReadUL(EE_ADDR_TOTAL_B);
  Serial.print("Stored totals - Lane A: "); Serial.print(totA);
  Serial.print("  Lane B: "); Serial.println(totB);
}

// Basic LED helpers
void red_on(uint8_t rPin, uint8_t yPin, uint8_t gPin){
  digitalWrite(rPin, HIGH);
  digitalWrite(yPin, LOW);
  digitalWrite(gPin, LOW);
}
void yellow_on(uint8_t rPin, uint8_t yPin, uint8_t gPin){
  digitalWrite(rPin, LOW);
  digitalWrite(yPin, HIGH);
  digitalWrite(gPin, LOW);
}
void green_on(uint8_t rPin, uint8_t yPin, uint8_t gPin){
  digitalWrite(rPin, LOW);
  digitalWrite(yPin, LOW);
  digitalWrite(gPin, HIGH);
}
void all_off_lane(uint8_t rPin, uint8_t yPin, uint8_t gPin){
  digitalWrite(rPin, LOW);
  digitalWrite(yPin, LOW);
  digitalWrite(gPin, LOW);
}

// Read IR sensor (return true if vehicle detected).
// Assumes INPUT_PULLUP; adjust if you're using active HIGH sensors.
bool vehicle_detected(uint8_t irPin){
  // Simple digital read. If your simulated IR returns LOW on detection, invert if needed.
  int val = digitalRead(irPin);
  // On many IR modules, output LOW when object detected, but simulator could be opposite.
  // We'll treat LOW as detected (with INPUT_PULLUP).
  return (val == LOW);
}

// Counts vehicles passing during green by simple edge detection (low->high or high->low depends on sensor).
// For simple sims we sample occasionally; in real hardware you'd want interrupt or better debouncing.
bool detect_pulse(uint8_t irPin, bool &lastState){
  bool s = (digitalRead(irPin) == LOW);
  if (s != lastState){
    // state changed
    lastState = s;
    if (s) return true; // rising of detection (entering)
  }
  return false;
}

void extendable_green_phase(uint8_t laneR, uint8_t laneY, uint8_t laneG, uint8_t irPin, unsigned long baseMs, unsigned long &detectedPasses){
  unsigned long greenStart = millis();
  unsigned long allowed = baseMs;
  bool lastState = (digitalRead(irPin) == LOW);
  detectedPasses = 0;

  green_on(laneR, laneY, laneG);
  Serial.print("Green ON for pin "); Serial.print(laneG); Serial.print(" base=");
  Serial.print(baseMs); Serial.println("ms");

  // active loop for green with possible extend
  while (millis() - greenStart < allowed){
    // simple short wait and check
    delay(50);

    // If vehicle detected, extend up to MAX_GREEN_MS and increment pass counter
    if (vehicle_detected(irPin)){
      // naive debouncing: count only if at least 300ms since last increment
      // We'll just increment and wait a small window to avoid flood counting.
      detectedPasses++;
      Serial.print("Vehicle detected on pin "); Serial.print(irPin); Serial.print(" (");
      Serial.print(detectedPasses); Serial.println(") -- extending green");

      // extend allowed time, but cap at MAX_GREEN_MS
      unsigned long elapsed = millis() - greenStart;
      unsigned long newAllowed = elapsed + EXTEND_STEP_MS;
      if (newAllowed > MAX_GREEN_MS) newAllowed = MAX_GREEN_MS;
      allowed = newAllowed;

      // small delay to avoid multiple counts for same car (tunable)
      delay(600);
    }
  }

  // turn off green for lane (leave as is; caller will rotate states)
  all_off_lane(laneR, laneY, laneG);
  Serial.print("Green phase ended after ");
  Serial.print(millis() - greenStart);
  Serial.println(" ms");
}

void saveTotalsToEEPROM(unsigned long addA, unsigned long addB){
  unsigned long totA = eeReadUL(EE_ADDR_TOTAL_A);
  unsigned long totB = eeReadUL(EE_ADDR_TOTAL_B);
  totA += addA;
  totB += addB;
  eeWriteUL(EE_ADDR_TOTAL_A, totA);
  eeWriteUL(EE_ADDR_TOTAL_B, totB);
  Serial.print("Saved totals -> A: "); Serial.print(totA);
  Serial.print("  B: "); Serial.println(totB);
}

void loop(){
  // Default: Lane A green, Lane B red
  red_on(B_RED, B_YELLOW, B_GREEN); // B red
  delay(RED_BUFFER_MS);

  unsigned long passedA = 0;
  extendable_green_phase(A_RED, A_YELLOW, A_GREEN, IR_A, BASE_GREEN_MS, passedA);

  // Yellow for A
  yellow_on(A_RED, A_YELLOW, A_GREEN);
  delay(YELLOW_MS);
  all_off_lane(A_RED, A_YELLOW, A_GREEN);

  // Short all-red for safety
  red_on(A_RED, A_YELLOW, A_GREEN);
  red_on(B_RED, B_YELLOW, B_GREEN);
  delay(RED_BUFFER_MS);

  // Now Lane B green
  red_on(A_RED, A_YELLOW, A_GREEN); // A red
  delay(RED_BUFFER_MS);

  unsigned long passedB = 0;
  extendable_green_phase(B_RED, B_YELLOW, B_GREEN, IR_B, BASE_GREEN_MS, passedB);

  // Yellow for B
  yellow_on(B_RED, B_YELLOW, B_GREEN);
  delay(YELLOW_MS);
  all_off_lane(B_RED, B_YELLOW, B_GREEN);

  // Save counts to EEPROM (aggregate)
  saveTotalsToEEPROM(passedA, passedB);

  // Print cycle summary
  Serial.print("Cycle summary: passedA=");
  Serial.print(passedA);
  Serial.print("  passedB=");
  Serial.println(passedB);

  // optional small pause
  delay(300);
}
