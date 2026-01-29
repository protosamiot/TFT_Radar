/*
 * Project: Ultrasonic Radar System on 1.8" TFT Display (Semi-Circle Radar)
 * Microcontroller: Arduino Nano
 * Display: 1.8" TFT SPI Display (ST7735 - 128x160)
 * Sensor: HC-SR04 Ultrasonic Sensor
 * Alert: Buzzer (Danger Alert)
 * GitHub: https://github.com/ProtoSamIoT
 * YouTube: https://www.youtube.com/@protosamiot
 * Date: January 29, 2026
 *
 * Description:
 * This project creates a live Ultrasonic Radar system using an Arduino Nano,
 * a 1.8" SPI TFT display (ST7735), an HC-SR04 ultrasonic sensor, and a buzzer.
 * The TFT display shows a semi-circle radar interface with a sweeping scan line.
 * The object distance is measured in real-time and displayed as a red dot on the radar.
 *
 * Features:
 * - Semi-circle radar UI with scan sweep animation
 * - Live distance measurement (0 to 50 cm range)
 * - Red dot marks object position on radar (LIVE ONLY)
 * - Danger alert at <= 15 cm:
 *   - Continuous buzzer ON
 *   - Red dot blinks for warning indication
 */

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <math.h>

/* TFT Pins (Same connections as your previous TFT project) */
#define TFT_CS   10
#define TFT_DC    8
#define TFT_RST   9

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

/* Ultrasonic Pins */
#define TRIG_PIN 6
#define ECHO_PIN 7

/* Buzzer Pin */
#define BUZZER_PIN 5

/* Colors (RGB565) */
#define BLACK   0x0000
#define GREEN   0x07E0
#define RED     0xF800
#define WHITE   0xFFFF
#define CYAN    0x07FF
#define YELLOW  0xFFE0

/* Radar Settings */
#define MAX_DISTANCE_CM      50
#define DANGER_DISTANCE_CM   15

/* Display size in Landscape Mode */
const int SCREEN_W = 160;
const int SCREEN_H = 128;

/* Radar center point (bottom middle of screen) */
const int CX = 80;
const int CY = 120;

/* Radar radius (REDUCED to avoid overlapping with text) */
const int RADIUS = 85;

/* Blink control */
bool blinkState = false;
unsigned long lastBlinkTime = 0;

/* Track last drawn sweep */
int lastAngle = 180;

/* Locked object position */
bool objectLocked = false;
int lockedAngle = 270;
int lockedDistance = 999;

/* Track last drawn locked dot */
int lastDotX = -1;
int lastDotY = -1;

/* Function to read distance in cm from HC-SR04 */
long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // timeout ~25ms
  if (duration == 0) return 999; // No echo detected

  long distance = duration * 0.034 / 2;
  return distance;
}

/* Startup Screen */
void showStartupScreen() {
  tft.fillScreen(BLACK);
  tft.setTextSize(2);
  tft.setTextColor(CYAN);
  tft.setCursor(10, 45);
  tft.print("Radar by");

  tft.setTextColor(WHITE);
  tft.setCursor(10, 70);
  tft.print("ProtoSam IoT");

  delay(1500);
}

/* Draw radar background (semi-circle + range arcs + angle lines) */
void drawRadarBackground() {
  tft.fillScreen(BLACK);

  // Radar Title
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.setCursor(5, 5);
  tft.print("ProtoSam IoT Radar");

  // Range text
  tft.setCursor(5, 15);
  tft.setTextColor(YELLOW);
  tft.print("Range: 0-50cm");

  tft.setCursor(5, 25);
  tft.setTextColor(RED);
  tft.print("Danger <= 15cm");

  // Semi-circle arcs (SOLID)
  for (int r = 20; r < RADIUS; r += 20) {
    for (int a = 180; a <= 360; a++) {
      float rad = a * DEG_TO_RAD;
      int x = CX + r * cos(rad);
      int y = CY + r * sin(rad);

      if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        tft.drawPixel(x, y, GREEN);
      }
    }
  }

  // Angle lines (SOLID)
  for (int a = 180; a <= 360; a += 30) {
    float rad = a * DEG_TO_RAD;
    int x2 = CX + RADIUS * cos(rad);
    int y2 = CY + RADIUS * sin(rad);
    tft.drawLine(CX, CY, x2, y2, GREEN);
  }
}

/* Draw radar sweep line */
void drawSweepLine(int angle, uint16_t color) {
  float rad = angle * DEG_TO_RAD;
  int x2 = CX + RADIUS * cos(rad);
  int y2 = CY + RADIUS * sin(rad);
  tft.drawLine(CX, CY, x2, y2, color);
}

/*
  PERFECT FIX (NO FLICKER):
  We erase sweep line by drawing BLACK line,
  then redraw radar background pattern at same angle in GREEN.
*/
void eraseSweepLine(int angle) {
  drawSweepLine(angle, BLACK);  // erase sweep
  drawSweepLine(angle, GREEN);  // restore radar solid
}

