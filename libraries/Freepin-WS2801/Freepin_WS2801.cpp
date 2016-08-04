#include "Freepin_WS2801.h"

// Example to control WS2801-based RGB LED Modules in a strand or strip
// Written by Adafruit - MIT license
/*****************************************************************************/

// Constructor for use with arbitrary clock/data pins:
Freepin_WS2801::Freepin_WS2801(uint16_t n, uint8_t dpin, uint8_t cpin, uint8_t order) {
  rgb_order = order;
  alloc(n);
  updatePins(dpin, cpin);
}

// Allocate 3 bytes per pixel, init to RGB 'off' state:
void Freepin_WS2801::alloc(uint16_t n) {
  begun   = false;
  numLEDs = ((pixels = (uint8_t *)malloc(n*3)) != NULL) ? n : 0;
}

// Release memory (as needed):
Freepin_WS2801::~Freepin_WS2801(void) {
  if (pixels != NULL) {
    free(pixels);
  }
}

// Activate soft SPI:
void Freepin_WS2801::begin(void) {
    pinMode(datapin, OUTPUT);
    pinMode(clkpin , OUTPUT);
    begun = true;
}


// Change pin assignments post-constructor, using arbitrary pins:
void Freepin_WS2801::updatePins(uint8_t dpin, uint8_t cpin) {

  if(begun == true) { // If begin() was previously invoked...
    // Regardless, now enable output on 'soft' SPI pins:
    pinMode(dpin, OUTPUT);
    pinMode(cpin, OUTPUT);
  } // Otherwise, pins are not set to outputs until begin() is called.

  // Note: any prior clock/data pin directions are left as-is and are
  // NOT restored as inputs!

  hardwareSPI = false;
  datapin     = dpin;
  clkpin      = cpin;
  clkpinmask  = digitalPinToBitMask(cpin);
  datapinmask = digitalPinToBitMask(dpin);
}

uint16_t Freepin_WS2801::numPixels(void) {
  return numLEDs;
}

// Change strand length (see notes with empty constructor, above):
void Freepin_WS2801::updateLength(uint16_t n) {
  if(pixels != NULL) free(pixels); // Free existing data (if any)
  // Allocate new data -- note: ALL PIXELS ARE CLEARED
  numLEDs = ((pixels = (uint8_t *)calloc(n, 3)) != NULL) ? n : 0;
  // 'begun' state does not change -- pins retain prior modes
}

// Change RGB data order (see notes with empty constructor, above):
void Freepin_WS2801::updateOrder(uint8_t order) {
  rgb_order = order;
  // Existing LED data, if any, is NOT reformatted to new data order.
  // Calling function should clear or fill pixel data anew.
}

void Freepin_WS2801::show(void) {
  uint16_t i, nl3 = numLEDs * 3; // 3 bytes per LED
  uint8_t  bit;

    for(i=0; i<nl3; i++ ) {
      for(bit=0x80; bit; bit >>= 1) {

	if(pixels[i] & bit) digitalWrite(datapin, HIGH);
        else                digitalWrite(datapin, LOW);
        digitalWrite(clkpin, HIGH);
        digitalWrite(clkpin, LOW);

      }
    }

  delay(1); // Data is latched by holding clock pin low for 1 millisecond
}

// Set pixel color from separate 8-bit R, G, B components:
void Freepin_WS2801::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  if(n < numLEDs) { // Arrays are 0-indexed, thus NOT '<='
    uint8_t *p = &pixels[n * 3];
    // See notes later regarding color order
    if(rgb_order == WS2801_RGB) {
      *p++ = r;
      *p++ = g;
    } else {
      *p++ = g;
      *p++ = r;
    }
    *p++ = b;
  }
}

// Set pixel color from separate 8-bit R, G, B components using x,y coordinate system:
void Freepin_WS2801::setPixelColor(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
  boolean evenRow = ((y % 2) == 0);
  // calculate x offset first
  uint16_t offset = x % width;
  if (!evenRow) {
    offset = (width-1) - offset;
  }
  // add y offset
  offset += y * width;
  setPixelColor(offset, r, g, b);
}

// Set pixel color from 'packed' 32-bit RGB value:
void Freepin_WS2801::setPixelColor(uint16_t n, uint32_t c) {
  if(n < numLEDs) { // Arrays are 0-indexed, thus NOT '<='
    uint8_t *p = &pixels[n * 3];
    // To keep the show() loop as simple & fast as possible, the
    // internal color representation is native to different pixel
    // types.  For compatibility with existing code, 'packed' RGB
    // values passed in or out are always 0xRRGGBB order.
    if(rgb_order == WS2801_RGB) {
      *p++ = c >> 16; // Red
      *p++ = c >>  8; // Green
    } else {
      *p++ = c >>  8; // Green
      *p++ = c >> 16; // Red
    }
    *p++ = c;         // Blue
  }
}

// Set pixel color from 'packed' 32-bit RGB value using x,y coordinate system:
void Freepin_WS2801::setPixelColor(uint16_t x, uint16_t y, uint32_t c) {
  boolean evenRow = ((y % 2) == 0);
  // calculate x offset first
  uint16_t offset = x % width;
  if (!evenRow) {
    offset = (width-1) - offset;
  }
  // add y offset
  offset += y * width;
  setPixelColor(offset, c);
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t Freepin_WS2801::getPixelColor(uint16_t n) {
  if(n < numLEDs) {
    uint16_t ofs = n * 3;
    // To keep the show() loop as simple & fast as possible, the
    // internal color representation is native to different pixel
    // types.  For compatibility with existing code, 'packed' RGB
    // values passed in or out are always 0xRRGGBB order.
    return (rgb_order == WS2801_RGB) ?
      ((uint32_t)pixels[ofs] << 16) | ((uint16_t) pixels[ofs + 1] <<  8) | pixels[ofs + 2] :
      (pixels[ofs] <<  8) | ((uint32_t)pixels[ofs + 1] << 16) | pixels[ofs + 2];
  }

  return 0; // Pixel # is out of bounds
}

