//#include <DS3232RTC.h>
#include <FastLED.h>
//sup
#define LED_PIN  3

#define COLOR_ORDER GRB
#define CHIPSET     WS2812B

#define BRIGHTNESS 60

#define LEFTSTARTX 24
#define MIDLEFTSTARTX 17
#define MIDRIGHTSTARTX 8
#define RIGHTSTARTX 1

#define DOTSCOLOR CRGB(255, 255, 255)
#define NUMBERCOLOR CRGB(0, 255, 255)
#define BARSIZE 5



// Helper functions for an two-dimensional XY matrix of pixels.
// Simple 2-D demo code is included as well.
//
//     XY(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;
//             No error checking is performed on the ranges of x and y.
//
//     XYsafe(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;
//             Error checking IS performed on the ranges of x and y, and an
//             index of "-1" is returned.  Special instructions below
//             explain how to use this without having to do your own error
//             checking every time you use this function.  
//             This is a slightly more advanced technique, and 
//             it REQUIRES SPECIAL ADDITIONAL setup, described below.


// Params for width and height
const uint8_t kMatrixWidth = 30;
const uint8_t kMatrixHeight = 10;

// Param for different pixel layouts
const bool    kMatrixSerpentineLayout = true;

// Set 'kMatrixSerpentineLayout' to false if your pixels are 
// laid out all running the same way, like this:
//
//     0 >  1 >  2 >  3 >  4
//                         |
//     .----<----<----<----'
//     |
//     5 >  6 >  7 >  8 >  9
//                         |
//     .----<----<----<----'
//     |
//    10 > 11 > 12 > 13 > 14
//                         |
//     .----<----<----<----'
//     |
//    15 > 16 > 17 > 18 > 19
//
// Set 'kMatrixSerpentineLayout' to true if your pixels are 
// laid out back-and-forth, like this:
//
//     0 >  1 >  2 >  3 >  4
//                         |
//                         |
//     9 <  8 <  7 <  6 <  5
//     |
//     |
//    10 > 11 > 12 > 13 > 14
//                        |
//                        |
//    19 < 18 < 17 < 16 < 15
//
// Bonus vocabulary word: anything that goes one way 
// in one row, and then backwards in the next row, and so on
// is call "boustrophedon", meaning "as the ox plows."


// This function will return the right 'led index number' for 
// a given set of X and Y coordinates on your matrix.  
// IT DOES NOT CHECK THE COORDINATE BOUNDARIES.  
// That's up to you.  Don't pass it bogus values.
//
// Use the "XY" function like this:
//
//    for( uint8_t x = 0; x < kMatrixWidth; x++) {
//      for( uint8_t y = 0; y < kMatrixHeight; y++) {
//      
//        // Here's the x, y to 'led index' in action: 
//        leds[ XY( x, y) ] = CHSV( random8(), 255, 255);
//      
//      }
//    }
//
//
uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }

  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }
  
  return i;
}


// Once you've gotten the basics working (AND NOT UNTIL THEN!)
// here's a helpful technique that can be tricky to set up, but 
// then helps you avoid the needs for sprinkling array-bound-checking
// throughout your code.
//
// It requires a careful attention to get it set up correctly, but
// can potentially make your code smaller and faster.
//
// Suppose you have an 8 x 5 matrix of 40 LEDs.  Normally, you'd
// delcare your leds array like this:
//    CRGB leds[40];
// But instead of that, declare an LED buffer with one extra pixel in
// it, "leds_plus_safety_pixel".  Then declare "leds" as a pointer to
// that array, but starting with the 2nd element (id=1) of that array: 
//    CRGB leds_with_safety_pixel[41];
//    CRGB* const leds( leds_plus_safety_pixel + 1);
// Then you use the "leds" array as you normally would.
// Now "leds[0..N]" are aliases for "leds_plus_safety_pixel[1..(N+1)]",
// AND leds[-1] is now a legitimate and safe alias for leds_plus_safety_pixel[0].
// leds_plus_safety_pixel[0] aka leds[-1] is now your "safety pixel".
//
// Now instead of using the XY function above, use the one below, "XYsafe".
//
// If the X and Y values are 'in bounds', this function will return an index
// into the visible led array, same as "XY" does.
// HOWEVER -- and this is the trick -- if the X or Y values
// are out of bounds, this function will return an index of -1.
// And since leds[-1] is actually just an alias for leds_plus_safety_pixel[0],
// it's a totally safe and legal place to access.  And since the 'safety pixel'
// falls 'outside' the visible part of the LED array, anything you write 
// there is hidden from view automatically.
// Thus, this line of code is totally safe, regardless of the actual size of
// your matrix:
//    leds[ XYsafe( random8(), random8() ) ] = CHSV( random8(), 255, 255);
//
// The only catch here is that while this makes it safe to read from and
// write to 'any pixel', there's really only ONE 'safety pixel'.  No matter
// what out-of-bounds coordinates you write to, you'll really be writing to
// that one safety pixel.  And if you try to READ from the safety pixel,
// you'll read whatever was written there last, reglardless of what coordinates
// were supplied.

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);

