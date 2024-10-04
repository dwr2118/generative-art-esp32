#include <TJpg_Decoder.h>
#include <TFT_eSPI.h> // Graphics and font library
#include <SPI.h> // Needed for TFT_eSPI
#include "backgrounds.h"
#include "snowflakes.h"
#include "leaves.h"
#include "germs.h"

TFT_eSPI tft = TFT_eSPI();  // Initialize display
TFT_eSprite leafSprite = TFT_eSprite(&tft);
TFT_eSprite snowSprite = TFT_eSprite(&tft);
TFT_eSprite germSprite = TFT_eSprite(&tft);
TFT_eSprite img = TFT_eSprite(&tft);

// Struct to hold properties of falling triangles 
struct Triangle {
  int x;    // X position
  int y;    // Y position
  int size; // Size of the triangle
  int speed; // Speed at which the triangle falls
};

// defining the width & height of the image section being drawn 
int screenW=240;
int screenH=135;

// Timing variables
unsigned long lastUpdate = 0;
unsigned long lastParticleUpdate = 0;
// time in milliseconds 
const long interval = 30000; // Change every 10 seconds
const long particleInterval = 2000;
int season = 0;
int notifAmount = 100;

// Array to hold multiple triangles
const int numTriangles = 15;
Triangle triangles[numTriangles];

// Function to initialize a triangle
void initTriangle(Triangle &tri) {
  tri.x = random(0, 200);         // Random x position (assuming landscape)
  tri.y = 0;                      // Start at the top of the screen
  tri.size = random(10, 30);      // Random triangle size
  tri.speed = random(10, 30);       // Random speed for falling
}

// Function to generate a random color
uint16_t randomColor() {
  uint16_t colors[] = {TFT_BROWN, TFT_YELLOW, TFT_MAROON};
  return colors[random(0, 3)];
}

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
   // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // Return 1 to decode next block
  return 1;
}

void setup() {
  tft.begin();
  tft.setRotation(1); // Landscape orientation
  
  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);

  // Draw the image, top left at 0,0
  rePushBackground(season);

  // initialize sprite for moving leaves 
  leafSprite.createSprite(redleafBlackW, redleafBlackH);
  leafSprite.setSwapBytes(true);

  snowSprite.createSprite(snowW, snowH);
  snowSprite.setSwapBytes(true);

  germSprite.createSprite(germW, germH);
  germSprite.setSwapBytes(true);

  // Initialize triangles
  for (int i = 0; i < numTriangles; i++) {
    initTriangle(triangles[i]);
  }

  Serial.begin(115200);
}

void loop() {
  // Get the current time
  unsigned long currentMillis = millis();

  numberBox(notifAmount - season*25, 190, 0);
  
  // Check if it's time to update the background image 
  if (currentMillis - lastUpdate >= interval) {
    lastUpdate = currentMillis; // Reset the timer
    // redraw the correct background image 
    rePushBackground(season);

    if (season > 2){
      season = 0;
      notifAmount = 100;
    } else{
      season++; // time to change the season 
    }
  }

  // Check if it's time to update the location of our dropping particles 
  if (currentMillis - lastParticleUpdate >= particleInterval) {
    lastParticleUpdate = currentMillis; // Reset the timer

    rePushBackground(season);

    // Update and draw each triangle
    for (int i = 0; i < numTriangles; i++) {
      // Update the triangle's position
      triangles[i].y += triangles[i].speed;
      
      // If the triangle goes off the screen, reset it
      if (triangles[i].y > 135) {
        initTriangle(triangles[i]);
      }

      // season, x pos, y pos 
      drawParticles(season, triangles[i].x, triangles[i].y);
    }
    
    delay(10);  // Small delay to control the animation speed
    notifAmount--; 
    
  }

}

void rePushBackground(int currSeason){

  // fall background 
  if (currSeason == 0){
    TJpgDec.drawJpg(0, 0, fall_bg, sizeof(fall_bg));
    
  } 
  // old man coughing background , should have germ emoji as falling particle 
  else if (currSeason == 1){
    TJpgDec.drawJpg(0, 0, old_cough, sizeof(old_cough));
  } 
  
  // Winter background picture 
  else if (currSeason == 2){
    TJpgDec.drawJpg(0, 0, butler_bg, sizeof(butler_bg));
  }
}

void drawParticles(int currSeason, int x, int y){
  if (currSeason == 0){
    leafSprite.pushImage(0, 0, redleafBlackW, redleafBlackH, red_leaf_blackbg);
    leafSprite.pushSprite(x, y, TFT_BLACK);
  } 

  else if (currSeason == 1){
    germSprite.pushImage(0, 0, germW, germH, germ_blackbg);
    germSprite.pushSprite(x, y, TFT_BLACK);
  }

  else if (currSeason == 2){
    snowSprite.pushImage(0, 0, snowW, snowH, snowflake_blackbg);
    snowSprite.pushSprite(x, y, TFT_BLACK);
  }

}

void numberBox(int num, int x, int y)
{
  // Create a sprite 40 pixels wide, 25 high (1kbyte of RAM needed)
  img.createSprite(40, 25);

  // Fill it with a background color (black in this case)
  img.fillSprite(TFT_BLACK);

  // Draw the outline of the envelope
  img.drawRect(0, 5, 40, 20, TFT_WHITE);  // Draw the body of the envelope (rectangle)

  // Draw the envelope flap (triangle at the top)
  img.fillTriangle(0, 5, 20, 0, 40, 5, TFT_WHITE);  // Flap of the envelope

  // Draw the dividing line inside the envelope to simulate the flap folding
  img.drawLine(0, 5, 20, 20, TFT_WHITE);  // Left diagonal line inside envelope
  img.drawLine(40, 5, 20, 20, TFT_WHITE);  // Right diagonal line inside envelope

  // Draw a small circle in the top right corner for the notification
  img.fillCircle(35, 10, 6, TFT_RED);  // Red notification bubble (circle)

  // Set text parameters
  img.setTextSize(0.5); // Font size scaling is x1
  img.setTextDatum(MC_DATUM); // Middle center for text alignment inside the circle
  img.setTextColor(TFT_WHITE, TFT_RED);  // White text, red background (the circle)

  // Draw the notification number inside the red circle
  img.drawNumber(num, 35, 10);  // Draw the number at the center of the circle

  // Push sprite to TFT screen at coordinate x,y (top-left corner)
  img.pushSprite(x, y);

  // Delete sprite to free up the RAM
  img.deleteSprite();

}