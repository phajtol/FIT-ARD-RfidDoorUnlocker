#include<SPI.h>
#include<MFRC522.h>
#include<EEPROM.h>

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
 *      Specifies on which address do values in database begin
 */
#define EEPROM_BEGIN 1

/*
 *      Specifies on which address is size of database stored
 */
#define EEPROM_SIZE 0

/*
 *      Specifies length of card identifier (in bytes)
 */
#define CARD_ID_LENGTH 4




/*
 *	Structure to store RFID card ID. 
 */
typedef struct card {
  byte id[CARD_ID_LENGTH];
} card;


/*
 *      Method used to compare two cards if they are equal.
 *      @param - cards to be compared
 *      @return - true if card are equal (their IDs are the same), false otherwise
 */
bool cardEqual(struct card* x, struct card* y) {
    for(int i = 0; i < CARD_ID_LENGTH; ++i)
      if(x->id[i] != y->id[i])
        return false;
      
    return true;
}

/*
 *      Database size.
 */ 
byte db_size = 0;




/*
 *	RFID instance used to communicate with RFID reader.
 */
MFRC522 rfid(SDA_PIN, RST_PIN);

/*
 *	
 */
struct card addingCard = { { 0x00, 0x00, 0x00, 0x00 } };
struct card deletingCard = { { 0x00, 0x00, 0x00, 0x00 } };
/*struct card addingCard = { { 0x9A, 0x19, 0x4B, 0x06 } }; //green tag from box
struct card deletingCard = { { 0x2A, 0x38, 0xB8, 0x0A } }; //red tag from box*/
struct card userCard;
struct card backup;

String serialInput;



/*
 *	Reads RFID card ID from connected RFID reader if there's card present near RFID reader.
 *	Input/output parameter c will not be modified if there's no card or the card is of incompatible type.
 *	@param c - card instance to write info from RFID card into
 *	@return - indicates whether read has been successful
 */
bool readCard(struct card* c)
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

  for(int i = 0; i < CARD_ID_LENGTH; ++i)
  	c->id[i] = rfid.uid.uidByte[i];

  //close RFID reader resources
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  return true;
}



/*
 *	Checks whether given card is stored in database.
 *	@param - card to be checked
 *	@return - true if card is in database, false otherwise
 */
bool checkCard(struct card* c)
{
  struct card card;
  
  for(int i = 0; i < db_size; ++i){
    for(int j = 0; j < CARD_ID_LENGTH; ++j)
      card.id[j] = EEPROM.read(EEPROM_BEGIN + i * 4 + j);
    
    if( cardEqual( (struct card*) &card, (struct card*) c) )
      return true;
  }
  
  return false;
}



/*
 *	Adds card to database.
 *	@param - card to be added
 */
void addCard(struct card* c)
{  
  for(int i = 0; i < CARD_ID_LENGTH; ++i)
    EEPROM.write(EEPROM_BEGIN + db_size * 4 + i, c->id[i]);

   db_size++;
   EEPROM.write(EEPROM_SIZE, db_size);
}



/*
 *	Deletes card from database.
 *	@param - card to be deleted
 */
void deleteCard(struct card* c)
{
    struct card card;
    
    for(int i = 0; i < db_size; ++i){
      for(int j = 0; j < CARD_ID_LENGTH; ++j)
        card.id[j] = EEPROM.read(EEPROM_BEGIN + i * 4 + j);
     
      if( cardEqual( (struct card*) &card, (struct card*) c) ){
         for(int j = 0; j < CARD_ID_LENGTH; ++j)
           EEPROM.write(EEPROM_BEGIN + i * 4 + j, 0);
         return;
      }
    }
}



/*
 *	Unlocks the electromagnetic lock for specified amount of time and locks it afterwards. 
 *	Turns the green LED on while the lock is unlocked.
 */
void unlockDoor(int miliseconds)
{        
        for(int i = 0; i < 5; ++i){
		flashGreen(100);
                delay(100);
        }
	//lock.unlock();
	flashGreen(miliseconds);
	//lock.lock();
}



/*
 *	Flashes red LED multiple times to signalize invalid card.
 */
void accessDenied()
{
	for(int i = 0; i < 5; ++i){
		flashRed(100);
                delay(100);
        }
}