uint32_t seconds = 0;
uint32_t hours = 1;
uint32_t minutes = 59;


uint16_t XYsafe( uint8_t x, uint8_t y)
{
  if( x >= kMatrixWidth) return -1;
  if( y >= kMatrixHeight) return -1;
  return XY(x,y);
}


// Demo that USES "XY" follows code below

void loop()
{
    /*
    uint32_t ms = millis();
    int32_t yHueDelta32 = ((int32_t)cos16( ms * (27/1) ) * (350 / kMatrixWidth));
    int32_t xHueDelta32 = ((int32_t)cos16( ms * (39/1) ) * (310 / kMatrixHeight));
    DrawOneFrame( ms / 65536, yHueDelta32 / 32768, xHueDelta32 / 32768);
    if( ms < 5000 ) {
      FastLED.setBrightness( scale8( BRIGHTNESS, (ms * 256) / 5000));
    } else {
      FastLED.setBrightness(BRIGHTNESS);
    }
    FastLED.show();
    */

    seconds += 1;

    if (seconds >= 60) {
      setBackground(CRGB(0, 0, 0));
      minutes += 1;
      seconds = 0;
    }
    if (minutes >= 60) {
      setBackground(CRGB(0, 0, 0));
      hours += 1;
      minutes = 0;
    }
    if (hours > 12) {
      setBackground(CRGB(0, 0, 0));
      hours = 1;
    }
    
    uint8_t hour10 = hours / 10;
    uint8_t hour = hours % 10;
    uint8_t minute10 = minutes / 10;
    uint8_t minute = minutes % 10;

    if (minutes == 50) {
      setBackground(CRGB(255, 0, 0)); 
    }
     
    if(hour10 != 0) {
      showNum(hour10, LEFTSTARTX);
    }
    showNum(hour, MIDLEFTSTARTX);
    showNum(minute10, MIDRIGHTSTARTX);
    showNum(minute, RIGHTSTARTX);

    
    if (seconds % 2 == 0) {
      dotsOff();
    } else {
      dots();
    }

    FastLED.show();
    
    delay(1000);
}

void setBackground(CRGB color) {
  for(uint8_t i = 0; i < kMatrixHeight; i++) {
    for(uint8_t j = 0; j < kMatrixWidth; j++) {
      leds[XY(j, i)] = color;
    }
  }
}

void DrawOneFrame( byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8)
{
  byte lineStartHue = startHue8;
  for( byte y = 0; y < kMatrixHeight; y++) {
    lineStartHue += yHueDelta8;
    byte pixelHue = lineStartHue;      
    for( byte x = 0; x < kMatrixWidth; x++) {
      pixelHue += xHueDelta8;
      leds[ XY(x, y)]  = CHSV( pixelHue, 255, 255);
    }
  }
}


void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness( BRIGHTNESS );

  //one(LEFTSTARTX, 5);
  //two (MIDLEFTSTARTX,5);
  //one (MIDRIGHTSTARTX,5);
  //eight(RIGHTSTARTX,5);

  dots();

  FastLED.show();
}


