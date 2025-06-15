#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN D4  //--> SDA / SS is connected to pinout D4
#define RST_PIN D3  //--> RST is connected to pinout D3

MFRC522 mfrc522(SS_PIN, RST_PIN);  //--> Create MFRC522 instance.
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo lock_servo;

byte readcard[4];
char str[32] = "";
String StrUID;
String whoOpened;

String get_uid();
void array_to_string(byte array[], unsigned int len, char buffer[]);
void printToLcd(String text, int row, bool clear);
void toggleLock();


void setup() {
  //For servo
  lock_servo.attach(D8);

  //For RFID reader
  SPI.begin();      
  mfrc522.PCD_Init(); 

  //For LCD
  lcd.init();
  lcd.backlight();

  //For Serial Dedugging
  Serial.begin(9600);

}

void loop() {
  String recievedUID = get_uid();

  if (recievedUID == "") {
    return;
  }else if (whoOpened == "") {
    whoOpened = recievedUID;
    printToLcd(recievedUID, 1, true);
    toggleLock();
  }else {
    if(whoOpened == recievedUID){
      printToLcd(recievedUID, 1, true);
      toggleLock();
      whoOpened = "";
    }
    else{
      printToLcd("Access Denied", 0, true);
      printToLcd(recievedUID, 1, false);
      delay(1000);

      printToLcd("Can't be Locked", 0, true);
    }
  }

  delay(500);
}

String get_uid() {  
  if(!mfrc522.PICC_IsNewCardPresent()) {
    return "";
  }
  if(!mfrc522.PICC_ReadCardSerial()) {
    return "";
  }
  
  for(int i=0;i<4;i++){
    readcard[i]=mfrc522.uid.uidByte[i]; //storing the UID of the tag in readcard
    array_to_string(readcard, 4, str);
    StrUID = str;
  }
  Serial.println(StrUID);

  mfrc522.PICC_HaltA();
  return StrUID;
}

void array_to_string(byte array[], unsigned int len, char buffer[]) {
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

void printToLcd(String text, int row, bool clear) {
  if (clear) {
    lcd.clear();
  }
  lcd.setCursor(0, row);
  lcd.print(text);
}

void toggleLock() {
  static bool isLocked = true;
  if (isLocked) {
    lock_servo.write(180); // Unlock
    printToLcd("Unlocked", 0, false);
  } else {
    lock_servo.write(70); // Lock
    printToLcd("Locked", 0, false);
  }
  isLocked = !isLocked;
}
