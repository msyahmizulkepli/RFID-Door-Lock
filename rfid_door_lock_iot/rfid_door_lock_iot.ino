//####################### Setting pins ##################################
#define SS_PIN 4  //D2
#define RST_PIN 5 //D1
#define BUZZER_PIN 9 //S2
#define LOCK_PIN 10 //S3
#define SWITCH_PIN 3 //RX
#define SENSOR_PIN 1 //TX
//####################### Include Libraries #############################
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "REPLACE_WITH_YOUR_SSID"
#define WIFI_PASSWORD "REPLACE_WITH_YOUR_PASSWORD"
#define API_KEY "REPLACE_WITH_YOUR_FIREBASE_PROJECT_API_KEY"
#define DATABASE_URL "REPLACE_WITH_YOUR_FIREBASE_DATABASE_URL"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const int MAXNOPEOPLE = 30;// set max no of people in database to 30
unsigned long senDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
String id[MAXNOPEOPLE];
const String Path = "/id/";
const String MasterCard = "A9 17 D5 6F";


MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
int status = LOW;


void setup()
{
  Serial.begin(9600); // Initiate a serial communication
  SPI.begin();        // Initiate  SPI bus
  mfrc522.PCD_Init(); // Initiate MFRC522
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  pinMode(SENSOR_PIN, INPUT);

  for(int i=0; i<MAXNOPEOPLE ; i++){
    if (Firebase.RTDB.getString(&fbdo, Path + i)){
      Serial.println(fbdo.stringData());
      id[i] = fbdo.stringData();
      Serial.println(id[i]);
    }
    else{
      Serial.println(fbdo.errorReason());
    }
  }
  
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
  for (int i = 0; i < MAXNOPEOPLE; i++) {
    if (content.substring(1) == id[i] || content.substring(1) == MasterCard)
    {
      Access();
    }
    else
    {
      NoAccess();
    }
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
