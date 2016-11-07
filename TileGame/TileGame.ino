#include <Adafruit_NeoPixel.h>

/*
 * Game mechanics
 * Tiles are all white by default
 * One red tile will be on the floor at all times
 * Stepping on the red tile ends the game and resets it 
 * When the location of the red tile is about to change, the tile  at the new location flickers between white and red
 * At most one green tile will be on the floor at a time
 * Stepping on the green tile will turn it white and increase the player's score
 * When the green tile is about to expire, it flickers between white and green
 * When the green first spawns, it should be invulnerable...
 */
#define DEBUG 1
 
#define PIXEL_PIN 5
#define NUM_TILES 4
#define NUM_PIXELS 32 // each tile has 8 neopixels
#define PIXELS_PER_TILE 8
#define BRIGHTNESS 155 // adjustable value for all pixels in the strip (0 to 255)
#define FLICKER_CONSTANT 75 // the higher it is, the less flickering for warnings
#define LAVA_TIME 5 // time in seconds that the lava tile is red for
#define LAVA_WARNING_TIME 3 // time in seconds that a white tile blinks for before turning red
int currentLavaTile = 0;  // index of the tile that is red, ext wont be a need for this 
int nextLavaTile = 0; // index of the next tile that is to be red, ext wont be needed
int lavaTileTime = LAVA_WARNING_TIME; // must not be 0

#define GREEN_DURATION 10 // lifespan of any green tile, in seconds 
#define GREEN_WARNING_TIME 3 // time that a green tile flickers for before turning white
#define SPAWN_CONSTANT 985 // the higher it is, the less frequent the event occurs
int currentGreenTile = 0; //index of the tile that is green
bool hasGreenSpawned = false;
int greenTileTime = GREEN_DURATION; // must not be 0

int score = 0; // score of the player 

int fsrPins[NUM_TILES] = {6,7,8,9}; // pins corresponding to the force sensative resistors

int colorsOfTiles[NUM_TILES]; // stores the color state of each tile (e.g. {0,0,0,0} means all white..

bool redOn = false; // flag to control the warning flickering
bool greenOn = true; // flag to control the expiration flickeing

