/*

This sketch has only is has only been tested on an Ardino Uno and an Arduino Lilypad, but it should work fine
on any Arduino-type board that has the appropriate amount of digital pins.

This sketch uses an HC-05 Bluetooth module on hcSerial to send joystick positions to the board running Arm.ino.

*/

#include <SoftwareSerial.h>

#define AT_PIN 3
#define HC_POWER_PIN 4
#define CALIBRATION_BUTTON_PIN 6

SoftwareSerial hcSerial(9, 10);

int offset2 = 0;
int offset3 = 0;
bool atMode = false;

void hcReboot(bool at = false)
{
  atMode = at;
  digitalWrite(HC_POWER_PIN, LOW);
  if (at) digitalWrite(AT_PIN, HIGH);
  delay(1000);
  digitalWrite(HC_POWER_PIN, HIGH);
  delay(1000);
  digitalWrite(AT_PIN, LOW);
  if (at) hcSerial.begin(38400);
  else hcSerial.begin(9600);
}

//Sends a signed 16-bit integer through a stream
void sendInt(int toSend, Stream* stream)
{
  void* a = &toSend;
  byte* b = a;
  byte c = b[0];
  byte d = b[1];
  stream->write(c);
  stream->write(d);
}


int analogToServo(int analog)
{
  return map(analog, 0, 1024, 80, 120);
}

void setup()
{
  Serial.begin(74880);
  //pinMode(A2, INPUT);
  //pinMode(A3, INPUT);
  pinMode(CALIBRATION_BUTTON_PIN, INPUT_PULLUP);
  pinMode(HC_POWER_PIN, OUTPUT);
  pinMode(AT_PIN, OUTPUT);
  hcReboot(atMode);
}

void loop()
{  
  if (atMode)
  {
    if (Serial.available() > 1)
    {
      delay(200);
      byte b1 = Serial.read();
      byte b2 = Serial.read();
      if (b1 == 'A' && b2 == 'T')
      {
        hcSerial.write('A');
        hcSerial.write('T');
        while (Serial.available() > 0)
        {
          hcSerial.write(Serial.read());
        }
        hcSerial.write(0x0A);
        hcSerial.write(0x0D);
      }
      else if (b1 == 'R' && b2 == 'E' && (Serial.available() > 1) && Serial.read() == 'B' && Serial.read() == 'T')
      {
        hcReboot();
        Serial.println("Rebooted back to Bluetooth mode! Flushing both Serial buffers!");
        while (hcSerial.available() > 0)
        {
          hcSerial.read();
        }
        while (Serial.available() > 0)
        {
          Serial.read();
        }

      }
      else
      {
        Serial.println("Invalid command. Flushing USB Serial.");
        while (Serial.available() > 0)
        {
          Serial.read();
        }
        //Serial.println("Enter a command, or enter REBT reboot back into Bluetooth mode");
      }
    }

    if (hcSerial.available() > 0)
    {
      delay(200);
      Serial.println("Response from the HC module:");
      while (hcSerial.available() > 0)
      {
        Serial.write(hcSerial.read());
      }
    }
  }

  else
  {
    for (int i = 0; i < 4; i++)
    {
      Serial.println("Header byte sent!");
      hcSerial.write((byte)0xF0);
    }
  
    if (digitalRead(CALIBRATION_BUTTON_PIN) == LOW)
    {
      Serial.println("Calibrating... ");
      offset2 = 512 - analogRead(A2);
      offset3 = 512 - analogRead(A3);
    }
    int readFrom2 = analogRead(A2) + offset2;
    int readFrom3 = analogRead(A3) + offset3;
    
    Serial.println(analogToServo(readFrom2));
    sendInt(analogToServo(readFrom2), &hcSerial);
    Serial.println(readFrom3);
    sendInt(readFrom3, &hcSerial);
    delay(50);

    if (Serial.available() > 1)
    {
      if (Serial.read() == 'A' && Serial.read() == 'T')
      {
        hcReboot(true);
        Serial.println("Entered AT command mode! Flushing buffers!");
        while (Serial.available() > 0)
        {
          Serial.read();
        }
        while (hcSerial.available() > 0)
        {
          hcSerial.read();
        }
        Serial.println("Enter a command, or enter REBT reboot back into Bluetooth mode");
      }
      else
      {
        Serial.println("Unrecognized input detected on USB Serial.\n\
                       Send AT to reboot the HC module in to At command mode and enter commands. Flushing USB Serial buffer.");
        while (Serial.available() > 0)
        {
          Serial.read();
        }
        while (hcSerial.available() > 0)
      }
    }
  }
}
