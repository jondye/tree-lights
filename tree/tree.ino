#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN   13    // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 22

#define BUTTON_PIN  3

static const uint32_t WHITE = Adafruit_NeoPixel::Color(255, 255, 255);
static const uint32_t OFF = Adafruit_NeoPixel::Color(0, 0, 0);

class Tree
{
private:
  Adafruit_NeoPixel pixels;
  uint32_t target;
  uint8_t steps;
  
public:
  Tree(uint16_t count, uint8_t pin)
    : pixels(count, pin)
    , target(Adafruit_NeoPixel::Color(0, 0, 0))
    , steps(50)
  {
  }
  
  void setup()
  {
    pixels.begin();
    pixels.show();
  }
  
  void step()
  { 
    pixels.show();
  }
};

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

Adafruit_NeoPixel pixels(PIXEL_COUNT, PIXEL_PIN); 
Button button(BUTTON_PIN);
byte brightness = 10;
int brightnessDelta = 1;
byte on = false;
byte lastHeld = false;

void setup()
{
  pixels.begin();
  pixels.show();
  button.setup();
  Serial.begin(9600);
  Serial.println("Hello");
}

void loop()
{
  button.read();
  
  if (button.pressed()) {
    on = !on;
    if (on) {
      Serial.println("Turning on");
      for (int i = 0; i != pixels.numPixels(); ++i) {
        pixels.setPixelColor(i, brightness, brightness/2, 0);
      }
    } else {
      Serial.println("Turning off");
      pixels.clear();
    }
    pixels.show();
  }
  
  const byte held = button.held();
  
  if (held) {
    Serial.println(brightnessDelta);
    if (on) {
      if (brightnessDelta > 0 && brightness == 255) {
        brightnessDelta = -1;
      } else if (brightnessDelta < 0 && brightness == 0) {
        brightnessDelta = 1;
      }
      brightness += brightnessDelta; 
    } else {
      on = true;
      brightness = 0;
      brightnessDelta = 1;
    }
    Serial.print("Brightness ");
    Serial.println(brightness);
    for (int i = 0; i != pixels.numPixels(); ++i) {
      pixels.setPixelColor(i, brightness, brightness/2, 0);
    }
    pixels.show();
  }
  
  if (held != lastHeld) {
    lastHeld = held;
    if (!held) {
      brightnessDelta = -brightnessDelta;
    }
  }
  
  delay(10);
}

