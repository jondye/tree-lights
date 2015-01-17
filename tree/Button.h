#ifndef BUTTON_H
#define BUTTON_H

class Button
{
public:
  Button(int pin, long longPress = 500, long debounceDelay = 50);
  void setup();
  bool pressed();
  bool held();
  void read();

private:
  long elapsed();

  const int pin;
  const long longPress;
  const long debounceDelay;
  int lastState;
  long changeTime;
  bool wasPressed;
};

#endif
