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
        if(++mode > 14) mode = 0;
        change = false;
    }

    //these functions are all blocking but check the serial to break

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
        case 10:
            random_walk(50, 30);
            break;
        case 11:
            colorWipe2Strands(neopixels.Color(255,   0,   0), 50);
            break;
        case 12:
            prettyColorWipe(50);
            break;
        case 13:
            prettyWalk(50);
            break;
        case 14:
            prettyComet(100);
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

// Take advantage of the led shape
void colorWipe2Strands(uint32_t color, int wait) {
    int numpix = neopixels.numPixels();
    int half = numpix / 2;
    for (int i=0; i<half; i++) {
        neopixels.setPixelColor(i, color);
        neopixels.setPixelColor(numpix - i, color);
        neopixels.show();
        delay(wait);
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

#define MINIMUM_LUX 50
#define MIDDLE_ADDED_INTENSITY 30

int lux = 30;

uint8_t permute_color(uint8_t start_color, uint8_t max_change) {
	uint8_t rand;

	rand = random(1, max_change);

	//always wander away from overflow/underflow
	if(rand > start_color) {
		return start_color + rand;	
	} else if(255 - rand > start_color) {
		return start_color - rand;
	}
	if(random(1, 2) == 2) {
		return start_color - rand;
	} 
	return start_color + rand;
}
uint8_t calculate_tween(uint8_t start_color, uint8_t end_color, uint8_t this_step, uint8_t tween_constant) {
	float tweenfactor;
	float difference;
	uint8_t change;

	if(this_step == 0) return start_color;
	tweenfactor = ((float) this_step / (float) tween_constant);

	if(end_color > start_color) {
		difference = (float) end_color - (float) start_color;
		change = (uint8_t) (start_color +  difference*tweenfactor);
	} else if(start_color > end_color) {
		difference = (float) start_color - (float) end_color;
		change = (uint8_t) (start_color - difference*tweenfactor);
	} 

	return change;
}
void random_walk(uint8_t max_change, uint8_t tween) {
	uint16_t i, j;
	uint8_t nextred[neopixels.numPixels()];
	uint8_t nextgreen[neopixels.numPixels()];
	uint8_t nextblue[neopixels.numPixels()];
	uint8_t lastred[neopixels.numPixels()];
	uint8_t lastgreen[neopixels.numPixels()];
	uint8_t lastblue[neopixels.numPixels()];

	//set colors separately from brightness
	for(j=0; ; j++) {
		if((j % tween) == 0) {
			//choose a new color for each pixel
			for(i=0; i<neopixels.numPixels(); i++) { 
				if(j == 0 && lastred[i] == 0 && lastgreen[i] == 0 && lastblue[i] == 0) {
					//Serial.print("Randomizing initial value for pixel "); Serial.println(i);
					lastred[i] = random(0, 255);
					lastgreen[i] = random(0, 255);
					lastblue[i] = random(0, 255);
				} else {
					//Serial.print("Shifting old value into previous array for pixel "); Serial.println(i);
					lastred[i] = nextred[i];
					lastgreen[i] = nextgreen[i];
					lastblue[i] = nextblue[i];
				}
			
				nextred[i] = permute_color(lastred[i], max_change);
				nextgreen[i] = permute_color(lastgreen[i], max_change);
				nextblue[i] = permute_color(lastblue[i], max_change);
			} 
		}
		for(i=0; i < neopixels.numPixels(); i++) { 
			uint8_t redtween;
			uint8_t greentween;
			uint8_t bluetween;

			//take tween cycles to go between lastred and nextred, etc
			redtween = calculate_tween(lastred[i], nextred[i], j%tween, tween);
			greentween = calculate_tween(lastgreen[i], nextgreen[i], j%tween, tween);
			bluetween = calculate_tween(lastblue[i], nextblue[i], j%tween, tween);

			#ifdef DEBUG_OUTPUT
			Serial.print("Setting color to "); Serial.print(redtween); Serial.print(", ");
			Serial.print(greentween); Serial.print(", ");
			Serial.print(bluetween); Serial.print(", ");
			Serial.print(" on pixel number "); Serial.print(i);
			Serial.println("");
			#endif

			neopixels.setPixelColor(i, redtween, greentween, bluetween);
	 	}
        neopixels.show();
        if (delay_and_check(10)) { 
            neopixels.clear();        
            break;
        } 
	}
}

void prettyColorWipe(int wait) {
	bool initialized = false;
    uint8_t max_change = 30;
    uint8_t nextred;
	uint8_t nextgreen;
	uint8_t nextblue;
	uint8_t lastred;
	uint8_t lastgreen;
	uint8_t lastblue;

    while(true) {
        if (initialized = false) {
            lastred = random(0, 255);
            lastgreen = random(0, 255);
            lastblue = random(0, 255);
            
            initialized = true;
        } else {
            lastred = nextred;
            lastgreen = nextgreen;
            lastblue = nextblue;
        }
        nextred = permute_color(lastred, max_change);
        nextgreen = permute_color(lastgreen, max_change);
        nextblue = permute_color(lastblue, max_change);
        
        colorWipe(neopixels.Color(nextred, nextgreen, nextblue), wait);
        if (delay_and_check(5)) { 
            neopixels.clear();        
            break;
        } 
    }
}

void prettyWalk(int wait) {
    bool initialized = false;
    uint8_t max_change = 100;
    uint8_t tween = 30;
    uint8_t nextred;
    uint8_t nextgreen;
    uint8_t nextblue;
    uint8_t lastred;
    uint8_t lastgreen;
    uint8_t lastblue;

    while(true) {
        if (initialized = false) {
            lastred = random(0, 255);
            lastgreen = random(0, 255);
            lastblue = random(0, 255);

            initialized = true;
        } else {
            lastred = nextred;
            lastgreen = nextgreen;
            lastblue = nextblue;
        }
        nextred = permute_color(lastred, max_change);
        nextgreen = permute_color(lastgreen, max_change);
        nextblue = permute_color(lastblue, max_change);

        for(int i=0; i<neopixels.numPixels(); i++) { 
            uint8_t redtween;
            uint8_t greentween;
            uint8_t bluetween;

            //take tween cycles to go between lastred and nextred, etc
            redtween = calculate_tween(lastred, nextred, i%tween, tween);
            greentween = calculate_tween(lastgreen, nextgreen, i%tween, tween);
            bluetween = calculate_tween(lastblue, nextblue, i%tween, tween);

            neopixels.setPixelColor(i, redtween, greentween, bluetween);
            neopixels.show();
            delay(wait);
        }
        if (delay_and_check(5)) { 
            neopixels.clear();        
            break;
        } 
    }
}


void prettyComet(int wait) {
    bool initialized = false;
    uint8_t max_change = 50;
    uint8_t tween = 30;
    uint8_t nextred;
    uint8_t nextgreen;
    uint8_t nextblue;
    uint8_t lastred;
    uint8_t lastgreen;
    uint8_t lastblue;
    
    int taillength = neopixels.numPixels() / 4;
    int tailstart = 0;
    float colorstep = 0.95 / taillength;
    int colorslength = taillength + 1;
    int colors[colorslength];
    int tailStart = 0;
    int direction = 1;
    int leftside = -taillength;
    int rightside = neopixels.numPixels();

    while (true) {
        if (initialized = false) {
            lastred = random(0, 255);
            lastgreen = random(0, 255);
            lastblue = random(0, 255);

            initialized = true;
        } else {
            lastred = nextred;
            lastgreen = nextgreen;
            lastblue = nextblue;
        }

        nextred = permute_color(lastred, max_change);
        nextgreen = permute_color(lastgreen, max_change);
        nextblue = permute_color(lastblue, max_change);

        colors[taillength] = neopixels.Color(nextred, nextgreen, nextblue);

        for(int i= taillength - 1; i >= 0; i--) {
            float a = (float) nextred;
            float b = (float) nextblue;
            float c = (float) nextgreen;
            float d = (float) i + 1;

            nextred =  (uint8_t) a * colorstep * d;
            nextblue = (uint8_t) b * colorstep * d;
            nextgreen = (uint8_t) c * colorstep * d;
            
            colors[i] = neopixels.Color(nextred, nextgreen, nextblue);
            Serial.print("Setting color to "); Serial.print(nextred); Serial.print(", ");
            Serial.print(nextblue); Serial.print(", ");
            Serial.print(nextgreen); Serial.print(", ");
            Serial.print(" on pixel number "); Serial.print(i);
            Serial.println("");

        }

        while (true) {
            for(int i=0; i < colorslength; i++) {
                neopixels.setPixelColor(tailstart + i, colors[i]);
            }
            neopixels.show();
            if (delay_and_check(wait)) { 
                neopixels.clear();        
                return;
            }
            neopixels.clear();
            tailstart += direction;
            if(tailstart < leftside || (tailstart >= rightside && !direction != 1)) {
                //change direction
                if (direction == 1) {
                    direction = -1;
                } else {
                    direction = 1;
                }
                //next color
                break;
            }
        }
    }
}
