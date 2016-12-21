#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include <Time.h>
#include <Wire.h>
#include <DS3232RTC.h>
#include "Button.h"

#define PIXEL_PIN   5    // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 22
#define BUTTON_PIN  4

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
      if (hour(t) >= 6 && hour(t) < 19) {
        return Adafruit_NeoPixel::Color(brightness, brightness/2, 0);
      }
    }

    return Adafruit_NeoPixel::Color(brightness, 0, brightness);
  }
};

time_t readDateTime()
{
  tmElements_t tm;
  tm.Year = CalendarYrToTm(Serial.parseInt());
  tm.Month = Serial.parseInt();
  tm.Day = Serial.parseInt();
  tm.Hour = Serial.parseInt();
  tm.Minute = Serial.parseInt();
  tm.Second = Serial.parseInt();
  return makeTime(tm);
}

void print_2digit(Print &out, int value)
{
  if (value < 10) {
    out.print('0');
  }
  out.print(value);
}

void print_date(Print &out, time_t t)
{
  out.print(year(t));
  out.print('-');
  print_2digit(Serial, month(t));
  out.print('-');
  print_2digit(Serial, day(t));
  out.print('T');
  print_2digit(Serial, hour(t));
  out.print(':');
  print_2digit(Serial, minute(t));
  out.print(':');
  print_2digit(Serial, second(t));
}

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
    Serial.print("RTC has set the system time to ");
    print_date(Serial, now());
    Serial.println();
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

  if (Serial.available() >= 14) {
    time_t t = readDateTime();
    RTC.set(t);
    setTime(t);
    Serial.print("Time set to ");
    print_date(Serial, now());
    Serial.println();
    while (Serial.available() > 0) {
      Serial.read(); // dump extra characters
    }
  }

  delay(10);
}
