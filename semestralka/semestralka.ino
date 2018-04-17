#include<SPI.h>
#include<MFRC522.h>


/*
 *	Defines pins used to communicated with RFID reader.
 */
#define SDA_PIN   10
#define RST_PIN   9

/*
 *	Defines pins used to control LED.
 */
#define RED_PIN    7
#define GREEN_PIN  6
#define BLUE_PIN   5

/*
 *	Specifies how long will the lock stay unlocked after successful card authentifcation (in seconds).
 */
#define DOOR_UNLOCK_DURATION 3

/*
 *	Specifies how long will green LED flash after new card has been added into system (in miliseconds).
 */
#define CARD_ADD_LED_FLASH_DURATION 1000

/*
 *	Specifies how long will green LED flash after card has been removed from system (in miliseconds).
 */
#define CARD_DEL_LED_FLASH_DURATION 1000

/*
 *	Specifies how long will blue LED flash when ready to read card to be added or deleted (in miliseconds).
 */
#define CARD_ADD_DEL_READY_SIGNAL_DURATION 500





/*
 *	Structure to store RFID card ID. 
 */
typedef struct RfidCard {
  byte id[4];
} card;





/*
 *	RFID instance used to communicate with RFID reader.
 */
MFRC522 rfid(SDA_PIN, RST_PIN);

/*
 *	
 */
card addCard;
card deleteCard;
card userCard;





/*
 *	Reads RFID card ID from connected RFID reader if there's card present near RFID reader.
 *	Input/output parameter c will not be modified if there's no card or the card is of incompatible type.
 *	@param c - card instance to write info from RFID card into
 *	@return - indicates whether read has been successful
 */
bool readCard(card* c)
{
  if ( ! rfid.PICC_IsNewCardPresent() )
	return false;

  if ( ! rfid.PICC_ReadCardSerial() )
	return false;
  
  MFRC522::PICC_Type type = rfid.PICC_GetType(rfid.uid.sak);

  //check if card is supported type
  if ( type != MFRC522::PICC_TYPE_MIFARE_MINI &&  
	   type != MFRC522::PICC_TYPE_MIFARE_1K &&
	   type != MFRC522::PICC_TYPE_MIFARE_4K ) {
	return false;
  }

  for(int i = 0; i < 4; ++i)
  	c->id[i] = rfid.uid.uidByte[i];

  //close RFID reader resources
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  return true;
}



/*
 *	Checks whether card ID is stored in database.
 *	@param - card to be checked
 *	@return - true if card is in database, false otherwise
 */
bool checkCard(card* c)
{

}



/*
 *	Adds card to database.
 *	@param - card to be added
 */
void addCard(card* c)
{

}



/*
 *	Deletes card from database.
 *	@param - card to be deleted
 */
void deleteCard(card* c)
{

}



/*
 *	Unlocks the electromagnetic lock for specified amount of time and locks it afterwards. 
 *	Turns the green LED on while the lock is unlocked.
 */
void unlockDoor(int miliseconds)
{
	//lock.unlock();
	flashGreen(miliseconds);
	//lock.lock();
}



/*
 *	Flashes red LED multiple times to signalize invalid card.
 */
void accessDenied()
{
	for(int i = 0; i < 3; ++i)
		flashRed(100);
}



void printHex(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
	Serial.print(buffer[i] < 0x10 ? " 0" : " ");
	Serial.print(buffer[i], HEX);
  }
}


/*
 *	Turns on green LED for specified amount of miliseconds.
 *	@param - amount of miliseconds during which will LED be on
 */
void flashGreen(int miliseconds)
{
  digitalWrite(GREEN_PIN, HIGH);
  delay(miliseconds);
  digitalWrite(GREEN_PIN, LOW);
}  



/*
 *	Turns on red LED for specified amount of miliseconds.
 *	@param - amount of miliseconds during which will LED be on
 */
void flashRed(int miliseconds)
{
  digitalWrite(RED_PIN, HIGH);
  delay(miliseconds);
  digitalWrite(RED_PIN, LOW);
}



/*
 *	Turns on blue LED for specified amount of miliseconds.
 *	@param - amount of miliseconds during which will LED be on
 */
void flashBlue(int miliseconds)
{
  digitalWrite(BLUE_PIN, HIGH);
  delay(miliseconds);
  digitalWrite(BLUE_PIN, LOW);
}





void setup() 
{
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}



void loop() 
{

  /*
  priblizna struktura mainu:
  
  if ( readCard(&userCard) ) {
	if ( readCard == addCard ) {
		flashBlue(CARD_ADD_DEL_READY_SIGNAL_DURATION);
	  	while( ! readCard(&userCard) );
	  	addCard(&userCard);
	  	flashGreen(CARD_ADD_LED_FLASH_DURATION);
	} else if ( readCard == deleteCard) {
		flashBlue(CARD_ADD_DEL_READY_SIGNAL_DURATION);
	  	while( ! readCard(&userCard) );
	  	deleteCard(&userCard);
	  	flashGreen(CARD_DEL_LED_FLASH_DURATION);
	} else {
	  	if ( checkCard(&readCard) )
			unlockDoor(DOOR_UNLOCK_DURATION * 1000);
	  	else 
			accessDenied();
	}
  }


  */
  
	/*
  if ( ! rfid.PICC_IsNewCardPresent())
	return;

  if ( ! rfid.PICC_ReadCardSerial())
	return;
  
  flashGreen(250);
  
  Serial.print("PICC type: ");
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
	  piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
	  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
	Serial.println("Your tag is not of type MIFARE Classic.");
	return;
  }
  
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

	for (byte i = 0; i < 4; i++) {
	  cardCode[i] = rfid.uid.uidByte[i];
	}
   
	Serial.print("NUID: ");
	printHex(rfid.uid.uidByte, rfid.uid.size);
	Serial.println();
  }
  else Serial.println("Card read previously.");
  
  Serial.println();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  */
}
 


