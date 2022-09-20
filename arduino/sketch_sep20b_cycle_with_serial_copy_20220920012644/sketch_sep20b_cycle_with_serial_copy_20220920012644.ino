#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include <NeoPixelPainter.h>

const int duration = 2000; //number of loops to run each animation for

#define NUMBEROFPIXELS 20 //Number of LEDs on the strip
#define PIXELPIN 4 //Pin where WS281X pixels are connected

// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 50 // Set BRIGHTNESS to about 1/5 (max = 255)

Adafruit_NeoPixel neopixels = Adafruit_NeoPixel(NUMBEROFPIXELS, PIXELPIN, NEO_GRB + NEO_KHZ800);

NeoPixelPainterCanvas pixelcanvas = NeoPixelPainterCanvas(&neopixels); //create canvas, linked to the neopixels (must be created before the brush)
NeoPixelPainterBrush pixelbrush = NeoPixelPainterBrush(&pixelcanvas); //crete brush, linked to the canvas to paint to


void setup() {
  randomSeed(analogRead(0)); //new random seed 
  pinMode(PIXELPIN, OUTPUT);

  neopixels.begin();
  neopixels.setBrightness(BRIGHTNESS);

  Serial.begin(9600);
	Serial.println("wtf");
	Serial.println("NeoPixel Painter Demo");

	//check if ram allocation of brushes and canvases was successful (painting will not work if unsuccessful, program should still run though)
	//this check is optional but helps to check if something does not work, especially on low ram chips like the Arduino Uno
	if (pixelcanvas.isvalid() == false) Serial.println("canvas allocation problem");
	else  Serial.println("canvas allocation ok");


	if (pixelbrush.isvalid() == false) Serial.println("brush allocation problem");
	else  Serial.println("brush allocation ok");

}

unsigned long loopcounter; //count the loops, switch to next animation after a while
bool initialized = false; //initialize the canvas & brushes in each loop when zero

int mode = 0;
char input;

void loop() {
  bool change = false;
  if(Serial.available()){
        input = Serial.read();
        if(input) {
            Serial.print("Changing mode");
            Serial.println(input);
            delay(20);
            change = true;
        }
  }



  if (change == true) {
    Serial.println("ready to update");
    if(++mode > 9) mode = 0;
    change = false;
  }
  
  switch(mode) {
      case 0:
          colorWipe(neopixels.Color(0,   255,   0), 50);    // Green 
          break;
        case 1:
          colorWipe(neopixels.Color(255,   0,   0), 50);    // Red
          break;
        case 2:
          colorWipe(neopixels.Color(  0,   0,   0), 50);    // Black/off
          break;
        case 3: 
          sparkler(50);
          break;
        case 4:
          rainbow(50);
          break;
        case 5:
          theaterChaseRainbow(200);
          break;
        case 6:
          theaterChase(neopixels.Color(127, 127, 127), 200); // White
          break;
        case 7:
          bouncyballs(25);
          break;
        case 8:
          twobrushcolor(50);
          break;
        case 9:
          huefader(50);
          break;
          
    }

}

bool delay_and_check(int wait) {
  if(Serial.available()) {
    return true;
  } 
  delay(wait);
  return false;
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<neopixels.numPixels(); i++) { // For each pixel in neopixels...
    neopixels.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    neopixels.show();                          //  Update neopixels to match
    delay(wait);                           //  Pause for a moment
  }
}

