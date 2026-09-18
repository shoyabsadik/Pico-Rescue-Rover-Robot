
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//================ MOTOR PINS ====================

#define LEFT_IN1 7
#define LEFT_IN2 8
#define LEFT_EN 5

#define RIGHT_IN1 9
#define RIGHT_IN2 10
#define RIGHT_EN 6

//================ SENSOR PINS ===================

#define FLAME_PIN 4
#define SOIL_PIN A0

#define TRIG_PIN 11
#define ECHO_PIN 12

//================ FIRE SYSTEM ===================

#define PUMP_PIN 2
#define BUZZER_PIN 13
#define SERVO_PIN 3

//================ LCD ===========================

#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

//================ COMMANDS ======================

#define CMD_FORWARD 'F'
#define CMD_REVERSE 'B'
#define CMD_LEFT 'L'
#define CMD_RIGHT 'R'

#define CMD_FORWARD_RIGHT 'I'
#define CMD_FORWARD_LEFT 'G'

#define CMD_REVERSE_RIGHT 'J'
#define CMD_REVERSE_LEFT 'H'

#define CMD_STOP 'S'

#define CMD_OBSTACLE_ON 'W'
#define CMD_OBSTACLE_OFF 'w'

#define CMD_HAND_ON 'X'
#define CMD_HAND_OFF 'x'

//================ SPEED =========================

int motorSpeed = 180;

//================ FIRE SETTINGS =================

int FIRE_SPEED = 180;

unsigned long FIRE_FORWARD_TIME = 800;
unsigned long FIRE_SPRAY_TIME = 3000;

//================ SERVO =========================

int SERVO_REST = 90;
int SERVO_MAX = 0;

//================ HAND FOLLOW ===================

int HAND_FAR = 15;
int HAND_NEAR = 8;

//================ OBSTACLE ======================

int OBSTACLE_DISTANCE = 20;

unsigned long AVOID_BACK_TIME = 300;
unsigned long AVOID_TURN_TIME = 500;

//================================================

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
Servo nozzleServo;

//================ ROBOT MODE ====================

enum RobotMode {
  RC_MODE,
  OBSTACLE_MODE,
  HAND_MODE
};

RobotMode currentMode = RC_MODE;

//================ OBSTACLE STATE ================

enum AvoidState {
  AVOID_NORMAL,
  AVOID_REVERSE,
  AVOID_TURN
};

AvoidState avoidState = AVOID_NORMAL;

//================ VARIABLES =====================

int soilValue = 0;

float distanceCM = -1;

unsigned long lastSoilUpdate = 0;
unsigned long lastDistanceUpdate = 0;
unsigned long avoidTimer = 0;

//================ FIRE VARIABLES ================

bool fireRunning = false;

unsigned long fireTimer = 0;

int oldMotorSpeed = 180;

//================================================
// LCD LINE WRITER
//================================================

void writeLine(byte row, const char *text) {

  char line[17];

  snprintf(line, sizeof(line), "%-16s", text);

  lcd.setCursor(0, row);
  lcd.print(line);
}

//================================================
// LCD TOP LINE
//================================================

void showTopLine(const char *mode) {

  char line[17];

  snprintf(
    line,
    sizeof(line),
    "%-9sS:%3d%%",
    mode,
    soilValue
  );

  lcd.setCursor(0, 0);
  lcd.print(line);
}

//================================================
// LCD STATUS
//================================================

void showStatus(const char *mode, const char *status) {

  showTopLine(mode);

  writeLine(1, status);
}

//================================================
// SOIL SENSOR
//================================================

void updateSoil() {

  int rawValue = analogRead(SOIL_PIN);

  soilValue = map(
    rawValue,
    1023,
    0,
    0,
    100
  );

  soilValue = constrain(
    soilValue,
    0,
    100
  );
}

//================================================
// ULTRASONIC DISTANCE
//================================================

float getDistance() {

  digitalWrite(TRIG_PIN, LOW);

  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);

  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(
    ECHO_PIN,
    HIGH,
    12000
  );

  if (duration == 0) {
    return -1;
  }

  return duration / 58.82;
}