Adafruit_NeoPixel strip = Adafruit_NeoPixel(80, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

int colorArray[][3] = { {127,127,200}, //white/blue/metallic
                        {255,20,0}, //red
                        {0,204 ,0}, //green
                        {128,0,255}, //purple
                        {255,255,0}, //yellow
                        {255,128,0}, //orange
                        {0,0,255}}; //blue
                   
// enumerate the colors to correspond to the color array
#define WHITE 0
#define RED 1
#define GREEN 2
#define PURPLE 3
#define YELLOW 4
#define ORANGE 5
#define BLUE 6


void setup() {
  
  for (int i = 0; i< NUM_TILES; i++){
    pinMode(fsrPins[i],INPUT_PULLUP);
  }
  
  Serial.begin(9600);

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  for (int i = 0; i < NUM_PIXELS; i++){
    strip.setPixelColor(i, strip.Color(colorArray[WHITE][0], colorArray[WHITE][1], colorArray[WHITE][2]));
  }
  strip.show(); // Initialize all pixels to 'off'
  delay(2000);
  
  currentLavaTile = random(0, NUM_TILES);
  nextLavaTile = random(0, NUM_TILES); // initiate the indices of the first two tiles to have lava
  while(nextLavaTile == currentLavaTile){
    nextLavaTile = random(0, NUM_TILES);
  }
  colorsOfTiles[currentLavaTile] = RED;

  // INITIALIZE TIMER INTERRUPTS
  cli(); // disable global interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  OCR1A = 15624; // set compare match register to desired timer count. 16 MHz with 1024 prescaler = 15624 counts/s
  // Bit enumeration: 7,6,5,4,3,2,1,0
  // WGM12: 3, CS10: 1, CS12: 2
  TCCR1B |= (1 << WGM12); // turn on CTC mode. clear timer on compare match
  TCCR1B |= (1 << CS10); // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12); // I think this is setting the clock terminal...
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei(); // enable global interrupts
  
}


void loop() {

  int switchStates[NUM_TILES]; // globals are awkward because many functions can modify the same variable
  getTileStates(switchStates); // updates the flags
  
  if (lavaTileTime == 0){
    updateLava();
  }

  if (!hasGreenSpawned){
    spawnGreen(); // 
  }else{
    if (greenTileTime == 0){ // the green tile has expired
      consumeGreen();
    }
  }
  
  flicker(switchStates); // sets the colours
  delay(50);
}


/* Polls for the states of the tiles and updates the flags in the swtichStates array
 * to keep track of which tiles are being stood on
 */

void getTileStates(int switchStates[]){
  for (int i = 0; i < NUM_TILES; i++){
    if (digitalRead(fsrPins[i]) == LOW){ // being stepped on
      switchStates[i] = 1;
      if (i == currentLavaTile){
        Serial.print("You stepped on lava");
        reset();
      }else if (hasGreenSpawned && i == currentGreenTile && greenTileTime < GREEN_DURATION -1){
        Serial.print("You stepped on a green tile, you're score is :");
        Serial.println(score++);   
        consumeGreen();
      }
    }else { // not stepped on
      switchStates[i] = 0; 
    }
  }
}

/*
 * Called from the main loop, if the lavaTime has reached 0
 * Randomly generates the index location of the next lava tile 
 * Updates the color states within the colorsOfTiles array
 * 
 */
 
void updateLava(){
      Serial.println("Update lava called");
      // update the array of tile color states
      colorsOfTiles[currentLavaTile] = WHITE; // lava tile has expired, turn it white
      colorsOfTiles[nextLavaTile] = RED; // new lava tile
      
      int copyNextLavaTile = nextLavaTile;
     
      currentLavaTile = copyNextLavaTile;
      lavaTileTime = LAVA_TIME;

      nextLavaTile = random(0,NUM_TILES);      // change the location of the next lava
      while(nextLavaTile == currentLavaTile || (hasGreenSpawned && (nextLavaTile == currentGreenTile))){
        nextLavaTile = random(0,NUM_TILES);
      }
  
}

/*
 * Spawn check alrady done
 * 
 */
void spawnGreen(){

  // there might not be a green tile at some times
  if (random(0,1000) >= SPAWN_CONSTANT){

     // randomly select a tile for it to spawn on
    currentGreenTile = random(0,NUM_TILES);
    while(currentGreenTile == currentLavaTile || currentGreenTile == nextLavaTile){
      currentGreenTile = random(0,NUM_TILES);
    }
    colorsOfTiles[currentGreenTile] = GREEN;
    greenTileTime = GREEN_DURATION;
    hasGreenSpawned = true;
    Serial.println("Green has spawned");
    
  }
}

/*
 * Responsible for removing the green tile
 * whether the tile was tapped by the player or expired
 */
void consumeGreen(){
  hasGreenSpawned = false;
  colorsOfTiles[currentGreenTile] = WHITE;
  Serial.println("Green consumed");
}

/* interrupt set to occur every 1 second
 * should really keep it clean and short
 * for now it just counts down for the single lava tile
 * when the timer reaches 0, a new tile index is assigned for the lava tile
 */
 
ISR(TIMER1_COMPA_vect){ 

  Serial.print("currentLavaTile: "); Serial.println(currentLavaTile);
  Serial.print("nextLavaTile: "); Serial.println( nextLavaTile);
  Serial.print("currentGreenTile: "); Serial.println(currentGreenTile);
  
   lavaTileTime--;
   if (hasGreenSpawned){
    greenTileTime--;
   }
}

/*
 * update the pixels based on the switchState flags
 * 
 */
void flicker(int switchStates[]){
   
    int R = 0;
    int G = 0;
    int B = 0;
    int noise = 0 ;
    int colorNum = 0; 
    
    for (int i = 0; i < NUM_TILES; i++){
        colorNum = colorsOfTiles[i];
        R = constrain(colorArray[colorNum][0],0,255);
        G = constrain(colorArray[colorNum][1],0,255);
        B = constrain(colorArray[colorNum][2],0,255);
        
        for (int j = PIXELS_PER_TILE*i; j < PIXELS_PER_TILE*(i+1); j++){
         

          // Uncomment for noise
          /* 
          int updateChance = random(0,100);
          if (updateChance <= 50){
            int noise = random(-30,30);
            R += noise; G += noise; B += noise;
          }
          */
          strip.setPixelColor(j, strip.Color(R,G,B));
        }
        
        // warning flicker for a red tile that is about to spawn
        // do not toggle the flags for this, bad idea 
        // this will over write the strips that were set above
        // maybe instead of flashing between the colors, have it fade???
        if (i == nextLavaTile && lavaTileTime < LAVA_WARNING_TIME && random(0,100) > FLICKER_CONSTANT){
          //Serial.println("Flash red and white");
          colorNum = (redOn? WHITE:RED);  
          redOn = !redOn; // toggle the flag      
          
          for (int j = PIXELS_PER_TILE*i; j < PIXELS_PER_TILE*(i + 1); j++){
            strip.setPixelColor(j, strip.Color(colorArray[colorNum][0], colorArray[colorNum][1],colorArray[colorNum][2]));
          }
        }
        
        // same as above, warning flicker for the green tile that is about to expire
        if (i == currentGreenTile && hasGreenSpawned && greenTileTime < GREEN_WARNING_TIME && random(0,100) > FLICKER_CONSTANT){
          
          colorNum = (greenOn? WHITE:GREEN);           
          greenOn = !greenOn; // toggle the flag      
          
          for (int j = PIXELS_PER_TILE*i; j < PIXELS_PER_TILE*(i + 1); j++){
            strip.setPixelColor(j, strip.Color(colorArray[colorNum][0], colorArray[colorNum][1],colorArray[colorNum][2]));
          }
        }
    
    }
    
    strip.show();
}


// turn it all red to indicate a fail maybe
void reset(){

  for (int i = 0; i < NUM_TILES; i++){
    colorsOfTiles[i] = WHITE;   
  }
  
  for (int i = 0; i < NUM_PIXELS; i++){ 
    strip.setPixelColor(i, strip.Color(colorArray[RED][0], colorArray[RED][1], colorArray[RED][2]));
  }
  
  strip.show();
  delay(3000);
  
  for (int i = 0; i < NUM_PIXELS; i++){
    strip.setPixelColor(i, strip.Color(colorArray[WHITE][0], colorArray[WHITE][1], colorArray[WHITE][2]));
  }
  
  strip.show();
  delay(3000);

  // re-initialize
  currentLavaTile = random(0, NUM_TILES);
  nextLavaTile = random(0, NUM_TILES); // initiate the indices of the first two tiles to have lava
  while(nextLavaTile == currentLavaTile){
    nextLavaTile = random(0, NUM_TILES);
  }
  colorsOfTiles[currentLavaTile] = RED;

  lavaTileTime = LAVA_TIME;
  
}

