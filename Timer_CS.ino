////////////////////////////////////////////////////////////////////////////////////////////////////////
// Timer tipo CS
// Uriel Engine, Ver: Jun 2026
//
// Descripción: Timer sencillo tipo Counter Strike, en la pantalla de inicio muestra
// dos opciones, A para "armar" el dispositivo con los valores default (Password: 1234),
// B para configurar el tiempo del timer que tendra la cuenta regresiva y el password de 4 digitos.
// Con la tecla # Aceptamos en el menu y con * cancelamos o regresamos a la pantalla anterior.
// Una vez iniciado el juego y la cuenta regresiva se tendrá que presionar la tecla # para ingresar
// el password correcto, si el password es correcto el dispositivo se desactiva y se gana, 
// si el password es incorrecto la cuenta regresiva continua, si llega a cero el juego termina y se
// pierde. 
//
// Conexiones:
//   _________________________
//  |LCD I2C   | Arduino      |
//  | GND      | GND          |
//  | VCC      | VCC (5V)     |
//  | SDA      | A4           |
//  | SCL      | A5           |
//   -------------------------
//   _________________________
//  |DFPlayer  | Arduino      |
//  |PIN1  VCC | VCC (5V)     |
//  |PIN2  Rx  | PIN11        |
//  |PIN3  Tx  | PIN10        |
//  |PIN6  SPK1| NA           |     SPK1 y SPK2 se conectan a la bocina 
//  |PIN7  GND | GND          |
//  |PIN8  SPK2| NA           |
//  |PIN10 GND | GND          |
//   -------------------------
//   _________________________
//  |Teclado4x4| Arduino      |
//  | 1        | 2            |
//  | 2        | 3            |
//  | 3        | 4            |
//  | 4        | 5            |
//  | 5        | 6            |
//  | 6        | 7            |
//  | 7        | 8            |
//  | 8        | 9            |
//   --------------------------
//
//  LED -> PIN12
//
/////////////////////////////////////////////////////////////////


#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

SoftwareSerial mp3Serial(10, 11);
DFRobotDFPlayerMini playerCS;
bool dfReady = false;

const byte LED_PIN = 12;

const byte ROWS = 4;
const byte COLS = 4;

char keyMap[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[COLS] = {6, 7, 8, 9};
byte colPins[ROWS] = {2, 3, 4, 5};

Keypad keypad = Keypad(makeKeymap(keyMap), rowPins, colPins, ROWS, COLS);

const byte SND_BEEP = 1;
const byte SND_PLANTED = 2;
const byte SND_WIN = 3;
const byte SND_LOSE = 4;

const byte VOLUME = 25;

const unsigned long DEFAULT_TIME = 110;
const char DEFAULT_PASS[5] = "1234";

const unsigned long LED_PULSE_MS = 80;
const unsigned long ENTER_TIMEOUT_MS = 15000;
const unsigned long WRONG_SCREEN_MS = 5000;
const unsigned long END_SCREEN_MS = 30000;

enum State {
  MAIN_MENU,
  CONFIG_MENU,
  SET_PASSWORD,
  SET_TIME,
  COUNTDOWN,
  ENTER_CODE,
  WRONG_PASSWORD,
  WIN_SCREEN,
  LOSE_SCREEN
};

State state = MAIN_MENU;

char password[5] = "1234";
unsigned long bombTime = DEFAULT_TIME;
unsigned long remainingTime = DEFAULT_TIME;

char input[7];
byte inputLen = 0;

unsigned long lastTickMs = 0;
unsigned long stateMs = 0;
unsigned long lastDigitMs = 0;
unsigned long ledOffMs = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  lcd.init();
  lcd.backlight();

  mp3Serial.begin(9600);
  delay(700);

  if (playerCS.begin(mp3Serial)) {
    dfReady = true;
    playerCS.volume(VOLUME);
  }

  strcpy(password, DEFAULT_PASS);
  bombTime = DEFAULT_TIME;
  remainingTime = bombTime;

  goMain();
}

void loop() {
  unsigned long now = millis();
  char key = keypad.getKey();

  if (key) handleKey(key, now);

  updateTimer(now);
  updateLed(now);
  updateStateTimers(now);
}

