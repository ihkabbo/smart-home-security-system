#include <Keypad.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// ============================================================
// LCD
// ============================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============================================================
// SERVO
// ============================================================

Servo servo;

// ============================================================
// PIN DEFINITIONS
// ============================================================

#define SERVO_PIN 10
#define BUZZER    11
#define GREEN     12
#define RED       13
#define RESET_BTN A0

// ============================================================
// KEYPAD CONFIGURATION
// ============================================================

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Your keypad wiring:
//
// Keypad Pin 1 = C4 -> D9
// Keypad Pin 2 = C3 -> D8
// Keypad Pin 3 = C2 -> D7
// Keypad Pin 4 = C1 -> D6
//
// Keypad Pin 5 = R4 -> D5
// Keypad Pin 6 = R3 -> D4
// Keypad Pin 7 = R2 -> D3
// Keypad Pin 8 = R1 -> D2

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);

// ============================================================
// PASSWORD VARIABLES
// ============================================================

String storedPassword = "";

const byte MIN_PASSWORD_LENGTH = 4;
const byte MAX_PASSWORD_LENGTH = 8;

// ============================================================
// SECURITY VARIABLES
// ============================================================

byte wrongAttempts = 0;

// Used to tell the program that factory reset was performed
bool resetPerformed = false;

// ============================================================
// FSM STATES
// ============================================================

enum SystemState {
  LOCKED,
  AUTHENTICATION,
  MENU,
  DOOR_OPEN,
  CHANGE_PASSWORD,
  ALARM
};

SystemState currentState = LOCKED;

// ============================================================
// FUNCTION DECLARATIONS
// ============================================================

void savePassword(String password);
bool loadPassword();
void clearEEPROM();

void lockDoor();
void openDoor();

void beepOnce();
void wrongBeep();

bool resetButtonPressed();

String getPassword(const char* message, bool hidden);

void firstPasswordSetup();
bool changePassword();

void alarmMode();
void authenticatedMenu();

void factoryReset();

// ============================================================
// SAVE PASSWORD TO EEPROM
// ============================================================

void savePassword(String password) {

  EEPROM.update(0, 1);

  EEPROM.update(1, password.length());

  for (byte i = 0; i < password.length(); i++) {
    EEPROM.update(2 + i, password[i]);
  }

  storedPassword = password;
}

// ============================================================
// LOAD PASSWORD FROM EEPROM
// ============================================================

bool loadPassword() {

  if (EEPROM.read(0) != 1) {
    return false;
  }

  byte length = EEPROM.read(1);

  if (length < MIN_PASSWORD_LENGTH ||
      length > MAX_PASSWORD_LENGTH) {

    return false;
  }

  storedPassword = "";

  for (byte i = 0; i < length; i++) {
    storedPassword += (char)EEPROM.read(2 + i);
  }

  return true;
}

// ============================================================
// CLEAR EEPROM
// ============================================================

void clearEEPROM() {

  EEPROM.update(0, 0);
  EEPROM.update(1, 0);

  storedPassword = "";
}

// ============================================================
// LOCK DOOR
// ============================================================

void lockDoor() {

  servo.write(0);

  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, LOW);
}

// ============================================================
// OPEN DOOR
// ============================================================

void openDoor() {

  servo.write(90);

  digitalWrite(RED, LOW);
  digitalWrite(GREEN, HIGH);
}

// ============================================================
// NORMAL SUCCESS BEEP
// ============================================================

void beepOnce() {

  digitalWrite(BUZZER, HIGH);
  delay(120);
  digitalWrite(BUZZER, LOW);
}

// ============================================================
// WRONG PASSWORD - THREE SHORT BEEPS
// ============================================================

void wrongBeep() {

  for (byte i = 0; i < 3; i++) {

    digitalWrite(BUZZER, HIGH);
    delay(120);

    digitalWrite(BUZZER, LOW);
    delay(100);
  }
}

// ============================================================
// RESET BUTTON CHECK
// ============================================================

bool resetButtonPressed() {

  if (digitalRead(RESET_BTN) == LOW) {

    delay(50);

    if (digitalRead(RESET_BTN) == LOW) {

      // Wait until button is released
      while (digitalRead(RESET_BTN) == LOW) {
        delay(10);
      }

      return true;
    }
  }

  return false;
}

// ============================================================
// PASSWORD INPUT
//
// hidden = false
// Password is visible.
//
// hidden = true
// Password is displayed as ****
// ============================================================

