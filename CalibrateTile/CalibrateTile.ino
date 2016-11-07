
/*
 * Created July 20 2016
 * This script is used to calibrate the sensitivity of the pressure tiles.
 * Make sure the connections are properly set up and variables are appropriate
 * 
 */
#include <Adafruit_NeoPixel.h>

#define RED 0
#define GREEN 1
#define PURPLE 2
#define YELLOW 3
#define CYAN 4
#define ORANGE 5
#define BLUE 6
#define MAGENTA 7
#define WHITE 9

#define PIN 5 // neo pixel data line pin

// to be adjusted
#define BRIGHTNESS 150
#define PIXELS_PER_TILE 8
#define NUM_SWITCHES 80

int DEFAULT_COLOUR = YELLOW;
int TRIGGER_COLOUR = RED;

int long t = 0;

int fsrPins[NUM_SWITCHES] = {6,7,8,9,
                            10,11,12,13,
                            22,24,26,28,
                            30,32,34,36,
                            38,40,42,44,
                            46,48,50,52,
                            54,55,56,57,
                            58,59,60,61,
                            62,63,64,65,
                            66,67,68,69};


int NUMPIXELS = NUM_SWITCHES*PIXELS_PER_TILE;
int score = 0; 

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int colorArray[][3] = { {255,20,0},    //red
                        {0,204 ,0}, //green
                        {128,0,255}, //purple
                        {255,255,0}, //yellow
                        {0,191,255}, //cyan
                        {255,128,0}, //orange
                        {0,0,255}, //blue
                        {255,0,255}, //magenta
                        {102,51,0}, //brown??????
                        {127,127,127}, //white
                        {127,127,200}, //white/blue
                        {255,129,129}, //white/red
                        {255,255,80}, //white/yellow
                        {80,255,80}, //white/green
                        {255,80,255}, //dummy color
                        {127,127,200}, //dummy
                        {255,129,129}, //dummy
                        {255,255,80}, //dummy
                        {80,255,80}, //dummy color
                        {80,255,80}}; //dummy color

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i<(sizeof(fsrPins)/sizeof(int)); i++){
    pinMode(fsrPins[i],INPUT_PULLUP);
  }
  Serial.begin(9600);

  strip.setBrightness(BRIGHTNESS);

  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}


void loop() {
  // put your main code here, to run repeatedly:
  
  int state = 0;
  t = millis();
  int switchStates[NUM_SWITCHES];
  switchRead(switchStates);
  flicker(switchStates);
  int delta = millis() - t;
  //Serial.println(delta);
}



//returns array based on which inputs are set to high
void switchRead(int switchStates[]){
  for (int i = 0; i<(sizeof(fsrPins)/sizeof(int)); i++){
    if(digitalRead(fsrPins[i]) == LOW){
      switchStates[i] = 1;
    }
    else {
      switchStates[i] = 0;
    }
    //Serial.print(switchStates[i]);
  }
  //Serial.println("");
}

void flicker(int switchStates[]){
    int R = 0;
    int G = 0;
    int B = 0;
    int noise = 0 ;
    int updateNumber = 0;
    int pixelIndex = 0;
    int colorNum = 0;
    for (uint16_t i = 0; i < NUM_SWITCHES; i = i +1){
   
      noise = 2; //random (-20,20);
        //updateNumber  = random(0,100);                             
     
      for (int j = 0; j < PIXELS_PER_TILE; j++){
        pixelIndex = PIXELS_PER_TILE*i + j; 
         
        if (switchStates[i] == 0){
          colorNum = DEFAULT_COLOUR;
        }else if (switchStates[i] == 1){
          colorNum = TRIGGER_COLOUR;
        }
      
      
      //clamping values for RGB, if over 255 rolls over
        R = constrain(colorArray[colorNum][0]+noise,0,255);
        G = constrain(colorArray[colorNum][1]+noise,0,255);
        B = constrain(colorArray[colorNum][2]+noise,0,255);     
      //random chance for updating,increases flicker
        strip.setPixelColor(pixelIndex,strip.Color(R,G,B));

        // random colours
       /*
        if (colorNum == TRIGGER_COLOUR){
          strip.setPixelColor(pixelIndex,strip.Color(R,G,B));
        }else{
          if (random(0,10) > 2){
            colorNum = random(0,8);  
            R = constrain(colorArray[colorNum][0]+noise,0,255);
            G = constrain(colorArray[colorNum][1]+noise,0,255);
            B = constrain(colorArray[colorNum][2]+noise,0,255); 
            strip.setPixelColor(pixelIndex,strip.Color(R,G,B));
          }
        }
        */
        
      }
      
      
  }
  strip.show();
}

