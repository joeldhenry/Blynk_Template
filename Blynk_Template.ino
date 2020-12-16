#include <BlynkSimpleEsp8266.h>

//logs
bool debug = true;

void setup()
{
  // Debug console
  Serial.begin(9600);
  // starts interactive wifi setup or connects to blynk
  bool result = setupBlynk();

  if (result != true)
  {
    Serial.println("BLYNK Connection Fail");
    resetSettings();
    return;
  }

  Serial.println("BLYNK Connected");

  // do stuff here
}

void loop()
{
  Blynk.run();
  timer.run();
}