//================================================
// UPDATE DISTANCE
//================================================

void updateDistance() {

  if (
    millis() - lastDistanceUpdate >= 70
  ) {

    lastDistanceUpdate = millis();

    distanceCM = getDistance();
  }
}

//================================================
// MOTOR FORWARD
//================================================

void forward() {

  analogWrite(
    LEFT_EN,
    motorSpeed
  );

  analogWrite(
    RIGHT_EN,
    motorSpeed
  );

  digitalWrite(
    LEFT_IN1,
    HIGH
  );

  digitalWrite(
    LEFT_IN2,
    LOW
  );

  digitalWrite(
    RIGHT_IN1,
    HIGH
  );

  digitalWrite(
    RIGHT_IN2,
    LOW
  );
}

//================================================
// MOTOR REVERSE
//================================================

void reverse() {

  analogWrite(
    LEFT_EN,
    motorSpeed
  );

  analogWrite(
    RIGHT_EN,
    motorSpeed
  );

  digitalWrite(
    LEFT_IN1,
    LOW
  );

  digitalWrite(
    LEFT_IN2,
    HIGH
  );

  digitalWrite(
    RIGHT_IN1,
    LOW
  );

  digitalWrite(
    RIGHT_IN2,
    HIGH
  );
}

//================================================
// MOTOR LEFT
//================================================

void left() {

  analogWrite(
    LEFT_EN,
    motorSpeed
  );

  analogWrite(
    RIGHT_EN,
    motorSpeed
  );

  digitalWrite(
    LEFT_IN1,
    LOW
  );

  digitalWrite(
    LEFT_IN2,
    HIGH
  );

  digitalWrite(
    RIGHT_IN1,
    HIGH
  );

  digitalWrite(
    RIGHT_IN2,
    LOW
  );
}

//================================================
// MOTOR RIGHT
//================================================

void right() {

  analogWrite(
    LEFT_EN,
    motorSpeed
  );

  analogWrite(
    RIGHT_EN,
    motorSpeed
  );

  digitalWrite(
    LEFT_IN1,
    HIGH
  );

  digitalWrite(
    LEFT_IN2,
    LOW
  );

  digitalWrite(
    RIGHT_IN1,
    LOW
  );

  digitalWrite(
    RIGHT_IN2,
    HIGH
  );
}

//================================================
// FORWARD LEFT
//================================================

void forwardLeft() {

  analogWrite(
    LEFT_EN,
    0
  );

  analogWrite(
    RIGHT_EN,
    motorSpeed
  );

  digitalWrite(
    LEFT_IN1,
    LOW
  );

  digitalWrite(
    LEFT_IN2,
    LOW
  );

  digitalWrite(
    RIGHT_IN1,
    HIGH
  );

  digitalWrite(
    RIGHT_IN2,
    LOW
  );
}

//================================================
// FORWARD RIGHT
//================================================

void forwardRight() {

  analogWrite(
    LEFT_EN,
    motorSpeed
  );

  analogWrite(
    RIGHT_EN,
    0
  );

  digitalWrite(
    LEFT_IN1,
    HIGH
  );

  digitalWrite(
    LEFT_IN2,
    LOW
  );

  digitalWrite(
    RIGHT_IN1,
    LOW
  );

  digitalWrite(
    RIGHT_IN2,
    LOW
  );
}

//================================================
// REVERSE LEFT
//================================================

void reverseLeft() {

  analogWrite(
    LEFT_EN,
    0
  );

  analogWrite(
    RIGHT_EN,
    motorSpeed
  );

  digitalWrite(
    LEFT_IN1,
    LOW
  );

  digitalWrite(
    LEFT_IN2,
    LOW
  );

  digitalWrite(
    RIGHT_IN1,
    LOW
  );

  digitalWrite(
    RIGHT_IN2,
    HIGH
  );
}

//================================================
// REVERSE RIGHT
//================================================

