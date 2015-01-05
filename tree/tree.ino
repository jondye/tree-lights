#include <Adafruit_NeoPixel.h>
#include <Time.h>
#include <Wire.h>
#include <DS3232RTC.h>

#define PIXEL_PIN   13    // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 22
#define BUTTON_PIN  3

class Button
{
private:
  const int pin;
  const long longPress;
  const long debounceDelay;
  int lastState;
  long changeTime;
  bool wasPressed;
  
  long elapsed()
  {
    return millis() - changeTime;
  }
  
public:
  Button(int pin, long longPress = 500, long debounceDelay = 50)
    : pin(pin)
    , longPress(longPress)
    , debounceDelay(debounceDelay)
    , lastState()
    , changeTime()
    , wasPressed(false)
  {
  }
  
  void setup()
  {
    pinMode(pin, INPUT_PULLUP);
    changeTime = millis();
    lastState = digitalRead(pin);
  }
  
  bool pressed()
  {
    if (wasPressed) {
      wasPressed = false;
      return true;
    }
    return false;
  }
  
  bool held()
  {
    return
      (lastState == LOW)
      && ((millis() - changeTime) >= longPress);
  }
  
  void read()
  {
    const int state = digitalRead(pin);
    const long now = millis();
    if (state != lastState) {
      const long elapsedTime = now - changeTime;
      wasPressed =
        (lastState == LOW)
        && (elapsedTime > debounceDelay)
        && (elapsedTime < longPress);
      changeTime = now;
      lastState = state;
    }
  }
};

class Nightlight
{
private:
  byte brightness;
  int8_t brightnessDelta;
  
public:
  Nightlight()
    : brightness(10)
    , brightnessDelta(1)
  {
  }
  
  void held()
  {
    if (brightnessDelta > 0 && brightness == 255) {
      brightnessDelta = -1;
    } else if (brightnessDelta < 0 && brightness == 0) {
      brightnessDelta = 1;
    }
    brightness += brightnessDelta;
    
    Serial.print("Brightness ");
    Serial.println(brightness);
  }
  
  void released()
  {
    brightnessDelta = -brightnessDelta;
  }
  
  void reset()
  {
    brightness = 0;
    brightnessDelta = 1;
  }
  
  uint32_t colour()
  {
    if (timeStatus() == timeSet) {
      const time_t t = now();
      if (hour(t) >= 6 && hour(t) < 18) {
        return Adafruit_NeoPixel::Color(brightness, brightness/2, 0);
      }
    }
    
    return Adafruit_NeoPixel::Color(brightness, 0, brightness);
  }
};

Adafruit_NeoPixel pixels(PIXEL_COUNT, PIXEL_PIN); 
Button button(BUTTON_PIN);
Nightlight nightlight;

void setup()
{
  pixels.begin();
  pixels.show();
  button.setup();
  Serial.begin(9600);
  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with RTC");
  } else {
    Serial.println("RTC has set the system time");
  }
}

void loop()
{
  static uint32_t lastColour = 0;
  static byte on = false;
  static byte lastHeld = false;
  
  button.read();
  
  if (button.pressed()) {
    on = !on;
    if (on) {
      Serial.println("Turning on");
    } else {
      Serial.println("Turning off");
      pixels.clear();
      pixels.show();
    }
  }
  
  const byte held = button.held();
  
  if (held) {
    if (on) {
      nightlight.held();
    } else {
      on = true;
      nightlight.reset();
    }
  }
  
  if (held != lastHeld) {
    lastHeld = held;
    if (!held) {
      nightlight.released();
    }
  }
  
  const uint32_t colour = nightlight.colour();
  if (lastColour != colour) {
    for (int i = 0; i != pixels.numPixels(); ++i) {
        pixels.setPixelColor(i, colour);
    }
    pixels.show();
    lastColour = colour;
  }
  
  delay(10);
}

