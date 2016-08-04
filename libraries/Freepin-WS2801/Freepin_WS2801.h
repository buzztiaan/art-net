#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

// Not all LED pixels are RGB order; 36mm type expects GRB data.
// Optional flag to constructors indicates data order (default if
// unspecified is RGB).  As long as setPixelColor/getPixelColor are
// used, other code can always treat 'packed' colors as RGB; the
// library will handle any required translation internally.
#define WS2801_RGB 0
#define WS2801_GRB 1

class Freepin_WS2801 {

 public:

  // Configurable pins:
  Freepin_WS2801(uint16_t n, uint8_t dpin, uint8_t cpin, uint8_t order=WS2801_RGB);
  // Release memory (as needed):
  ~Freepin_WS2801();

  void
    begin(void),
    show(void),
    setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
    setPixelColor(uint16_t n, uint32_t c),
    setPixelColor(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b),
    setPixelColor(uint16_t x, uint16_t y, uint32_t c),
    updatePins(uint8_t dpin, uint8_t cpin), // Change pins, configurable
    updateLength(uint16_t n), // Change strand length
    updateOrder(uint8_t order); // Change data order
  uint16_t
    numPixels(void);
  uint32_t
    getPixelColor(uint16_t n);

 private:

  uint16_t
    numLEDs,
    width,     // used with matrix mode
    height;    // used with matrix mode
  uint8_t
    *pixels,   // Holds color values for each LED (3 bytes each)
    rgb_order, // Color order; RGB vs GRB (or others, if needed in future)
    clkpin    , datapin,     // Clock & data pin numbers
    clkpinmask, datapinmask; // Clock & data PORT bitmasks
  volatile uint32_t
    *clkport  , *dataport;   // Clock & data PORT registers
  void
    alloc(uint16_t n);
  boolean
    hardwareSPI, // If 'true', using hardware SPI
    begun;       // If 'true', begin() method was previously invoked
};