void showNum(uint8_t n, uint8_t x) {
  if (n == 0) {
    zero(x, BARSIZE);
  } else if (n == 1) {
    one(x, BARSIZE);
  } else if (n == 2) {
    two(x, BARSIZE);
  } else if (n == 3) {
    three(x, BARSIZE);
  } else if (n == 4) {
    four(x, BARSIZE);
  } else if (n == 5) {
    five(x, BARSIZE);
  } else if (n == 6) {
    six(x, BARSIZE);
  } else if (n == 7) {
    seven(x, BARSIZE);
  } else if (n == 8) {
    eight(x, BARSIZE);
  } else if (n == 9) {
    nine(x, BARSIZE);
  }
}

void zero(uint8_t x, uint8_t barSize) {
  // Drawing a zero

  // 2 horizontal bars
  line(x, 0, barSize, true);
  line(x, 2 * barSize - 1, barSize, true);

  // 4 veritcal bars
  line(x, 0, barSize, false);
  line(x, barSize - 1, 5, false);
  line(x + barSize, 0, barSize, false);
  line(x + barSize, barSize, barSize, false);
}

void one(uint8_t x, uint8_t barSize) {
  line(x + 3, 0, barSize, false);
  line(x + 3, barSize, barSize, false);
}

void two(uint8_t x, uint8_t barSize) {
  // 3 horizontal bars
  line(x, 0, barSize, true);
  line(x, barSize - 1, barSize, true);
  line(x, 2 * barSize - 1, barSize, true);

  line(x + barSize, barSize - 1, barSize + 1, false);
  line(x, 0, 5, false);
}

void three(uint8_t x, uint8_t barSize) {
  // 3 horizontal bars
  line(x, 0, barSize, true);
  line(x, barSize - 1, barSize, true);
  line(x, 2 * barSize - 1, barSize, true);

  line(x, 0, barSize, false);
  line(x, barSize - 1, 5, false);
}

void four(uint8_t x, uint8_t barSize) {

  // Horizontal
  line(x, barSize - 1, barSize, true);
  

  line(x, 0, barSize, false);
  line(x, barSize, 5, false);
  line(x + barSize, 0, barSize, false);
}

void five(uint8_t x, uint8_t barSize) {
  // 3 horizontal bars
  line(x, 0, barSize, true);
  line(x, barSize - 1, barSize, true);
  line(x, 2 * barSize - 1, barSize, true);

  line(x + barSize, 0, barSize, false);
  line(x, barSize - 1, 5, false);
}

void six(uint8_t x, uint8_t barSize) {

//vertical
line(x + barSize, 0, barSize, false);
line(x + barSize, barSize - 1, barSize, false);
line(x, barSize - 1, barSize, false);


//horizontal
line(x, 0, barSize, true);
line(x, barSize - 1, barSize, true);
line(x, 2 * barSize - 1, barSize + 1, true);


}

void seven(uint8_t x, uint8_t barSize) {
  line(x, 0, barSize, true);

  line(x, 0, barSize, false);
  line(x, barSize, barSize, false);
}

void eight(uint8_t x, uint8_t barSize) {
  // Drawing an eight

  // 3 horizontal bars
  line(x, 0, barSize, true);
  line(x, barSize - 1, barSize, true);
  line(x, 2 * barSize - 1, barSize, true);

  // 4 veritcal bars
  line(x, 0, barSize, false);
  line(x, barSize - 1, 5, false);
  line(x + barSize, 0, barSize, false);
  line(x + barSize, barSize, barSize, false);
}

void nine(uint8_t x, uint8_t barSize) {
  // Drawing an eight

  // 3 horizontal bars
  line(x, 0, barSize, true);
  line(x, barSize - 1, barSize, true);
  line(x, 2 * barSize - 1, barSize, true);

  // 4 veritcal bars
  line(x, 0, barSize, false);
  line(x, barSize - 1, 5, false);
  line(x + barSize, 0, barSize, false);
}



void dots() {
  leds[XY(15, 3)] = DOTSCOLOR;

  leds[XY(15, 6)] = DOTSCOLOR;
}

void dotsOff() {
  leds[XY(15, 3)] = CRGB(0, 0, 0);

  leds[XY(15, 6)] = CRGB(0, 0, 0);
}

void line(uint8_t x, uint8_t y, uint8_t size, bool horizontal) {
  
  for (byte i = 0; i < size; i++) {
    leds[XY(x, y)] = NUMBERCOLOR;
    
    if (horizontal) {
      x++;
    } else {
      y++;
    }
  }
  
}
