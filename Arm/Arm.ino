/*

This sketch has only is has only been tested on an Ardino Mega, but it should work fine
on any Arduino-type board that has multiple hardware Serial ports and the appropriate amount of digital pins.

This sketch uses an HC-05 Bluetooth module on Serial1 to recieve joystick positions from the board running Controller.ino

*/

#include <Servo.h>

#define PACKET_SIZE 8

//Digital pin definitions
#define AT_PIN 3
#define HC_POWER_PIN 4
#define IR_RECV_PIN 10

//Non-continuous servo constraint definitions
#define ELBOW1_MIN 11
#define ELBOW1_MAX 185
#define ELBOW2_MIN 11
#define ELBOW2_MAX 185
#define CLAW_MIN 11
#define CLAW_MAX 185

Servo baseRotate;
Servo elbow1;
Servo elbow2;
Servo claw;

int elbow1pos = 0;
int elbow2pos = 0;
int clawpos = 0;
int rotations = 0;

bool atMode = false;

void returnToIR()
{
  if (rotations == 1)
  {
    baseRotate.write(90);
    if (digitalRead(IR_RECV_PIN) == LOW) while (digitalRead(IR_RECV_PIN) == LOW);
    while (digitalRead(IR_RECV_PIN) == HIGH);
    while (digitalRead(IR_RECV_PIN) == LOW);
    baseRotate.write(100);
  }
  else if (rotations == -1)
  {
    baseRotate.write(110);
    if (digitalRead(IR_RECV_PIN) == LOW) while (digitalRead(IR_RECV_PIN) == LOW);
    while (digitalRead(IR_RECV_PIN) == HIGH);
    while (digitalRead(IR_RECV_PIN) == LOW);
    baseRotate.write(100);
  }
  else
  {
    baseRotate.write(110);
    while (digitalRead(IR_RECV_PIN) == LOW);
    baseRotate.write(100);
  }
}

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
  elbow2.attach(8);
  claw.attach(9);
  pinMode(HC_POWER_PIN, OUTPUT);
  pinMode(AT_PIN, OUTPUT);  
  pinMode(IR_RECV_PIN, INPUT);
  digitalWrite(HC_POWER_PIN, HIGH);
  returnToIR();
  pinMode(34, INPUT_PULLUP); //temp thing for testing
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
        Serial.println("Rebooted back to Bluetooth mode! Flushing buffers!");
        while (Serial.available() > 0)
        {
          Serial.read();
        }
        while (Serial1.available() > 0)
        {
          Serial1.read();
        }
        return;
      }
      else
      {
        Serial.println("Invalid command. Flushing buffer.");
        while (Serial1.available() > 0)
        {
          Serial1.read();
        }
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
    if(digitalRead(34) == LOW) //temp testing
      returnToIR();
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
  
      int toWrite1 = (int)Serial1.read();
  
      if (toWrite1 >= 95 and toWrite1 <= 105)
        toWrite1 = 100;
      else if (toWrite1 < 95)
        toWrite1 += 5;
      else 
        toWrite1 -= 5;
      
      
      
      Serial.print("Writing to continuous servo: ");
      Serial.println(toWrite1);
      baseRotate.write(toWrite1);
  
      int toIncrease1 = Serial1.read();
      int toIncrease2 = Serial1.read();
      int toIncrease3 = Serial1.read();
      
      if (abs(toIncrease1) < 3) toIncrease1 = 0;
      if (abs(toIncrease2) < 3) toIncrease2 = 0;
      if (abs(toIncrease3) < 5) toIncrease3 = 0;
  
      Serial.print("Increasing elbow 1 by: ");
      Serial.println(toIncrease1);
      Serial.print("Increasing elbow 2 by: ");
      Serial.println(toIncrease2);
      Serial.print("Increasing claw by: ");
      Serial.println(toIncrease3);
  
      elbow1pos = constrain(elbow1pos + toIncrease1, ELBOW1_MIN, ELBOW1_MAX);
      elbow2pos = constrain(elbow1pos + toIncrease1, ELBOW1_MIN, ELBOW1_MAX);
      clawpos = constrain(elbow1pos + toIncrease1, ELBOW1_MIN, ELBOW1_MAX);
      
      Serial.print("Writing to elbow 1: ");
      Serial.println(elbow1pos);
      elbow1.write(elbow1pos);
      Serial.print("Writing to elbow 2: ");
      Serial.println(elbow2pos);
      elbow2.write(elbow2pos);
      Serial.print("Writing to claw: ");
      Serial.println(clawpos);
      claw.write(clawpos);
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
        while (Serial1.available() > 0)
        {
          Serial1.read();
        }
        Serial.println("Enter a command, or enter REBT reboot back into Bluetooth mode");
      }
    }
  }
}