String getPassword(const char* message, bool hidden) {

  lcd.clear();
  lcd.print(message);

  lcd.setCursor(0, 1);

  String input = "";

  while (true) {

    // --------------------------------------------------------
    // CHECK FACTORY RESET BUTTON
    // --------------------------------------------------------

    if (resetButtonPressed()) {

      factoryReset();

      // Return special value so the current operation stops
      return "";
    }

    char key = keypad.getKey();

    if (!key) {
      continue;
    }

    // ========================================================
    // ENTER
    // ========================================================

    if (key == '#') {

      if (input.length() >= MIN_PASSWORD_LENGTH) {
        return input;
      }

      lcd.clear();
      lcd.print("Min 4 Digits");

      delay(1000);

      lcd.clear();
      lcd.print(message);

      lcd.setCursor(0, 1);

      if (hidden) {

        for (byte i = 0; i < input.length(); i++) {
          lcd.print('*');
        }

      } else {

        lcd.print(input);
      }
    }

    // ========================================================
    // CLEAR
    // ========================================================

    else if (key == '*') {

      input = "";

      lcd.setCursor(0, 1);
      lcd.print("                ");

      lcd.setCursor(0, 1);
    }

    // ========================================================
    // DIGIT
    // ========================================================

    else if (key >= '0' && key <= '9') {

      if (input.length() < MAX_PASSWORD_LENGTH) {

        input += key;

        if (hidden) {
          lcd.print('*');
        } else {
          lcd.print(key);
        }

      } else {

        lcd.clear();
        lcd.print("Max 8 Digits");

        delay(1000);

        lcd.clear();
        lcd.print(message);

        lcd.setCursor(0, 1);

        if (hidden) {

          for (byte i = 0; i < input.length(); i++) {
            lcd.print('*');
          }

        } else {

          lcd.print(input);
        }
      }
    }
  }
}

// ============================================================
// FIRST PASSWORD SETUP
// ============================================================

void firstPasswordSetup() {

  while (true) {

    String password1 =
      getPassword("Set New Password", false);

    // If reset was triggered again, restart setup
    if (password1 == "") {
      continue;
    }

    String password2 =
      getPassword("Confirm Password", false);

    if (password2 == "") {
      continue;
    }

    if (password1 == password2) {

      savePassword(password1);

      beepOnce();

      lcd.clear();
      lcd.print("Password Saved");

      delay(1500);

      return;
    }

    else {

      wrongBeep();

      lcd.clear();
      lcd.print("Password");

      lcd.setCursor(0, 1);
      lcd.print("Mismatch!");

      delay(1500);
    }
  }
}

// ============================================================
// CHANGE PASSWORD
// ============================================================

bool changePassword() {

  currentState = CHANGE_PASSWORD;

  // ----------------------------------------------------------
  // CURRENT PASSWORD
  // ----------------------------------------------------------

  String currentPassword =
    getPassword("Current Password", false);

  if (currentPassword == "") {
    return false;
  }

  if (currentPassword != storedPassword) {

    wrongBeep();

    lcd.clear();
    lcd.print("Wrong Password");

    delay(1200);

    return false;
  }

  beepOnce();

  lcd.clear();
  lcd.print("New Password");

  delay(700);

  // ----------------------------------------------------------
  // NEW PASSWORD
  // ----------------------------------------------------------

  String newPassword =
    getPassword("New Password", false);

  if (newPassword == "") {
    return false;
  }

  // ----------------------------------------------------------
  // CONFIRM NEW PASSWORD
  // ----------------------------------------------------------

  String confirmPassword =
    getPassword("Confirm Password", false);

  if (confirmPassword == "") {
    return false;
  }

  if (newPassword == confirmPassword) {

    savePassword(newPassword);

    beepOnce();

    lcd.clear();
    lcd.print("Password");

    lcd.setCursor(0, 1);
    lcd.print("Changed!");

    delay(1500);

    return true;
  }

  else {

    wrongBeep();

    lcd.clear();
    lcd.print("Mismatch!");

    delay(1200);

    return false;
  }
}

// ============================================================
// ALARM MODE
// ============================================================

