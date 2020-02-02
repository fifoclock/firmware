/**
 * @brief FIFO Clock Firmware
 * @author Eddie Zhou
 * @author Connor Northway
 * @version 1.1
 * @date 2020-02-02
 * 
 * https://fifoclock.github.io
 * https://github.com/fifoclock/firmware
 */

#include <TinyWireM.h>
#include <TinyRTClib.h>
#include <Adafruit_NeoPixel.h>

// Configurable Firmware Options
#define BRIGHTNESS      1   // 0-255
#define ANIMATION_DELAY 200

// Board Constants
#define UP          3
#define DOWN        2
#define LEFT        7
#define RIGHT       0
#define CENTER      1
#define LED_PIN     8
#define LED_COUNT   14

// Function Declarations
void check_switch();
void update_time();
void update_animation(uint8_t left,     uint8_t middle,     uint8_t right, 
                     uint8_t new_left, uint8_t new_middle, uint8_t new_right);
void set_LEDs(uint8_t left, uint8_t middle, uint8_t right);
void set_column(boolean LEDs[], uint8_t first, uint8_t size, uint8_t num);

// Global Variables
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
RTC_DS1307 rtc;
DateTime now;
DateTime time = DateTime(0, 0, 0 ,0 ,0, 0); // set all to 0 to get startup animation

// LED options
// TODO: store in EEPROM and load in setup
uint8_t brightness = 1; // 0-255
uint32_t foregroundColor = strip.Color(  0,   0,   0, 255); // Default to white
uint32_t backgroundColor = strip.Color(  0,   0,   0,   0); // Default to off

/**
 * @brief Configure IO for LEDs, RTC and 5-way switch
 * 
 * Sets 5-way switch inputs high to use internal pullups.
 * Initializes LED strip object, and configures brighness.
 * Sets RTC time to compile time if the RTC does not have a time saved.
 */
void setup() {
  // Initialize RTC
  TinyWireM.begin();
  rtc.begin();

  // Input Pins
//  pinMode(UP, INPUT);
//  pinMode(DOWN, INPUT);
//  pinMode(LEFT, INPUT);
//  pinMode(RIGHT, INPUT);
//  pinMode(CENTER, INPUT);
  digitalWrite(UP, 1);
  digitalWrite(DOWN, 1);
  digitalWrite(LEFT, 1);
  digitalWrite(RIGHT, 1);
  digitalWrite(CENTER, 1);

  // Initialize LED strip
  strip.begin();
  strip.show(); // Turn off pixels immediately
  strip.setBrightness(brightness);

  // Set RTC time to compile time if no time on RTC
  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(__DATE__, __TIME__)); // Set to compile time

    // Display red for one second to indicate new RTC time
    strip.fill(strip.Color(255, 0, 0, 0));
    strip.show();
    delay(1000);
  }
}

/**
 * @brief Main loop that runs continuously
 * 
 * Checks for switch inputs and updates the LEDs when the minute changes.
 */
void loop() {
  //check_switch();
  update_time();
}

/**
 * @brief Process 5-way switch inputs
 * 
 * Toggles the LEDs on/off if the switch is pressed in.
 * Adjusts minute if switch is held up/down.
 * Changes color if switch is held left/right.
 */
void check_switch() {
  if(digitalRead(UP) == LOW && brightness < 255) {
    brightness = 1;
    strip.setBrightness(brightness);
    strip.show();
    delay(50);
  }
  if(digitalRead(DOWN) == LOW && brightness > 1) {
    brightness = 0;
    strip.setBrightness(brightness);
    strip.show();
    delay(50);
  }
}

/**
 * @brief Update the LEDs to match the current minute
 * 
 * Updates the now global variable to the rtc's time.
 * Calls update_animation if the old time is behind a minute.
 * Updates time to be the new minute.
 */
void update_time() {
  now = rtc.now();
  if(time.minute() != now.minute()) {
    update_animation(time.hour()%12, time.minute()/10, time.minute()%10, 
                    now.hour()%12,  now.minute()/10,  now.minute()%10);
    time = now;
  }
}

/**
 * @brief Animates between two states on the FIFO display
 * 
 * Updates the LEDs to slowly change states to the new number being displayed.
 * 
 * @param left        The current number from 0-11 displayed on the left LED column
 * @param middle      The current number from 0-5 displayed on the middle LED column
 * @param right       The current number from 0-9 displayed on the right LED column
 * @param new_left    The new number from 0-11 to display on the left LED column
 * @param new_middle  The new number from 0-5 to display on the middle LED column
 * @param new_right   The new number from 0-0 to display on the right LED column
 */
void update_animation(uint8_t left,     uint8_t middle,     uint8_t right, 
                     uint8_t new_left, uint8_t new_middle, uint8_t new_right) {
  while(left != new_left || middle != new_middle || right != new_right) {
    if(left != new_left) {
      left = (left + 11) % 12;
    }
    if(middle != new_middle) {
      middle = (middle + 5) % 6;
    }
    if(right != new_right) {
      right = (right + 9) % 10;
    }
    set_LEDs(left, middle, right);
    delay(ANIMATION_DELAY);
  }
}

/**
 * @brief Set the LEDs to display the given numbers
 * 
 * Set each column's LEDs to be FIFO representation of the three numbers.
 * 
 * @param left    The number from 0-11 to display on the left LED column
 * @param middle  The number from 0-5 to display on the middle LED column
 * @param right   The number from 0-9 to display on the right LED column
 */
void set_LEDs(uint8_t left, uint8_t middle, uint8_t right) {
  set_column(0, 5, right);
  set_column(5, 3, middle);
  set_column(8, 6, left);
  strip.show();
}

/**
 * @brief Set the LEDs in a sigle column to display a number
 * 
 * Converts the given number into its FIFO representation and updates the strip object.
 * 
 * @param offset  Index of the first LED in the column
 * @param size    Total number of LEDs in the column
 * @param num     Number to display in the column, cannot exceed 2*size-1
 */
void set_column(uint8_t offset, uint8_t size, uint8_t num) {
  int first = (num <= size) ? 0 : num - size;
  int last = (num <= size) ? num : size;
  
  for(uint8_t i = 0; i < size; i++) {
    if(i >= first && i < last) {
      strip.setPixelColor(i + offset, foregroundColor);
    } else {
      strip.setPixelColor(i + offset, backgroundColor);
    }
  }
}