void sparkler(int wait) {
//SPARKLER: a brush seeding sparkles
initialized = false;
	while(1)
	{

		HSV brushcolor;

		if (initialized == false)
		{
			initialized = true;
			pixelbrush.setSpeed(600);
			pixelbrush.setFadeout(true); //sparkles fade in
			pixelbrush.setFadein(true);  //and fade out immediately after reaching the set brightness
		}

		//set a new brush color in each loop
		brushcolor.h = random(255); //random color
		brushcolor.s = random(130); //random but low saturation, giving white-ish sparkles
		brushcolor.v = random(200); //random (peak) brighness

		pixelbrush.setColor(brushcolor);
		pixelbrush.setFadeSpeed(random(100) + 150); //set a new fadespeed with some randomness

		neopixels.clear();

		pixelbrush.paint(); //paint the brush to the canvas (and update the brush, i.e. move it a little)
		pixelcanvas.transfer(); //transfer (add) the canvas to the neopixels

		neopixels.show();

    if (delay_and_check(wait)) { 
      break;
    } 
	}
}

// Rainbow cycle along whole neopixels. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 3 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 3*65536. Adding 256 to firstPixelHue each time
  // means we'll make 3*65536/256 = 768 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 3*65536; firstPixelHue += 256) {
    for(int i=0; i<neopixels.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (neopixels.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / neopixels.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      neopixels.setPixelColor(i, neopixels.gamma32(neopixels.ColorHSV(pixelHue)));
    }
    neopixels.show(); // Update neopixels with new contents
    if (delay_and_check(wait)) { 
      break;
    } 
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      neopixels.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<neopixels.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / neopixels.numPixels();
        uint32_t color = neopixels.gamma32(neopixels.ColorHSV(hue)); // hue -> RGB
        neopixels.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      neopixels.show();                // Update neopixels with new contents
    if (delay_and_check(wait)) { 
      neopixels.clear();
      break;
    }               
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      neopixels.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<neopixels.numPixels(); c += 3) {
        neopixels.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      neopixels.show(); // Update strip with new contents
      if (delay_and_check(wait)) { 
        neopixels.clear();        
        break;
    } 
    }
  }
}

void bouncyballs(int wait) {
  initialized = false;
//-------------
	//BOUNCY BALLS
  //-------------
	//three brushes painting on one canvas, attracted to the zero pixel as if by gravity

	while(true) //create a loop with two additional brushes (are deleted automatically once the loop finishes)
	{

		//create additional brushes, painting on the same canvas as the globally defined brush
		NeoPixelPainterBrush pixelbrush2 = NeoPixelPainterBrush(&pixelcanvas); 
		NeoPixelPainterBrush pixelbrush3 = NeoPixelPainterBrush(&pixelcanvas); 

		if (pixelbrush2.isvalid() == false) Serial.println(F("brush2 allocation problem"));
		else  Serial.println(F("brush2 allocation ok"));

		if (pixelbrush3.isvalid() == false) Serial.println(F("brush3 allocation problem"));
		else  Serial.println(F("brush3 allocation ok"));

		byte skipper = 0;

		while (true) 
		{


			if (initialized == false) //initialize the brushes
			{
				initialized = true;

				HSV brushcolor;

				brushcolor.h = 20; //orange
				brushcolor.s = 240; //almost full saturation
				brushcolor.v = 150; //medium brightness

				//first brush
				pixelbrush.setSpeed(0); //zero initial speed
				pixelbrush.setFadeSpeed(150);
				pixelbrush.setFadeout(true);
				pixelbrush.setColor(brushcolor);
				pixelbrush.moveTo(NUMBEROFPIXELS - 1); //move to end of the strip
				pixelbrush.setBounce(true); //bounce if either end of the strip is reached

				//second brush
				brushcolor.h = 220; //pink
				pixelbrush2.setSpeed(0); //zero initial speed
				pixelbrush2.setFadeSpeed(190);
				pixelbrush2.setFadeout(true);
				pixelbrush2.setColor(brushcolor);
				pixelbrush2.moveTo(NUMBEROFPIXELS / 3); //move to one third of the strip
				pixelbrush2.setBounce(true);

				brushcolor.h = 70; //green-ish (pure green is 85 or 255/3)
				pixelbrush3.setSpeed(0);
				pixelbrush3.setFadeSpeed(220);
				pixelbrush3.setFadeout(true);
				pixelbrush3.setColor(brushcolor);
				pixelbrush3.moveTo(2 * NUMBEROFPIXELS / 3);
				pixelbrush3.setBounce(true);
			}

			//apply some gravity force that accelerates the painters (i.e. add speed in negative direction = towards zero pixel)
			//  if (skipper % 5 == 0) //only apply gravity at some interval to make it slower on fast processors
			//  {
			//read current speed of each brush and speed it up in negative direction (towards pixel zero)
			pixelbrush.setSpeed(pixelbrush.getSpeed() - 10); 
			pixelbrush2.setSpeed(pixelbrush2.getSpeed() - 10);
			pixelbrush3.setSpeed(pixelbrush3.getSpeed() - 10);
			//  }
			//  skipper++;


			neopixels.clear();

			pixelbrush.paint(); //apply the paint of the first brush to the canvas (and update the brush)
			pixelbrush2.paint(); //apply the paint of the second brush to the canvas (and update the brush)
			pixelbrush3.paint(); //apply the paint of the third brush to the canvas (and update the brush)
			pixelcanvas.transfer(); //transfer the canvas to the neopixels (and update i.e. fade pixels)

			neopixels.show();
      if (delay_and_check(wait)) { 
        neopixels.clear();        
        break;
      } 
		}
		break; //quit the while loop immediately (and delete the created brush)
	}

}

