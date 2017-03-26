#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include <DS3232RTC.h>
#include <SerialCommand.h>
#include <Time.h>
#include <Wire.h>

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
      const byte h = hour(t);
      const byte m = minute(t);
      if ((h > 6 && h < 19) || (h == 6 && m >= 30) || (h == 19 && m < 30)) {
        return Adafruit_NeoPixel::Color(brightness, brightness/2, 0);
      }
    }

    return Adafruit_NeoPixel::Color(brightness, 0, brightness);
  }
};

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
  print_2digit(out, month(t));
  out.print('-');
  print_2digit(out, day(t));
  out.print('T');
  print_2digit(out, hour(t));
  out.print(':');
  print_2digit(out, minute(t));
  out.print(':');
  print_2digit(out, second(t));
}

Adafruit_NeoPixel pixels(PIXEL_COUNT, PIXEL_PIN);
Button button(BUTTON_PIN);
Nightlight nightlight;
SerialCommand serialcmd;

void set_time()
{
  int numbers[6] = {};
  for (auto & number : numbers) {
    const char *const arg = serialcmd.next();
    if (arg == nullptr) {
      Serial.println("Invalid time format");
      return;
    }
    number = strtol(arg, NULL, 10);
  }
  tmElements_t tm;
  tm.Year = CalendarYrToTm(numbers[0]);
  tm.Month = numbers[1];
  tm.Day = numbers[2];
  tm.Hour = numbers[3];
  tm.Minute = numbers[4];
  tm.Second = numbers[5];
  const time_t t = makeTime(tm);
  RTC.set(t);
  setTime(t);
  Serial.print("time ");
  print_date(Serial, now());
  Serial.println();
}

void setup()
{
  Serial.begin(9600);
  pixels.begin();
  pixels.show();
  button.setup();
  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with RTC");
  } else {
    Serial.print("RTC has set the system time to ");
    print_date(Serial, now());
    Serial.println();
  }
  serialcmd.addCommand("settime", set_time);
}

void loop()
{
  static uint32_t lastColour = 0;
  static byte on = false;
  static byte lastHeld = false;

  button.read();
  serialcmd.readSerial();

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
    for (unsigned i = 0; i != pixels.numPixels(); ++i) {
        pixels.setPixelColor(i, colour);
    }
    pixels.show();
    lastColour = colour;
  }

  delay(10);
}