/* Get dot position */
bool getDotPosition(int angle, int distanceCM, int &x, int &y) {
  if (distanceCM > MAX_DISTANCE_CM) return false;

  int r = map(distanceCM, 0, MAX_DISTANCE_CM, 0, RADIUS);

  float rad = angle * DEG_TO_RAD;
  x = CX + r * cos(rad);
  y = CY + r * sin(rad);

  return true;
}

/* Draw dot */
void drawDot(int x, int y, uint16_t color) {
  tft.fillCircle(x, y, 3, color);
}

/* Erase old locked dot WITHOUT damaging radar background */
void eraseOldDot() {
  if (lastDotX != -1 && lastDotY != -1) {
    tft.fillCircle(lastDotX, lastDotY, 3, BLACK);

    // Restore green radar pixels under the dot
    for (int a = 180; a <= 360; a += 1) {
      for (int r = 20; r < RADIUS; r += 20) {
        float rad = a * DEG_TO_RAD;
        int x = CX + r * cos(rad);
        int y = CY + r * sin(rad);

        if (abs(x - lastDotX) <= 3 && abs(y - lastDotY) <= 3) {
          tft.drawPixel(x, y, GREEN);
        }
      }
    }

    // Restore angle lines under dot
    for (int a = 180; a <= 360; a += 30) {
      float rad = a * DEG_TO_RAD;
      int x2 = CX + RADIUS * cos(rad);
      int y2 = CY + RADIUS * sin(rad);
      tft.drawLine(CX, CY, x2, y2, GREEN);
    }
  }
}

/* Show distance text on top right */
void showDistanceText(int distanceCM) {
  tft.fillRect(110, 5, 50, 15, BLACK);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.setCursor(110, 5);

  if (distanceCM > MAX_DISTANCE_CM) {
    tft.print("Out");
  } else {
    tft.print(distanceCM);
    tft.print("cm");
  }
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1); // Landscape Mode

  // Startup screen first
  showStartupScreen();

  // Draw radar background ONCE (solid, no flicker)
  drawRadarBackground();

  // Draw first sweep line
  drawSweepLine(lastAngle, CYAN);
}

void loop() {

  // Sweep from 180 to 360
  for (int angle = 180; angle <= 360; angle += 3) {

    eraseSweepLine(lastAngle);
    eraseOldDot();

    int dist = readDistanceCM();
    showDistanceText(dist);

    drawSweepLine(angle, CYAN);

    // ✅ FIX: LIVE ONLY (No storing last position)
    if (dist <= MAX_DISTANCE_CM) {
      objectLocked = true;
      lockedAngle = angle;
      lockedDistance = dist;
    } else {
      objectLocked = false;
      lockedDistance = 999;
    }

    // Continuous buzzer ON when danger
    if (dist <= DANGER_DISTANCE_CM) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }

    // Blink only in danger
    if (lockedDistance <= DANGER_DISTANCE_CM) {
      if (millis() - lastBlinkTime > 150) {
        blinkState = !blinkState;
        lastBlinkTime = millis();
      }
    } else {
      blinkState = true;
    }

    // Draw dot only when object is currently detected
    int x, y;
    if (objectLocked && getDotPosition(lockedAngle, lockedDistance, x, y)) {
      if (blinkState) {
        drawDot(x, y, RED);
        lastDotX = x;
        lastDotY = y;
      } else {
        lastDotX = -1;
        lastDotY = -1;
      }
    } else {
      lastDotX = -1;
      lastDotY = -1;
    }

    lastAngle = angle;
    delay(30);
  }

  // Sweep back from 360 to 180
  for (int angle = 360; angle >= 180; angle -= 3) {

    eraseSweepLine(lastAngle);
    eraseOldDot();

    int dist = readDistanceCM();
    showDistanceText(dist);

    drawSweepLine(angle, CYAN);

    // ✅ FIX: LIVE ONLY (No storing last position)
    if (dist <= MAX_DISTANCE_CM) {
      objectLocked = true;
      lockedAngle = angle;
      lockedDistance = dist;
    } else {
      objectLocked = false;
      lockedDistance = 999;
    }

    if (dist <= DANGER_DISTANCE_CM) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }

    if (lockedDistance <= DANGER_DISTANCE_CM) {
      if (millis() - lastBlinkTime > 150) {
        blinkState = !blinkState;
        lastBlinkTime = millis();
      }
    } else {
      blinkState = true;
    }

    int x, y;
    if (objectLocked && getDotPosition(lockedAngle, lockedDistance, x, y)) {
      if (blinkState) {
        drawDot(x, y, RED);
        lastDotX = x;
        lastDotY = y;
      } else {
        lastDotX = -1;
        lastDotY = -1;
      }
    } else {
      lastDotX = -1;
      lastDotY = -1;
    }

    lastAngle = angle;
    delay(30);
  }
}