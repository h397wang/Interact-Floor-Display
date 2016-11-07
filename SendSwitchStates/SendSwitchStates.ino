#include <Wire.h>

/*
 * Created August 31rst
 * Set up for interfacing Arduino slaves with the Pi master
 * such that the quadrant can be calibrated. Refer to diagram set up.
 * The current Arduino sketches poll for switch states and assign colours
 * This is to be decoupled.
 * This sketch sends 40 bits = 5 bytes of data through i2c
 * The bits are encoded into bytes as follows: 
 * Sending a byte value of 15 (base 10) translates into a bitstring
 * of 0000 1111. This indicates that the four switches of lower enumeration
 * all share a state, and the four switches of high enumeration all share 
 * the inverse state. 
 */

#define SLAVE_ADDRESS 0x04
#define NUM_TILES 40
#define NUM_BYTES 5

/* 1: within the bitstring the MSB corresponds to the 
 * tile of lowest enumeration
 * 0: the LSB corresponds to the tile of lowest enumeration
 */
#define LEFT 1

                              
byte theBytes[NUM_BYTES] = {0x00};

int fsrPins[NUM_TILES] = {6,7,8,9,
                            10,11,12,13,
                            22,24,26,28,
                            30,32,34,36,
                            38,40,42,44,
                            46,48,50,52,
                            54,55,56,57,
                            58,59,60,61,
                            62,63,64,65,
                            66,67,68,69};


int request = 0;
int byteCounter = 0; // the index of the byte to be sent

bool donePolling = true; // just experimenting with interrupts

void setup() {
  
 Serial.begin(9600);
 
 for (int i = 0; i < NUM_TILES; i++){
  pinMode(fsrPins[i],INPUT_PULLUP);
 }

 // initialize i2c as slave
 Wire.begin(SLAVE_ADDRESS);
 
 // define callbacks for i2c communication
 Wire.onRequest(sendData); 
}

void loop() {
    //donePolling = false;
  bool switchStates[NUM_TILES] = {0};

  for (int i = 0; i < NUM_TILES; i++){
   
    int byteIndex = i/8;
    byte tempByte;
    int shift = (LEFT? 7-i%8 : i&8); 
    
    if (digitalRead(fsrPins[i]) == LOW){
      tempByte = 0b00000001;
    }else{
      tempByte = 0b00000000; 
    }
    
    tempByte = tempByte << shift;
    //Serial.print("tempByte "); Serial.println(tempByte);
    theBytes[byteIndex] = theBytes[byteIndex] | tempByte;
  }
  
  // this flag tries to account for interrupts...
  donePolling = true;
  //Serial.print("theBytes "); Serial.print(theBytes[0], HEX); Serial.println(theBytes[1], HEX);
 
}


/*
 * This function is being called 5 times to send the 5 bytes
 * On the Pi end, it's making 5 requests
 * TODO: Look into a better way to send multiple bytes
 */
void sendData(){
  
  if (donePolling){
    if (byteCounter == NUM_BYTES){
      byteCounter = 0;
    }else{
      Wire.write(theBytes[byteCounter]); // send the byte
      //Serial.print("Bytes transmitted");
      Serial.println(theBytes[byteCounter],HEX);
      byteCounter++; 
    }
  }
}


