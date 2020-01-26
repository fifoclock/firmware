/*
  FIFO Clock Firmware
  By Eddie Zhou and Connor Northway
  https://filoclock.github.io
  https://github.com/filoclock/firmware
*/

// Libraries to communicate with DS1307 over software 12C
#include <TinyWireM.h>
#include <TinyRTClib.h>

// Library to control RGBW SK6812s
#include <Adafruit_NeoPixel.h>

// Define Board Constants
#define LED_PIN     8
#define LED_COUNT   14
#define BRIGHTNESS  1   // 0-255

// Declare functions
void updateTime();
void updateAnimation(uint8_t left, uint8_t middle, uint8_t right, uint8_t newLeft, uint8_t newMiddle, uint8_t newRight);
void refreshLEDs(uint8_t left, uint8_t middle, uint8_t right);
void intToFILO(boolean LEDs[], uint8_t first, uint8_t size, uint8_t num);

// Declare RGBW SK6812 strip
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

// Declare DS1307 RTC
RTC_DS1307 rtc;

// LED options
// TODO: store in EEPROM and load in setup
uint8_t brightness = 1; // 0-255
uint32_t foregroundColor = strip.Color(  0,   0,   0, 255); // Default to white
uint32_t backgroundColor = strip.Color(  0,   0,   0,   0); // Default to off

// Animation Options
uint8_t updateAnimationDelay = 200; // ms between animation states

// Time
DateTime now;
DateTime time = DateTime(0, 0, 0 ,0 ,0, 0); // set all to 0 to get startup animation

void setup() {
  // Initialize RTC
  TinyWireM.begin();
  rtc.begin();

  // Initialize LED strip
  strip.begin();
  strip.show(); // Turn off pixels immediately
  strip.setBrightness(brightness);

  // Set RTC time to compile time if no time on RTC
  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(__DATE__, __TIME__)); // Set to compile time

    // Display red for one second
    strip.fill(strip.Color(255, 0, 0, 0));
    strip.show();
    delay(1000);
  }
}

void loop() {
  // Check for clock update
  updateTime();

  // LED refresh only necessary if there are animations while time display does not change
  // refreshLEDs(now.hour()%12, now.minute()/10, now.minute()%10);
}

// Check if the minute has changed and run animation if so
void updateTime() {
  now = rtc.now();
  if(time.minute() != now.minute()) {
    updateAnimation(time.hour()%12, time.minute()/10, time.minute()%10, now.hour()%12, now.minute()/10, now.minute()%10);
    time = now;
  }
}

// Animates between three numbers representing the old state of the LED columns and 3 numbers representing their new state
// This can be replaced to produce other animations
void updateAnimation(uint8_t left, uint8_t middle, uint8_t right, uint8_t newLeft, uint8_t newMiddle, uint8_t newRight) {
  while(left != newLeft || middle != newMiddle || right != newRight) {
    if(left != newLeft) {
      left = (left + 11) % 12;
    }
    if(middle != newMiddle) {
      middle = (middle + 5) % 6;
    }
    if(right != newRight) {
      right = (right + 9) % 10;
    }
    refreshLEDs(left, middle, right);
    delay(updateAnimationDelay);
  }
}

// Displays the FILO representation of the three given numbers
void refreshLEDs(uint8_t left, uint8_t middle, uint8_t right) {
  boolean LEDMask[14]; // Could update this to be an int to use less memory with each bit representing an led
  intToFILO(LEDMask, 0, 5, right);
  intToFILO(LEDMask, 5, 3, middle);
  intToFILO(LEDMask, 8, 6, left);

  for(uint8_t i = 0; i < 14; i++) {
    if(LEDMask[i]) {
      // The following line can be replaced to create animations
      strip.setPixelColor(i, foregroundColor);
    } else {
      // The following line can be replaced to create animations
      strip.setPixelColor(i, backgroundColor);
    }
  }
  strip.show();
}

// Creates a FILO representation of the given num in the LEDMask at a given offset, with a given column size
void intToFILO(boolean LEDMask[], uint8_t offset, uint8_t size, uint8_t num) {
  int first = (num <= size) ? 0 : num - size;
  int last = (num <= size) ? num : size;
  
  for(uint8_t i = 0; i < size; i++) {
    LEDMask[i+offset] = (i >= first && i < last);
  }
}
