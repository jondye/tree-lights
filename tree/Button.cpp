#include "Button.h"
#include "Arduino.h"

Button::Button(int pin, long longPress, long debounceDelay)
  : pin(pin)
  , longPress(longPress)
  , debounceDelay(debounceDelay)
  , lastState()
  , changeTime()
  , wasPressed(false)
{
}

void Button::setup()
{
  pinMode(pin, INPUT_PULLUP);
  changeTime = millis();
  lastState = digitalRead(pin);
}

bool Button::pressed()
{
  if (wasPressed) {
    wasPressed = false;
    return true;
  }
  return false;
}

bool Button::held()
{
  return
    (lastState == LOW)
    && ((millis() - changeTime) >= longPress);
}

void Button::read()
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


long Button::elapsed()
{
  return millis() - changeTime;
}

