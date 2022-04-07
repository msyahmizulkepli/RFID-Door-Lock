#define SS_PIN 4     //D2
#define RST_PIN 5    //D1
#define BUZZER_PIN 9 //S2
#define LOCK_PIN 10  //S3
#define SWITCH_PIN 3 //RX
#define SENSOR_PIN 1 //TX

#include <SPI.h>
#include <MFRC522.h>

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
int status = LOW;

void setup()
{
  Serial.begin(9600); // Initiate a serial communication
  SPI.begin();        // Initiate  SPI bus
  mfrc522.PCD_Init(); // Initiate MFRC522
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  pinMode(SENSOR_PIN, INPUT);
}
void loop()
{
  status = digitalRead(SWITCH_PIN);
  if (status == HIGH)
  {
    Access();
  }
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Show UID on serial monitor
  Serial.println();
  Serial.print(" UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();
  if (content.substring(1) == "A9 17 D5 6F") //change UID of the card that you want to give access
  {
    Access();
  }

  else
  {
    NoAccess();
  }
}

void Access()
{
  Serial.println(" Access Granted ");
  Serial.println();
  Serial.println("Unlocking Door...");
  //1 long beep
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
  //Unlock door
  digitalWrite(LOCK_PIN, HIGH);
  delay(5000);
  Serial.println("Locking Door...");
  digitalWrite(LOCK_PIN, LOW);
  status = LOW;
}

void NoAccess()
{
  Serial.println(" Access Denied ");
  //3 short beeps
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}
