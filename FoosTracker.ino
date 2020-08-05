#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// SCREEN ---

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define SDA_WHITE 18
#define SCL_WHITE 19

#define SDA_BLUE 21
#define SCL_BLUE 22

TwoWire I2CWhite = TwoWire(0);
TwoWire I2CBlue = TwoWire(1);

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 displayWhite(SCREEN_WIDTH, SCREEN_HEIGHT, &I2CWhite, OLED_RESET);
Adafruit_SSD1306 displayBlue(SCREEN_WIDTH, SCREEN_HEIGHT, &I2CBlue, OLED_RESET);

// BUTTON ---

struct Button
{
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
  unsigned long timePressed;
};

Button buttonReset = {23, 0, false, millis()};

void IRAM_ATTR interruptResetButton()
{
  // Button pressed
  if (millis() - buttonReset.timePressed > 100) // Prevent double clicks
  {
    buttonReset.numberKeyPresses++;
    buttonReset.pressed = true;
    buttonReset.timePressed = millis();
  }
}

// SENSORS ---

#define OBJSENSOR_WHITE 15
#define OBJSENSOR_BLUE 2

int pressesWhite = 0;
int pressesBlue = 0;

unsigned long pressTimeWhite = millis();
unsigned long pressTimeBlue = millis();

void IRAM_ATTR interruptGoalOnWhite()
{
  if (millis() - pressTimeWhite > 1000) // Prevent double clicks
  {
    pressesWhite++;
    pressTimeWhite = millis();
  }
}

void IRAM_ATTR interruptGoalOnBlue()
{
  if (millis() - pressTimeBlue > 1000) // Prevent double clicks
  {
    pressesBlue++;
    pressTimeBlue = millis();
  }
}

// MAIN ---

void setup()
{
  Serial.begin(9600);

  // Init I2C
  I2CWhite.begin(SDA_WHITE, SCL_WHITE, 100000);
  I2CBlue.begin(SDA_BLUE, SCL_BLUE, 100000);

  // Setup displays
  displaySetup(displayWhite);
  displaySetup(displayBlue);

  // Show initial display buffer contents on the screen
  displayWhite.display();
  displayBlue.display();

  // Init button
  pinMode(buttonReset.PIN, INPUT_PULLUP);
  attachInterrupt(buttonReset.PIN, interruptResetButton, FALLING);

  // Init sensors
  pinMode(OBJSENSOR_WHITE, INPUT_PULLUP);
  pinMode(OBJSENSOR_BLUE, INPUT_PULLUP);
  attachInterrupt(OBJSENSOR_WHITE, interruptGoalOnWhite, FALLING);
  attachInterrupt(OBJSENSOR_BLUE, interruptGoalOnBlue, FALLING);
}

void loop()
{
  drawScore(displayWhite, digitalRead(OBJSENSOR_WHITE));
  drawScore(displayBlue, digitalRead(OBJSENSOR_BLUE));
  delay(1000);
}

void displaySetup(Adafruit_SSD1306 &display)
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.setTextSize(4); // Draw 4X-scale text
  display.setTextColor(SSD1306_WHITE);
}

void drawScore(Adafruit_SSD1306 &display, int16_t score)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(score);
  display.display();
}