void reverseRight() {

  analogWrite(
    LEFT_EN,
    motorSpeed
  );

  analogWrite(
    RIGHT_EN,
    0
  );

  digitalWrite(
    LEFT_IN1,
    LOW
  );

  digitalWrite(
    LEFT_IN2,
    HIGH
  );

  digitalWrite(
    RIGHT_IN1,
    LOW
  );

  digitalWrite(
    RIGHT_IN2,
    LOW
  );
}

//================================================
// STOP MOTOR
//================================================

void stopMotor() {

  analogWrite(
    LEFT_EN,
    0
  );

  analogWrite(
    RIGHT_EN,
    0
  );

  digitalWrite(
    LEFT_IN1,
    LOW
  );

  digitalWrite(
    LEFT_IN2,
    LOW
  );

  digitalWrite(
    RIGHT_IN1,
    LOW
  );

  digitalWrite(
    RIGHT_IN2,
    LOW
  );
}

//================================================
// FIRE START
//================================================

void startFire() {

  if (fireRunning) {
    return;
  }

  fireRunning = true;

  oldMotorSpeed = motorSpeed;

  stopMotor();

  motorSpeed = FIRE_SPEED;

  // FIRE = SERVO 90 DEGREE
  nozzleServo.write(SERVO_MAX);

  showStatus(
    "FIRE",
    "MOVING FORWARD"
  );

  forward();

  fireTimer = millis();
}

//================================================
// FIRE PROCESS
//================================================

void processFire() {

  if (!fireRunning) {
    return;
  }

  unsigned long elapsed =
    millis() - fireTimer;

  //================ FORWARD =====================

  if (
    elapsed < FIRE_FORWARD_TIME
  ) {

    return;
  }

  //================ SPRAY =======================

  if (
    elapsed <
    FIRE_FORWARD_TIME + 300
  ) {

    stopMotor();

    motorSpeed = oldMotorSpeed;

    digitalWrite(
      BUZZER_PIN,
      HIGH
    );

    digitalWrite(
      PUMP_PIN,
      HIGH
    );

    // SERVO STAYS AT 90
    nozzleServo.write(
      SERVO_MAX
    );

    showStatus(
      "FIRE",
      "SPRAYING WATER"
    );

    return;
  }

  //================ FIRE END ===================

  if (
    elapsed >=
    FIRE_FORWARD_TIME +
    300 +
    FIRE_SPRAY_TIME
  ) {

    digitalWrite(
      PUMP_PIN,
      LOW
    );

    digitalWrite(
      BUZZER_PIN,
      LOW
    );

    // NORMAL = 0 DEGREE
    nozzleServo.write(
      SERVO_REST
    );

    stopMotor();

    motorSpeed = oldMotorSpeed;

    fireRunning = false;

    showStatus(
      "FIRE",
      "SYSTEM READY"
    );
  }
}

//================================================
// OBSTACLE MODE
//================================================

void obstacleAvoiding() {

  updateDistance();

  //================ REVERSE =====================

  if (
    avoidState ==
    AVOID_REVERSE
  ) {

    reverse();

    showTopLine(
      "OBSTACLE"
    );

    writeLine(
      1,
      "REVERSING"
    );

    if (
      millis() - avoidTimer >=
      AVOID_BACK_TIME
    ) {

      stopMotor();

      avoidState =
        AVOID_TURN;

      avoidTimer =
        millis();
    }

    return;
  }

  //================ TURN ========================

  if (
    avoidState ==
    AVOID_TURN
  ) {

    right();

    showTopLine(
      "OBSTACLE"
    );

    writeLine(
      1,
      "TURNING RIGHT"
    );

    if (
      millis() - avoidTimer >=
      AVOID_TURN_TIME
    ) {

      stopMotor();

      avoidState =
        AVOID_NORMAL;

      avoidTimer =
        millis();
    }

    return;
  }

  //================ SENSOR ERROR ===============

  if (
    distanceCM < 0
  ) {

    stopMotor();

    showStatus(
      "OBSTACLE",
      "SENSOR ERROR"
    );

    return;
  }

  //================ CLEAR =======================

  if (
    distanceCM >
    OBSTACLE_DISTANCE
  ) {

    forward();

    showTopLine(
      "OBSTACLE"
    );

    char line[17];

    snprintf(
      line,
      sizeof(line),
      "D:%-13.1f",
      distanceCM
    );

    writeLine(
      1,
      line
    );

  }

  //================ OBSTACLE ====================

  else {

    stopMotor();

    showStatus(
      "OBSTACLE",
      "OBSTACLE!"
    );

    avoidState =
      AVOID_REVERSE;

    avoidTimer =
      millis();
  }
}