void alarmMode() {

  currentState = ALARM;

  // ==========================================================
  // INITIAL 3-BEEP WARNING
  // ==========================================================

  wrongBeep();

  // ==========================================================
  // ALARM WARNING FOR APPROX. 2.5 SECONDS
  // ==========================================================

  unsigned long alarmStart = millis();

  while (millis() - alarmStart < 2500) {

    // Check reset button during initial alarm
    if (resetButtonPressed()) {

      digitalWrite(BUZZER, LOW);
      digitalWrite(RED, LOW);

      factoryReset();

      return;
    }

    digitalWrite(RED, HIGH);
    digitalWrite(BUZZER, HIGH);

    delay(150);

    digitalWrite(RED, LOW);
    digitalWrite(BUZZER, LOW);

    delay(100);
  }

  // ==========================================================
  // CONTINUOUS ALARM
  // ==========================================================

  lcd.clear();
  lcd.print("!!! ALARM !!!");

  lcd.setCursor(0, 1);
  lcd.print("Recovery Needed");

  while (true) {

    // --------------------------------------------------------
    // FACTORY RESET BUTTON
    // --------------------------------------------------------

    if (resetButtonPressed()) {

      digitalWrite(BUZZER, LOW);
      digitalWrite(RED, LOW);

      factoryReset();

      return;
    }

    // --------------------------------------------------------
    // CONTINUOUS BUZZER
    // --------------------------------------------------------

    digitalWrite(RED, HIGH);
    digitalWrite(BUZZER, HIGH);

    // --------------------------------------------------------
    // RECOVERY ATTEMPT 1
    // --------------------------------------------------------

    String recovery1 =
      getPassword("Recovery 1/2", true);

    // If factory reset was activated
    if (recovery1 == "") {
      return;
    }

    digitalWrite(BUZZER, LOW);
    digitalWrite(RED, LOW);

    // --------------------------------------------------------
    // RECOVERY ATTEMPT 2
    // --------------------------------------------------------

    String recovery2 =
      getPassword("Recovery 2/2", true);

    // If factory reset was activated
    if (recovery2 == "") {
      return;
    }

    // --------------------------------------------------------
    // CHECK BOTH PASSWORDS
    // --------------------------------------------------------

    bool firstCorrect =
      (recovery1 == storedPassword);

    bool secondCorrect =
      (recovery2 == storedPassword);

    // ========================================================
    // BOTH CORRECT
    // ========================================================

    if (firstCorrect && secondCorrect) {

      digitalWrite(BUZZER, LOW);
      digitalWrite(RED, LOW);

      beepOnce();

      lcd.clear();
      lcd.print("Recovery");

      lcd.setCursor(0, 1);
      lcd.print("Successful");

      delay(1500);

      wrongAttempts = 0;

      // ------------------------------------------------------
      // OPEN DOOR
      // ------------------------------------------------------

      openDoor();

      beepOnce();

      lcd.clear();
      lcd.print("Door Open");

      delay(1500);

      // ------------------------------------------------------
      // RETURN TO AUTHENTICATED MENU
      // ------------------------------------------------------

      authenticatedMenu();

      return;
    }

    // ========================================================
    // ONE OR BOTH WRONG
    // ========================================================

    else {

      digitalWrite(BUZZER, LOW);
      digitalWrite(RED, LOW);

      wrongBeep();

      lcd.clear();
      lcd.print("Wrong Password");

      lcd.setCursor(0, 1);
      lcd.print("Try Recovery");

      delay(1500);

      // Alarm starts again
      lcd.clear();
      lcd.print("!!! ALARM !!!");

      lcd.setCursor(0, 1);
      lcd.print("Recovery Needed");
    }
  }
}

// ============================================================
// AUTHENTICATED MENU
//
// Correct password automatically opens the door.
//
// Menu:
// A = Change Password
// B = Close
// ============================================================

void authenticatedMenu() {

  currentState = MENU;

  // ==========================================================
  // AUTOMATICALLY OPEN DOOR
  // ==========================================================

  openDoor();

  beepOnce();

  lcd.clear();
  lcd.print("Access Granted");

  lcd.setCursor(0, 1);
  lcd.print("Door Open");

  delay(1500);

  // ==========================================================
  // MENU
  // ==========================================================

  while (true) {

    // Check reset button
    if (resetButtonPressed()) {

      factoryReset();

      return;
    }

    lcd.clear();
    lcd.print("A:Change Pass");

    lcd.setCursor(0, 1);
    lcd.print("B:Close");

    char key = 0;

    while (!key) {

      // Check reset button while waiting for keypad
      if (resetButtonPressed()) {

        factoryReset();

        return;
      }

      key = keypad.getKey();
    }

    // ========================================================
    // A = CHANGE PASSWORD
    // ========================================================

    if (key == 'A') {

      beepOnce();

      changePassword();
    }

    // ========================================================
    // B = CLOSE
    // ========================================================

    else if (key == 'B') {

      lockDoor();

      beepOnce();

      lcd.clear();
      lcd.print("Door Closed");

      delay(1200);

      currentState = LOCKED;

      return;
    }
  }
}

