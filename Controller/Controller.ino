/*

This sketch has only been tested on an Ardino Uno, but it should work fine
on any Arduino-type board that has the appropriate amount of digital and analog pins.

This sketch uses an HC-05 Bluetooth module on hcSerial to send joystick positions to the board running Arm.ino.

*/

#include <SoftwareSerial.h>

//Digital pin definitions
#define AT_PIN 3
#define HC_POWER_PIN 4
#define CALIBRATION_BUTTON_PIN 5
#define SLOT1_PIN 6
#define SLOT2_PIN 7
#define SLOT3_PIN 8
#define STOP_PIN 11
#define RECORD_PIN 12
#define SINGLE_PLAYBACK_PIN A4
#define LOOP_PLAYBACK_PIN A5

//Analog pin definitions
#define JOY1X_PIN A0
#define JOY1Y_PIN A1
#define JOY2X_PIN A2
#define JOY2Y_PIN A3

SoftwareSerial hcSerial(9, 10);
int offset1 = 0;
int offset2 = 0;
int offset3 = 0;
int offset4 = 0;
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

//Converts the analog reading of a joystick to the appropriate value by which to change the value being written to a 180 degree servo
int analogTo180Servo(int analog)
{
  return map(analog, 0, 1024, -20, 20);
}

void setup()
{
  Serial.begin(74880); //Debug output on USB
  pinMode(CALIBRATION_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SLOT1_PIN, INPUT_PULLUP);
  pinMode(SLOT2_PIN, INPUT_PULLUP);
  pinMode(SLOT3_PIN, INPUT_PULLUP);
  pinMode(STOP_PIN, INPUT_PULLUP);
  pinMode(RECORD_PIN, INPUT_PULLUP);
  pinMode(LOOP_PLAYBACK_PIN, INPUT_PULLUP);
  pinMode(SINGLE_PLAYBACK_PIN, INPUT_PULLUP);
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
      offset1 = 512 - analogRead(JOY1X_PIN);
      offset2 = 512 - analogRead(JOY1Y_PIN);
      offset3 = 512 - analogRead(JOY2X_PIN);
      offset4 = 512 - analogRead(JOY2Y_PIN);
    }
    int readFrom1 = analogRead(JOY1X_PIN) + offset1;
    int readFrom2 = analogRead(JOY1Y_PIN) + offset2;
    int readFrom3 = analogRead(JOY2X_PIN) + offset3;
    int readFrom4 = analogRead(JOY2Y_PIN) + offset4;
    
    Serial.println(readFrom1);
    sendInt(analogTo180Servo(readFrom1), &hcSerial);
    Serial.println(readFrom2);
    sendInt(analogTo180Servo(readFrom2), &hcSerial);
    Serial.println(readFrom3);
    sendInt(analogTo180Servo(readFrom3), &hcSerial);
    Serial.println(readFrom4);
    sendInt(analogTo180Servo(readFrom4), &hcSerial);
    delay(200);

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
        Serial.println("Enter a command, or enter REBT to reboot back into Bluetooth mode");
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
        {
          hcSerial.read();
        }
      }
    }
  }
}