//================================================
// HAND FOLLOW MODE
//================================================

void handFollowing() {

  updateDistance();

  //================ SENSOR ERROR ===============

  if (
    distanceCM < 0
  ) {

    stopMotor();

    showStatus(
      "HAND",
      "SENSOR ERROR"
    );

    return;
  }

  //================ FAR =========================

  if (
    distanceCM > HAND_FAR
  ) {

    forward();

    showTopLine(
      "HAND"
    );

    char line[17];

    snprintf(
      line,
      sizeof(line),
      "FORWARD %.1fcm",
      distanceCM
    );

    writeLine(
      1,
      line
    );
  }

  //================ HOLD ========================

  else if (
    distanceCM >= HAND_NEAR
  ) {

    stopMotor();

    showTopLine(
      "HAND"
    );

    char line[17];

    snprintf(
      line,
      sizeof(line),
      "HOLDING %.1fcm",
      distanceCM
    );

    writeLine(
      1,
      line
    );
  }

  //================ NEAR ========================

  else {

    reverse();

    showTopLine(
      "HAND"
    );

    char line[17];

    snprintf(
      line,
      sizeof(line),
      "REVERSE %.1fcm",
      distanceCM
    );

    writeLine(
      1,
      line
    );
  }
}

//================================================
// BLUETOOTH
//================================================

void checkBluetooth() {

  while (
    Serial.available() > 0
  ) {

    char c =
      Serial.read();

    if (
      c == '\n' ||
      c == '\r'
    ) {

      continue;
    }

    //================ OBSTACLE ON ==============

    if (
      c == CMD_OBSTACLE_ON
    ) {

      currentMode =
        OBSTACLE_MODE;

      avoidState =
        AVOID_NORMAL;

      stopMotor();

      showStatus(
        "OBSTACLE",
        "MODE ON"
      );

      continue;
    }

    //================ OBSTACLE OFF =============

    if (
      c == CMD_OBSTACLE_OFF
    ) {

      currentMode =
        RC_MODE;

      avoidState =
        AVOID_NORMAL;

      stopMotor();

      showStatus(
        "RC",
        "OBSTACLE OFF"
      );

      continue;
    }

    //================ HAND ON ===================

    if (
      c == CMD_HAND_ON
    ) {

      currentMode =
        HAND_MODE;

      stopMotor();

      showStatus(
        "HAND",
        "MODE ON"
      );

      continue;
    }

    //================ HAND OFF ==================

    if (
      c == CMD_HAND_OFF
    ) {

      currentMode =
        RC_MODE;

      stopMotor();

      showStatus(
        "RC",
        "HAND OFF"
      );

      continue;
    }

    //================ SPEED =====================

    if (
      c >= '0' &&
      c <= '9'
    ) {

      motorSpeed =
        map(
          c - '0',
          0,
          9,
          80,
          245
        );

      continue;
    }

    if (
      c == 'q'
    ) {

      motorSpeed = 255;

      continue;
    }

    //================ FORWARD ===================

    if (
      c == CMD_FORWARD
    ) {

      currentMode =
        RC_MODE;

      forward();

      showStatus(
        "RC",
        "FORWARD"
      );
    }

    //================ REVERSE ===================

    else if (
      c == CMD_REVERSE
    ) {

      currentMode =
        RC_MODE;

      reverse();

      showStatus(
        "RC",
        "REVERSE"
      );
    }

    //================ LEFT ======================

    else if (
      c == CMD_LEFT
    ) {

      currentMode =
        RC_MODE;

      left();

      showStatus(
        "RC",
        "LEFT"
      );
    }

    //================ RIGHT =====================

    else if (
      c == CMD_RIGHT
    ) {

      currentMode =
        RC_MODE;

      right();

      showStatus(
        "RC",
        "RIGHT"
      );
    }

    //================ FWD RIGHT =================

    else if (
      c == CMD_FORWARD_RIGHT
    ) {

      currentMode =
        RC_MODE;

      forwardRight();

      showStatus(
        "RC",
        "FWD RIGHT"
      );
    }

    //================ FWD LEFT ==================

    else if (
      c == CMD_FORWARD_LEFT
    ) {

      currentMode =
        RC_MODE;

      forwardLeft();

      showStatus(
        "RC",
        "FWD LEFT"
      );
    }

    //================ REV RIGHT ================

    else if (
      c == CMD_REVERSE_RIGHT
    ) {

      currentMode =
        RC_MODE;

      reverseRight();

      showStatus(
        "RC",
        "REV RIGHT"
      );
    }

    //================ REV LEFT ==================

    else if (
      c == CMD_REVERSE_LEFT
    ) {

      currentMode =
        RC_MODE;

      reverseLeft();

      showStatus(
        "RC",
        "REV LEFT"
      );
    }

    //================ STOP ======================

    else if (
      c == CMD_STOP
    ) {

      currentMode =
        RC_MODE;

      stopMotor();

      showStatus(
        "RC",
        "STOP"
      );
    }
  }
}

