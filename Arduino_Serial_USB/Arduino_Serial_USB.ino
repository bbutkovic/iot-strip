/*  RGB Lamp code
 *  Uses the Blynk app :)
 *  github.com/bbutkovic
 */

#include <SoftwareSerial.h>
#include <BlynkSimpleStream.h>
#include <Adafruit_NeoPixel.h>

#define BLYNK_PRINT DebugSerial

#define MAX_GRAVITY 11
#define MAX_COLOR 255

#define STRIP_PIN  6
#define STRIP_LEDS 30

#define STATE_OFF 0
#define STATE_ON  1

#define MODE_COLOR 3
#define MODE_WAND  2
#define MODE_CYCLE 1

#define CYCLE_OFFSET  1000
#define CYCLE_INCREMENT 40
#define CYCLE_DELAY   25
#define CYCLE_MAX_HUE 65535

SoftwareSerial DebugSerial(2, 3); // RX, TX
char auth[] = "MyAuthKey";
int brightness = 255;
int state   = STATE_OFF;
int mode    = MODE_CYCLE;
int color_r = 0;
int color_g = 0;
int color_b = 0;
long cycle_hue = 0;
unsigned long cycle_next = 0;

Adafruit_NeoPixel strip(STRIP_LEDS, STRIP_PIN, NEO_GRB + NEO_KHZ800);

// ON/OFF button
BLYNK_WRITE(V0) {
  if (param.asInt() == 1) {
    state = STATE_ON;
  } else {
    state = STATE_OFF;
  }
}

// Read brightness
BLYNK_WRITE(V1) {
  brightness = param.asInt();
}

// Read accelometer
BLYNK_WRITE(V20) {
  if (mode == MODE_WAND) {
    color_r = map(param[0].asFloat(), 0, MAX_GRAVITY, 0, MAX_COLOR);
    color_g = map(param[1].asFloat(), 0, MAX_GRAVITY, 0, MAX_COLOR);
    color_b = map(param[2].asFloat(), 0, MAX_GRAVITY, 0, MAX_COLOR);
  }
}

// Read RGB
BLYNK_WRITE(V2) {
  if (mode == MODE_COLOR) {
    color_r = param[0].asInt();
    color_g = param[1].asInt();
    color_b = param[2].asInt();
  }
}

// Check mode
BLYNK_WRITE(V5) {
  mode = param.asInt();
}

/* STRIP MODES */
void runModeColor() {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(color_r, color_g, color_b));
  }
}

void runModeCycle() {
  if (millis() < cycle_next) {
    return;
  }

  cycle_next = millis() + CYCLE_DELAY;

  cycle_hue += CYCLE_INCREMENT;
  int current_cycle_hue = cycle_hue;
  for (int i = 0; i < strip.numPixels(); i++) {
    current_cycle_hue = map(current_cycle_hue + CYCLE_OFFSET, 0, CYCLE_MAX_HUE, 0, CYCLE_MAX_HUE);
    
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(current_cycle_hue)));
  }
}


void setup()
{
  // Debug console
  DebugSerial.begin(9600);

  // Blynk will work through Serial
  // Do not read or write this serial manually in your sketch
  Serial.begin(9600);
  Blynk.begin(Serial, auth);

  strip.begin();
  strip.show();
  strip.setBrightness(0);
}

void loop()
{
  Blynk.run();

  if (state == STATE_ON) {
    strip.setBrightness(brightness);
  } else {
    strip.setBrightness(0);
  }
  
  switch(mode) {
    case MODE_CYCLE:
      runModeCycle();
      break;
    case MODE_COLOR:
    case MODE_WAND:
      runModeColor();
      break;    
  }
  
  strip.show();
}