void printHex(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
        if(i != 0) Serial.print("-");
	if(buffer[i] < 0x10) Serial.print("0");
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



/*
 *	Creates card struct from given String.
 *      Warning: may leave card in inconsistent state if failed
 *	@param - card type output parameter
 *      @param - String to parse from
 *      @return - true if successful, false otherwise
 */
bool parseCard(struct card* card, String input)
{
  for(int i = 0; i < CARD_ID_LENGTH; ++i)
    card->id[i] = 0;
  
  int stringPos = 0;
  for(int i = 0; i < CARD_ID_LENGTH; ++i){
     if(input[stringPos] >= 'A' && input[stringPos] <= 'F') card->id[i] += (input[stringPos] - 'A' + 10) * 16;
     else if(input[stringPos] >= '0' && input[stringPos] <= '9') card->id[i] += (input[stringPos] - '0') * 16;
     else {Serial.print("parsefail, i = ");Serial.print(i);Serial.print(", strpos = ");Serial.println(stringPos);return false;}
     
     stringPos++;
     
     if(input[stringPos] >= 'A' && input[stringPos] <= 'F') card->id[i] += input[stringPos] - 'A' + 10;
     else if(input[stringPos] >= '0' && input[stringPos] <= '9') card->id[i] += input[stringPos] - '0';
     else {Serial.print("parsefail, i = ");Serial.print(i);Serial.print(", strpos = ");Serial.println(stringPos);return false;}
     
     stringPos += 2;
  }
  
  return true;
}




void setup() 
{
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  //uncomment to reset database on startup
  //EEPROM.write(0, 0);
  db_size = EEPROM.read(EEPROM_SIZE);
}



void loop() 
{
  
  if ( readCard(&userCard) ) {
        Serial.print("Card connected (");
        printHex( (byte*) &userCard.id, CARD_ID_LENGTH);
        Serial.print("), ");
        
	if ( cardEqual( (struct card*) &userCard, (struct card*) &addingCard) ) {
                Serial.print("adding: ");
		flashBlue(CARD_ADD_DEL_READY_SIGNAL_DURATION);
	  	while( ! readCard(&userCard) );
                printHex( (byte*) &userCard.id, CARD_ID_LENGTH);
	  	addCard(&userCard);
                Serial.println(" added.");
	  	flashGreen(CARD_ADD_LED_FLASH_DURATION);
	} else if ( cardEqual( (struct card*) &userCard, (struct card*) &deletingCard) ) {
                Serial.print("deleting: ");
		flashBlue(CARD_ADD_DEL_READY_SIGNAL_DURATION);
	  	while( ! readCard(&userCard) );
                printHex( (byte*) &userCard.id, CARD_ID_LENGTH);
                deleteCard(&userCard);
                Serial.println(" deleted.");
	  	flashGreen(CARD_DEL_LED_FLASH_DURATION);
	} else {
	  	if ( checkCard( (struct card*) &userCard) ) {
			Serial.println("approved.");
                        unlockDoor(DOOR_UNLOCK_DURATION * 1000);
	  	} else {
                        Serial.println("refused."); 
			accessDenied();
                }
	}
  }
  
  if( (serialInput = Serial.readString()) != "" ){
    flashBlue(100);
    if(serialInput == "CARD_ADD"){
      Serial.println("Waiting for card to be added...");
      while( (serialInput = Serial.readString()) == "" ){
        flashBlue(100);
        delay(100);
      }
      if( parseCard( &userCard, serialInput) ){
        addCard(&userCard);
        Serial.print("Card (");
        printHex( (byte*) &userCard.id, CARD_ID_LENGTH);
        Serial.println(") added.");
      } else {
        Serial.println("Invalid card string, try again.");
      }
    } else if(serialInput == "CARD_DELETE"){
      Serial.println("Waiting for card to be deleted...");
      while( (serialInput = Serial.readString()) == "" ){
        flashBlue(100);
        delay(100);
      }
      if( parseCard( &userCard, serialInput) ){
        deleteCard(&userCard);
        Serial.print("Card (");
        printHex( (byte*) &userCard.id, CARD_ID_LENGTH);
        Serial.println(") deleted.");
      } else {
        Serial.println("Invalid card string, try again.");
      }
    } else if(serialInput == "CARD_DEFINE_ADDING"){
      for(int i = 0; i < CARD_ID_LENGTH; ++i)
  	backup.id[i] = addingCard.id[i];
      
      Serial.println("Waiting for card to be set as adding card...");
      while( (serialInput = Serial.readString()) == "" ){
        flashBlue(100);
        delay(100);
      }
      if( parseCard( &addingCard, serialInput) ){
        Serial.print("Card (");
        printHex( (byte*) &addingCard.id, CARD_ID_LENGTH);
        Serial.println(") set as adding card.");
      } else {
        for(int i = 0; i < CARD_ID_LENGTH; ++i)
  	  addingCard.id[i] = backup.id[i];
        Serial.println("Invalid card string, try again.");
      }
    } else if(serialInput == "CARD_DEFINE_DELETING"){
      for(int i = 0; i < CARD_ID_LENGTH; ++i)
  	backup.id[i] = deletingCard.id[i];
      
      Serial.println("Waiting for card to be set as deleting card...");
      while( (serialInput = Serial.readString()) == "" ){
        flashBlue(100);
        delay(100);
      }
      if( parseCard( &deletingCard, serialInput) ){
        Serial.print("Card (");
        printHex( (byte*) &deletingCard.id, CARD_ID_LENGTH);
        Serial.println(") set as deleting card.");
      } else {
        for(int i = 0; i < CARD_ID_LENGTH; ++i)
  	  deletingCard.id[i] = backup.id[i];
        Serial.println("Invalid card string, try again.");
      }
    } else if(serialInput == "RESET_DB"){
      db_size = 0;
      EEPROM.write(EEPROM_SIZE, db_size);
      Serial.println("Database has been reset.");
    } else {
      Serial.println("Unknown command, please try again.");
    }
  }

}
 


