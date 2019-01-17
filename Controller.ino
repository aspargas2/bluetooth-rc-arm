
#include <SoftwareSerial.h>

#define AT_PIN 3
#define HC_POWER_PIN 4

SoftwareSerial btSerial(9, 10);

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
  if (at) btSerial.begin(38400);
  else btSerial.begin(9600);
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(74880);
  if (atMode) btSerial.begin(38400);
  else btSerial.begin(9600);
  //pinMode(A2, INPUT);
  //pinMode(A3, INPUT);
  pinMode(6, INPUT_PULLUP);
  pinMode(HC_POWER_PIN, OUTPUT);
  pinMode(AT_PIN, OUTPUT);
  digitalWrite(HC_POWER_PIN, HIGH);
}

void loop() {
  
  if (atMode)
  {
    if (Serial.available() > 1)
    {
      delay(200);
      byte b1 = Serial.read();
      byte b2 = Serial.read();
      if (b1 == 'A' && b2 == 'T')
      {
        btSerial.write('A');
        btSerial.write('T');
        while (Serial.available() > 0)
        {
          btSerial.write(Serial.read());
        }
        btSerial.write(0x0A);
        btSerial.write(0x0D);
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

    if (btSerial.available() > 0)
    {
      delay(200);
      Serial.println("Response from the HC module:");
      while (btSerial.available() > 0)
      {
        Serial.write(btSerial.read());
      }
    }
  }

  else
  {
    for (int i = 0; i < 4; i++)
    {
      Serial.println("Header byte sent!");
      btSerial.write((byte)0xF0);
    }
  
    if (digitalRead(6) == LOW)
    {
      Serial.println("Calibrating... ");
      offset2 = 512 - analogRead(A2);
      offset3 = 512 - analogRead(A3);
    }
    int readFrom2 = analogRead(A2) + offset2;
    int readFrom3 = analogRead(A3) + offset3;
    
    Serial.println(analogToServo(readFrom2));
    sendInt(analogToServo(readFrom2), &btSerial);
    Serial.println(readFrom3);
    sendInt(readFrom3, &btSerial);
    delay(50);

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
