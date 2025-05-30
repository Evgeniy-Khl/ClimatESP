#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

typedef struct
{
  uint8_t sector; 
  uint8_t radius; 
  int16_t value; 
  int16_t greenValue; 
  int16_t yellowValue; 
  int16_t redValue;
} GrafDispl;

extern TFT_eSPI tft;

#define DEG2RAD 0.0174532925
#define LOOP_DELAY 10 // Loop delay to slow things down

void initArcFill();
void loopArcFill();
void fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour);
unsigned int rainbow(byte value);
void diagram(GrafDispl grafDispl);