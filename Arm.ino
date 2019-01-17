#include <SoftwareSerial.h>
#include <Servo.h>

#define PACKET_SIZE 8

#define AT_PIN 3
#define HC_POWER_PIN 4

Servo baseRotate;
Servo elbow1;

int elbow1pos = 0;
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
  if (at) Serial1.begin(38400);
  else Serial1.begin(9600);
}

int recvInt(Stream* stream)
{
  int r = 0;
  void* a = &r;
  byte* b = a;
  stream->readBytes(b, 2);
  return r;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(74880);
  if (atMode) Serial1.begin(38400);
  else Serial1.begin(9600);
  baseRotate.attach(6);
  elbow1.attach(7);
  pinMode(HC_POWER_PIN, OUTPUT);
  pinMode(AT_PIN, OUTPUT);
  digitalWrite(HC_POWER_PIN, HIGH);
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
        Serial1.write('A');
        Serial1.write('T');
        while (Serial.available() > 0)
        {
          Serial1.write(Serial.read());
        }
        Serial1.write(0x0A);
        Serial1.write(0x0D);
      }
      else if (b1 == 'R' && b2 == 'E' && (Serial.available() > 1) && Serial.read() == 'B' && Serial.read() == 'T')
      {
        hcReboot();
        Serial.println("Rebooted back to Bluetooth mode! Flushing buffer!");
        while (Serial.available() > 0)
        {
          Serial.read();
        }
        return;
      }
      else
      {
        Serial.println("Invalid command");
        //Serial.println("Enter a command, or enter REBT reboot back into Bluetooth mode");
      }
    }

    if (Serial1.available() > 0)
    {
      delay(200);
      Serial.println("Response from the HC module:");
      while (Serial1.available() > 0)
      {
        Serial.write(Serial1.read());
      }
    }
  }
  
  else
  {
    if (Serial1.available() >= PACKET_SIZE)
    {
      Serial.println(Serial1.available());
      byte header[4];
      Serial1.readBytes(header, 4);
      bool correct = true;
      for(byte* i = header; i < header + 4; i++)
      {
        Serial.print("Header byte: ");
        Serial.println(*i);
        if (*i != 0xF0)
        {
          correct = false;
        }
      }
      if (!correct)
      {
        Serial.println("Header check failed! Flushing buffer...");
        while (Serial1.available() > 0)
        {
          Serial1.read();
        }
        return;
      }
      Serial.println("Header check passed!");
  
      int toWrite1 = recvInt(&Serial1);
  
      if (toWrite1 >= 95 and toWrite1 <= 105)
        toWrite1 = 100;
      else if (toWrite1 < 95)
        toWrite1 += 5;
      else 
        toWrite1 -= 5;
  
      Serial.print("Writing to continuous servo: ");
      Serial.println(toWrite1);
      baseRotate.write(toWrite1);
  
      int toIncrease1 = map(recvInt(&Serial1), 0, 1024, -10, 10);
  
      if (abs(toIncrease1) < 3)
      {
        toIncrease1 = 0;
      }
  
      Serial.print("Increasing elbow 1 by: ");
      Serial.println(toIncrease1);
  
      elbow1pos = constrain(elbow1pos + toIncrease1, 0, 170);
      Serial.print("Writing to elbow 1: ");
      Serial.println(elbow1pos);
      elbow1.write(elbow1pos);
    }

    if (Serial.available() > 1)
    {
      if (Serial.read() == 'A' && Serial.read() == 'T')
      {
        hcReboot(true);
        Serial.println("Entered AT command mode! Flushing buffer!");
        while (Serial.available() > 0)
        {
          Serial.read();
        }
        Serial.println("Enter a command, or enter REBT reboot back into Bluetooth mode");
      }
    }
  }
}