void twobrushcolor(int wait) {
  initialized = false;
//---------------------
	//TWO-BRUSH-COLORMIXING
  //---------------------

	//two brushes moving around randomly paint on their individual canvas, resulting in colors being mixed 
	while(true) //create a loop with two additional brushes (are deleted automatically once the loop finishes)
	{

		//create an additional canvas for the second brush (needs to be created before the brush)
		NeoPixelPainterCanvas pixelcanvas2 = NeoPixelPainterCanvas(&neopixels);

		//create additional brush, painting on the second canvas 
		NeoPixelPainterBrush pixelbrush2 = NeoPixelPainterBrush(&pixelcanvas2); 

		if (pixelcanvas2.isvalid() == false) Serial.println("canvas2 allocation problem");
		else  Serial.println("canvas2 allocation ok");

		if (pixelbrush2.isvalid() == false) Serial.println(F("brush2 allocation problem"));
		else  Serial.println(F("brush2 allocation ok"));


		while(true)
		{


			HSV brushcolor;
			static bool firstrun = true;

			brushcolor.s = 255; //full color saturation
			brushcolor.v = 100; //medium-low brightness

			if (initialized == false) //initialize the brushes
			{
				initialized = true;

				brushcolor.h = 8;

				//setup the first brush
				pixelbrush.setSpeed(-750);
				pixelbrush.setSpeedlimit(1000);
				pixelbrush.setFadeSpeed(random(80) + 50);
				pixelbrush.setFadeout(true);
				pixelbrush.setFadein(true);
				pixelbrush.setColor(brushcolor);
				pixelbrush.moveTo(random(NUMBEROFPIXELS));
				pixelbrush.setBounce(true);

				//setup the second brush
				brushcolor.h = 160;
				pixelbrush2.setSpeed(600);
				pixelbrush2.setSpeedlimit(1000);
				pixelbrush2.setFadeSpeed(random(80) + 50);
				pixelbrush2.setFadeout(true);
				pixelbrush2.setFadein(true);
				pixelbrush2.setColor(brushcolor);
				pixelbrush2.moveTo(random(NUMBEROFPIXELS));
				pixelbrush2.setBounce(true);
			}


			if (rand() % 10) //slowly (and randomly) change hue of brushes
			{
				brushcolor = pixelbrush.getColor();
				brushcolor.h += random(3) - 1; //randomly change hue a little ( ± random(1))
				pixelbrush.setColor(brushcolor);

				brushcolor = pixelbrush2.getColor();
				brushcolor.h += random(3) - 1; //randomly change hue a little ( ± random(1))
				pixelbrush2.setColor(brushcolor);

			}

			//slowly change speed of both brushes
			pixelbrush.setSpeed(pixelbrush.getSpeed() + random(6) - 3); //means speed = currentspeed ± random(3)
			pixelbrush2.setSpeed(pixelbrush2.getSpeed() + random(6) - 3); //means speed = currentspeed ± random(3)


			neopixels.clear(); //remove any previously applied paint

			pixelbrush.paint(); //apply the paint of the first brush to its assigned canvas (and update the brush)
			pixelbrush2.paint(); //apply the paint of the second brush to  its assigned canvas (and update the brush)

			pixelcanvas.transfer(); //transfer the first canvas to the neopixels
			pixelcanvas2.transfer(); //transfer the sedonc canvas to the neopixels (adding colors, rather than overwriting colors)

			neopixels.show();
      if (delay_and_check(wait)) { 
        neopixels.clear();        
        break;
      } 
		}
		break; //quit the while loop immediately (and delete the created brush)
	}
}