void handleKey(char key, unsigned long now) {
  if (state == MAIN_MENU) {
    if (key == 'A') startGame(now);
    else if (key == 'B') goConfig();
  }

  else if (state == CONFIG_MENU) {
    if (key == 'A') goSetPassword();
    else if (key == 'B') goSetTime();
    else if (key == '*') goMain();
  }

  else if (state == SET_PASSWORD) {
    if (key == '*') goConfig();
    else if (key == '#') savePassword();
    else if (isDigitKey(key) && inputLen < 4) {
      input[inputLen++] = key;
      input[inputLen] = '\0';
      renderSetPassword();
    }
  }

  else if (state == SET_TIME) {
    if (key == '*') goConfig();
    else if (key == '#') saveTime();
    else if (isDigitKey(key) && inputLen < 6) {
      input[inputLen++] = key;
      input[inputLen] = '\0';
      renderSetTime();
    }
  }

  else if (state == COUNTDOWN) {
    if (key == '#') goEnterCode(now);
  }

  else if (state == ENTER_CODE) {
    if (key == '*') goCountdown();
    else if (isDigitKey(key) && inputLen < 4) {
      input[inputLen++] = key;
      input[inputLen] = '\0';
      lastDigitMs = now;
      renderEnterCode();
    }
    else if (key == '#') {
      checkCode(now);
    }
  }
}

void startGame(unsigned long now) {
  remainingTime = bombTime;
  lastTickMs = now;
  playTrack(SND_PLANTED);
  delay(2000);
  state = COUNTDOWN;
  stateMs = now;
  renderCountdown();
}

void goMain() {
  state = MAIN_MENU;
  stateMs = millis();
  digitalWrite(LED_PIN, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("A:START/PLANT");
  lcd.setCursor(0, 1);
  lcd.print("B:CONFIG");
}

void goConfig() {
  state = CONFIG_MENU;
  stateMs = millis();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("A:PASSWORD");
  lcd.setCursor(0, 1);
  lcd.print("B:TIME  *:BACK");
}

void goSetPassword() {
  state = SET_PASSWORD;
  stateMs = millis();
  clearInput();
  renderSetPassword();
}

void goSetTime() {
  state = SET_TIME;
  stateMs = millis();
  clearInput();
  renderSetTime();
}

void goCountdown() {
  state = COUNTDOWN;
  stateMs = millis();
  renderCountdown();
}

void goEnterCode(unsigned long now) {
  state = ENTER_CODE;
  stateMs = now;
  lastDigitMs = now;
  clearInput();
  renderEnterCode();
}

void goWrongPassword(unsigned long now) {
  if (remainingTime > 15) remainingTime = 15;
  lastTickMs = now;
  state = WRONG_PASSWORD;
  stateMs = now;
  renderWrongPassword();
}

void goWin(unsigned long now) {
  state = WIN_SCREEN;
  stateMs = now;
  digitalWrite(LED_PIN, LOW);
  playTrack(SND_WIN);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BOMB DEFUSED");
  lcd.setCursor(0, 1);
  lcd.print("YOU WIN");
}

void goLose(unsigned long now) {
  state = LOSE_SCREEN;
  stateMs = now;
  digitalWrite(LED_PIN, LOW);
  playTrack(SND_LOSE);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BOOM!");
  lcd.setCursor(0, 1);
  lcd.print("YOU LOSE");
}

void savePassword() {
  if (inputLen == 4) {
    for (byte i = 0; i < 4; i++) password[i] = input[i];
    password[4] = '\0';
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PASSWORD SAVED");
    delay(700);
    goConfig();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("4 DIGITS REQ");
    delay(700);
    renderSetPassword();
  }
}

void saveTime() {
  if (inputLen != 6) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("6 DIGITS REQ");
    delay(700);
    renderSetTime();
    return;
  }

  byte hh = (input[0] - '0') * 10 + (input[1] - '0');
  byte mm = (input[2] - '0') * 10 + (input[3] - '0');
  byte ss = (input[4] - '0') * 10 + (input[5] - '0');

  if (mm > 59 || ss > 59) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INVALID TIME");
    delay(700);
    renderSetTime();
    return;
  }

  unsigned long t = (unsigned long)hh * 3600UL + (unsigned long)mm * 60UL + ss;

  if (t == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TIME > 0");
    delay(700);
    renderSetTime();
    return;
  }

  bombTime = t;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TIME SAVED");
  delay(700);
  goConfig();
}

