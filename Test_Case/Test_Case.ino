// Test program for the BT modules
//foo

#include <SoftwareSerial.h>

//Define some pin numbers
#define BT_SERIAL_RX 8
#define BT_SERIAL_TX 9
#define LED_PIN 5
#define BUTTON_PIN 4

//Define some baudrates
#define DEFAULT_USB_BAUD 9600
#define DEFAULT_HC_05_BAUD 9600
#define AT_HC_05_BAUD 38400

SoftwareSerial btSerial(BT_SERIAL_RX, BT_SERIAL_TX);

bool atMode = false;
unsigned long lastSend = 0;

void checkAT()
{
  btSerial.println("AT");

  delay(30);

  Serial.print("Returned from module in checkAT: ");

  String returned("");

  while(btSerial.available() > 0)
  {
    returned += (char)btSerial.read();
  }

  Serial.print(returned);

  if (returned == "OK\r\n")
  {
    atMode = true;
    Serial.println("atMode set to true!");
    return;
  }

  Serial.println("atMode not set!");
  return;
}

void setup()
{
  Serial.begin(DEFAULT_USB_BAUD);
  btSerial.begin(38400);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  checkAT();

  if (!atMode)
  {
    btSerial.begin(9600);
  }
}

void loop()
{
  if (atMode)
  {
    while(btSerial.available() > 0)
    {
      Serial.print((char)btSerial.read());
    }
  
    while(Serial.available() > 0)
    {
      btSerial.print((char)Serial.read());
    }
  }

  if (!atMode)
  {
    if (btSerial.available() > 0)
    {
      byte readChar = btSerial.read();
      if (readChar == 'A')
      {
        digitalWrite(LED_PIN, HIGH);
      }
      if (readChar == 'B')
      {
        digitalWrite(LED_PIN, LOW);
      }
    }
    if (lastSend + 50 <= millis())
    {
      if (digitalRead(BUTTON_PIN) == LOW)
      {
        btSerial.print('A');
      }
      if (digitalRead(BUTTON_PIN) == HIGH)
      {
        btSerial.print('B');
      }
      lastSend = millis();
    }
  }
}