// ============================================================
// FACTORY RESET
// ============================================================

void factoryReset() {

  currentState = LOCKED;

  // ----------------------------------------------------------
  // STOP ALARM / SECURE DOOR
  // ----------------------------------------------------------

  digitalWrite(BUZZER, LOW);
  digitalWrite(RED, LOW);

  lockDoor();

  // ----------------------------------------------------------
  // CLEAR OLD PASSWORD
  // ----------------------------------------------------------

  clearEEPROM();

  beepOnce();

  // ----------------------------------------------------------
  // SET NEW PASSWORD
  // ----------------------------------------------------------

  lcd.clear();
  lcd.print("Set New Password");

  delay(1000);

  firstPasswordSetup();

  // ----------------------------------------------------------
  // RETURN TO LOCKED STATE
  // ----------------------------------------------------------

  lockDoor();

  lcd.clear();
  lcd.print("System Ready");

  lcd.setCursor(0, 1);
  lcd.print("Door Locked");

  delay(1500);

  resetPerformed = true;
}

// ============================================================
// SETUP
// ============================================================

void setup() {

  // ----------------------------------------------------------
  // PIN MODES
  // ----------------------------------------------------------

  pinMode(BUZZER, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);

  // Internal pull-up resistor
  pinMode(RESET_BTN, INPUT_PULLUP);

  digitalWrite(BUZZER, LOW);

  // ----------------------------------------------------------
  // SERVO
  // ----------------------------------------------------------

  servo.attach(SERVO_PIN);

  // ----------------------------------------------------------
  // LCD
  // ----------------------------------------------------------

  lcd.init();
  lcd.backlight();

  // ----------------------------------------------------------
  // START LOCKED
  // ----------------------------------------------------------

  lockDoor();

  lcd.clear();
  lcd.print("Smart Security");

  lcd.setCursor(0, 1);
  lcd.print("System Starting");

  delay(1500);

  // ----------------------------------------------------------
  // LOAD PASSWORD
  // ----------------------------------------------------------

  if (!loadPassword()) {

    firstPasswordSetup();
  }

  currentState = LOCKED;
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {

  // ==========================================================
  // FACTORY RESET BUTTON
  //
  // INPUT_PULLUP:
  // LOW = BUTTON PRESSED
  // ==========================================================

  if (resetButtonPressed()) {

    factoryReset();

    return;
  }

  // ==========================================================
  // LEVEL 1 SECURITY
  // LOCKED
  // ==========================================================

  currentState = AUTHENTICATION;

  // ----------------------------------------------------------
  // NORMAL PASSWORD IS VISIBLE
  // ----------------------------------------------------------

  String enteredPassword =
    getPassword("Enter Password", false);

  // If factory reset happened
  if (enteredPassword == "") {
    return;
  }

  // ==========================================================
  // CORRECT PASSWORD
  // ==========================================================

  if (enteredPassword == storedPassword) {

    wrongAttempts = 0;

    beepOnce();

    lcd.clear();
    lcd.print("Access Granted");

    delay(1000);

    // --------------------------------------------------------
    // AUTOMATICALLY OPEN DOOR
    // --------------------------------------------------------

    authenticatedMenu();
  }

  // ==========================================================
  // WRONG PASSWORD
  // ==========================================================

  else {

    wrongAttempts++;

    // --------------------------------------------------------
    // THREE SHORT BEEPS
    // --------------------------------------------------------

    wrongBeep();

    // --------------------------------------------------------
    // DISPLAY ATTEMPT
    // --------------------------------------------------------

    lcd.clear();
    lcd.print("Wrong Password");

    lcd.setCursor(0, 1);

    lcd.print("Attempt: ");
    lcd.print(wrongAttempts);
    lcd.print("/3");

    delay(1500);

    // ========================================================
    // THIRD WRONG ATTEMPT
    // ========================================================

    if (wrongAttempts >= 3) {

      alarmMode();
    }
  }
}