void checkCode(unsigned long now) {
  if (inputLen != 4) return;

  bool ok = true;
  for (byte i = 0; i < 4; i++) {
    if (input[i] != password[i]) ok = false;
  }

  if (ok) goWin(now);
  else goWrongPassword(now);
}

void updateTimer(unsigned long now) {
  if (!timerRunning()) return;

  while (now - lastTickMs >= 1000UL) {
    lastTickMs += 1000UL;

    if (remainingTime > 0) {
      remainingTime--;
      beepPulse(now);
    }

    if (remainingTime == 0) {
      goLose(now);
      return;
    }

    if (state == COUNTDOWN) renderCountdown();
    else if (state == WRONG_PASSWORD) renderWrongPassword();
  }
}

void updateStateTimers(unsigned long now) {
  if (state == ENTER_CODE && now - lastDigitMs >= ENTER_TIMEOUT_MS) {
    goCountdown();
  }

  else if (state == WRONG_PASSWORD && now - stateMs >= WRONG_SCREEN_MS) {
    goCountdown();
  }

  else if ((state == WIN_SCREEN || state == LOSE_SCREEN) && now - stateMs >= END_SCREEN_MS) {
    goMain();
  }
}

void updateLed(unsigned long now) {
  if (ledOffMs > 0 && now >= ledOffMs) {
    digitalWrite(LED_PIN, LOW);
    ledOffMs = 0;
  }
}

void beepPulse(unsigned long now) {
  digitalWrite(LED_PIN, HIGH);
  ledOffMs = now + LED_PULSE_MS;
  playTrack(SND_BEEP);
}

void playTrack(byte n) {
  if (dfReady) playerCS.play(n);
}

bool timerRunning() {
  return state == COUNTDOWN || state == ENTER_CODE || state == WRONG_PASSWORD;
}

bool isDigitKey(char k) {
  return k >= '0' && k <= '9';
}

void clearInput() {
  inputLen = 0;
  input[0] = '\0';
}

void renderSetPassword() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PASSWORD:");
  lcd.setCursor(0, 1);

  for (byte i = 0; i < 4; i++) {
    if (i < inputLen) lcd.print(input[i]);
    else lcd.print("_");
  }

  lcd.setCursor(6, 1);
  lcd.print("*:CAN #:OK");
}

void renderSetTime() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SET HH:MM:SS");
  lcd.setCursor(0, 1);

  char h1 = inputLen > 0 ? input[0] : '_';
  char h2 = inputLen > 1 ? input[1] : '_';
  char m1 = inputLen > 2 ? input[2] : '_';
  char m2 = inputLen > 3 ? input[3] : '_';
  char s1 = inputLen > 4 ? input[4] : '_';
  char s2 = inputLen > 5 ? input[5] : '_';

  lcd.print(h1);
  lcd.print(h2);
  lcd.print(":");
  lcd.print(m1);
  lcd.print(m2);
  lcd.print(":");
  lcd.print(s1);
  lcd.print(s2);
}

void renderCountdown() {
  char buf[9];
  formatTime(remainingTime, buf);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TIME LEFT");
  lcd.setCursor(0, 1);
  lcd.print(buf);
  lcd.setCursor(10, 1);
  lcd.print("#:DEF");
}

void renderEnterCode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTER CODE");
  lcd.setCursor(0, 1);

  for (byte i = 0; i < 4; i++) {
    if (i < inputLen) lcd.print("*");
    else lcd.print("_");
  }

  lcd.setCursor(6, 1);
  lcd.print("*:CAN #:OK");
}

void renderWrongPassword() {
  char buf[9];
  formatTime(remainingTime, buf);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WRONG PASSWORD");
  lcd.setCursor(0, 1);
  lcd.print(buf);
}

void formatTime(unsigned long total, char *buf) {
  byte hh = total / 3600UL;
  byte mm = (total % 3600UL) / 60UL;
  byte ss = total % 60UL;

  buf[0] = '0' + hh / 10;
  buf[1] = '0' + hh % 10;
  buf[2] = ':';
  buf[3] = '0' + mm / 10;
  buf[4] = '0' + mm % 10;
  buf[5] = ':';
  buf[6] = '0' + ss / 10;
  buf[7] = '0' + ss % 10;
  buf[8] = '\0';
}
 