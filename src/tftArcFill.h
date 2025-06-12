#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

extern TFT_eSPI tft;

#define DEG2RAD 0.0174532925
#define LOOP_DELAY 10 // Loop delay to slow things down
#define MAX_SENSOR 2

typedef struct
{
  uint16_t xpos; 
  uint16_t ypos; 
  uint8_t radius; 
  int16_t value; 
  int16_t sp;
} GrafDispl;

extern GrafDispl grafDispl[MAX_SENSOR];

typedef struct {
  int16_t pvT;
  uint8_t err;
} Ds;

extern Ds ds[MAX_SENSOR];

void initArcFill(void);
void fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour);
unsigned int rainbow(byte value);
void diagram(GrafDispl grafDispl, uint16_t color);
