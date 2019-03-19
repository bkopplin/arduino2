#include <Adafruit_NeoPixel.h>
#define PIN 13
#define NUM_PIXELS 12
Adafruit_NeoPixel strip(NUM_PIXELS, PIN, NEO_GRB);


void setup() {
  // put your setup code here, to run once:
  strip.begin();
  strip.setBrightness(64);
  strip.show();
}

void loop() {
  // put your main code here, to run repeatedly:
  strip.setPixelColor(2,255,0,0);
  for (int i = 0; i < 255; i++) {
    for (int pixel = 0; pixel < NUM_PIXELS; pixel++) {
    strip.setPixelColor(pixel, wheel(i));
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