//================================================
// SETUP
//================================================

void setup() {

  //================ MOTOR =======================

  pinMode(
    LEFT_IN1,
    OUTPUT
  );

  pinMode(
    LEFT_IN2,
    OUTPUT
  );

  pinMode(
    LEFT_EN,
    OUTPUT
  );

  pinMode(
    RIGHT_IN1,
    OUTPUT
  );

  pinMode(
    RIGHT_IN2,
    OUTPUT
  );

  pinMode(
    RIGHT_EN,
    OUTPUT
  );

  //================ SENSORS =====================

  pinMode(
    FLAME_PIN,
    INPUT
  );

  pinMode(
    SOIL_PIN,
    INPUT
  );

  pinMode(
    TRIG_PIN,
    OUTPUT
  );

  pinMode(
    ECHO_PIN,
    INPUT
  );

  //================ FIRE ========================

  pinMode(
    PUMP_PIN,
    OUTPUT
  );

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  //================ INITIAL =====================

  stopMotor();

  digitalWrite(
    PUMP_PIN,
    LOW
  );

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  //================ SERVO =======================

  nozzleServo.attach(
    SERVO_PIN
  );

  // NORMAL POSITION
  nozzleServo.write(
    SERVO_REST
  );

  //================ LCD =========================

  lcd.init();

  lcd.backlight();

  //================ BLUETOOTH ===================

  Serial.begin(9600);

  //================ INITIAL SOIL ===============

  updateSoil();

  showStatus(
    "FIRE FIGHTER",
    "ROBOT"
  );

  delay(700);

  showStatus(
    "RC",
    "SYSTEM READY"
  );

  delay(500);
}

//================================================
// MAIN LOOP
//================================================

void loop() {

  //================ SOIL ========================

  if (
    millis() - lastSoilUpdate >= 300
  ) {

    lastSoilUpdate =
      millis();

    updateSoil();
  }

  //================ FIRE ========================

  if (
    !fireRunning &&
    digitalRead(FLAME_PIN) == LOW
  ) {

    startFire();
  }

  processFire();

  //================ BLUETOOTH ==================

  checkBluetooth();

  //================ FIRE RUNNING ===============

  if (fireRunning) {
    return;
  }

  //================ MODE ========================

  if (
    currentMode ==
    OBSTACLE_MODE
  ) {

    obstacleAvoiding();

  }

  else if (
    currentMode ==
    HAND_MODE
  ) {

    handFollowing();
  }

  //================ SMALL DELAY ================

  delay(5);
}
