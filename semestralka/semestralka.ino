#include<SPI.h>
#include<MFRC522.h>


// definování pinů pro SDA a RST
#define SDA_PIN   10
#define RST_PIN   9
// definovani pinu pro LED
#define RED_PIN    7
#define GREEN_PIN  6
#define BLUE_PIN   5

MFRC522 rfid(SDA_PIN, RST_PIN);

MFRC522::MIFARE_Key key; 
byte cardCode[4];



void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}


void flashGreen(int miliseconds){
  digitalWrite(GREEN_PIN, HIGH);
  delay(miliseconds);
  digitalWrite(GREEN_PIN, LOW);
}  

void flashRed(int miliseconds){
  digitalWrite(RED_PIN, HIGH);
  delay(miliseconds);
  digitalWrite(RED_PIN, LOW);
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  
  pinMode(RED_PIN, OUTPUT);
}




void loop() {
  
   // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;
  
  flashGreen(250);
  
  Serial.print("PICC type: ");
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check if card is Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println("Your tag is not of type MIFARE Classic.");
    return;
  }
  
  //recognise blue tag
  if(rfid.uid.uidByte[0] == 0xA0 &&
     rfid.uid.uidByte[1] == 0x6B &&
     rfid.uid.uidByte[2] == 0x32 &&
     rfid.uid.uidByte[3] == 0x52)
    Serial.println("BLUE TAG!");

  if (rfid.uid.uidByte[0] != cardCode[0] || 
      rfid.uid.uidByte[1] != cardCode[1] || 
      rfid.uid.uidByte[2] != cardCode[2] || 
      rfid.uid.uidByte[3] != cardCode[3] ) {
    Serial.print("New card. ");

    // Store NUID into cardCode array
    for (byte i = 0; i < 4; i++) {
      cardCode[i] = rfid.uid.uidByte[i];
    }
   
    Serial.print("NUID: ");
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else Serial.println("Card read previously.");
  
  Serial.println();


  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}
 