void huefader(int wait) {
  initialized = false;
   //------------------------------
	//HUE FADER: demo of hue fading
  //------------------------------

	//hue fading can be done in two ways: change the color moving the shortest distance around the colorwheel (setFadeHueNear)
	//or intentionally moving around the colorwheel choosing the long way (setFadeHueFar)
	//two brushes move along the strip in different speeds, each painting a different color that the canvas will then fade to
	//a new color is set when the first brush passes pixel 0
	//both brushes act on the same canvas

	while(true) //create a loop with an additional brush (is deleted automatically once the loop finishes)
	{

		//create an additional brush, painting on the same canvas as the globally defined brush
		NeoPixelPainterBrush pixelbrush2 = NeoPixelPainterBrush(&pixelcanvas); 

		if (pixelbrush2.isvalid() == false) Serial.println(F("brush2 allocation problem"));
		else  Serial.println(F("brush2 allocation ok"));

     pixelcanvas.clear(); //clear the canvas

		while(true)
		{

			static unsigned int lastposition = 0; //to detect zero crossing only once (brush may stay at pixel zero for some time since it uses sub-pixel resolution movement)

			if (pixelbrush.getPosition() == 0 && lastposition > 0) initialized = false; //choose new color & speed if brush reaches pixel 0

			lastposition = pixelbrush.getPosition(); //save current position for next position check

			if (initialized == false)
			{
				initialized = true;

				HSV brushcolor;
   

				brushcolor.h = random(255); //random color
				brushcolor.s = 255; //full saturation
				brushcolor.v = 130; //medium brightness

				pixelbrush.setSpeed(random(150) + 150); //random movement speed
				pixelbrush.setFadeSpeed(random(10) + 20); //set random fading speed
				pixelbrush.setColor(brushcolor); //update the color of the brush
				pixelbrush.setFadeHueNear(true); //fade using the near path on the colorcircle
				
				//second brush paints on the same canvas
				brushcolor.h = random(255);
				pixelbrush2.setSpeed(random(150) + 150);
				pixelbrush2.setFadeSpeed(random(10) + 20);
				pixelbrush2.setColor(brushcolor);
				pixelbrush2.setFadeHueNear(true); //fade using the near path on the colorcircle
				//pixelbrush.setFadeHueFar(true); //fade using the far path on the colorcircle (if both are set, this path is chosen)
				pixelbrush2.setBounce(true); //bounce this brush at the end of the strip
			}

			neopixels.clear();

			pixelbrush.paint(); //apply the paint of the first brush to the canvas (and update the brush)
			pixelbrush2.paint(); //apply the paint of the second brush to the canvas (and update the brush)
			pixelcanvas.transfer(); //transfer the canvas to the neopixels

			neopixels.show();
      if (delay_and_check(wait)) { 
        neopixels.clear();        
        break;
      } 
		}
		break; //quit the while loop immediately (and delete the created brush)
	}

}