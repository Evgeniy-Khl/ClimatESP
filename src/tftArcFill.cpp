#include <tftArcFill.h>

extern GrafDispl grafDispl[4];
extern int16_t t[4];
byte inc = 0;
unsigned int col = 0;

byte red = 31; // Red is the top 5 bits of a 16-bit colour value
byte green = 0;// Green is the middle 6 bits
byte blue = 0; // Blue is the bottom 5 bits
byte state = 0;
uint16_t xpos, ypos;

void initArcFill(){
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);

    /* xpos = radius+5; ypos = radius+5; 
    fillArc(xpos, ypos, 240, 10, radius, radius, seg_w, TFT_BLUE);
    fillArc(xpos, ypos, 300, 10, radius, radius, seg_w, TFT_GREEN);
    fillArc(xpos, ypos, 0,   10, radius, radius, seg_w, TFT_YELLOW);
    fillArc(xpos, ypos, 60,  10, radius, radius, seg_w, TFT_RED);

    fillArc(xpos, ypos, 358, 1, radius-10, radius-10, radius-20, TFT_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    // tft.fillRect(0, 0, 320, 30, TFT_BLUE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("67C", xpos, ypos+10, 4);
    //------------------------------------------------------------------------
    xpos = tft.width()-radius-5; ypos = radius+5; 
    fillArc(xpos, ypos, 240, 10, radius, radius, seg_w, TFT_BLUE);
    fillArc(xpos, ypos, 300, 10, radius, radius, seg_w, TFT_GREEN);
    fillArc(xpos, ypos, 0,   10, radius, radius, seg_w, TFT_YELLOW);
    fillArc(xpos, ypos, 60,  10, radius, radius, seg_w, TFT_RED);

    fillArc(xpos, ypos, 348, 1, radius-10, radius-10, radius-20, TFT_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    // tft.fillRect(0, 0, 320, 30, TFT_BLUE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("57C", xpos, ypos+10, 4);
    //------------------------------------------------------------------------
    xpos = radius+5; ypos = tft.height()-radius-5; 
    fillArc(xpos, ypos, 240, 10, radius, radius, seg_w, TFT_BLUE);
    fillArc(xpos, ypos, 300, 10, radius, radius, seg_w, TFT_GREEN);
    fillArc(xpos, ypos, 0,   10, radius, radius, seg_w, TFT_YELLOW);
    fillArc(xpos, ypos, 60,  10, radius, radius, seg_w, TFT_RED);

    fillArc(xpos, ypos, 358, 1, radius-10, radius-10, radius-20, TFT_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    // tft.fillRect(0, 0, 320, 30, TFT_BLUE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("67%", xpos, ypos+10, 4);
    //------------------------------------------------------------------------
    xpos = tft.width()-radius-5; ypos = tft.height()-radius-5; 
    fillArc(xpos, ypos, 240, 10, radius, radius, seg_w, TFT_BLUE);
    fillArc(xpos, ypos, 300, 10, radius, radius, seg_w, TFT_GREEN);
    fillArc(xpos, ypos, 0,   10, radius, radius, seg_w, TFT_YELLOW);
    fillArc(xpos, ypos, 60,  10, radius, radius, seg_w, TFT_RED);

    fillArc(xpos, ypos, 28, 1, radius-10, radius-10, radius-20, TFT_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    // tft.fillRect(0, 0, 320, 30, TFT_BLUE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("77%", xpos, ypos+10, 4); */
    //------------------------------------------------------------------------
    //diagram(byte sector, byte radius, int value, int greenValue, int yeeloValue, int redValue)
    diagram(grafDispl[0]);
    diagram(grafDispl[1]);
    diagram(grafDispl[2]);
    diagram(grafDispl[3]);

}

void loopArcFill(){

  // Continuous elliptical arc drawing
  fillArc(xpos, ypos, inc * 6, 1, 140, 100, 10, rainbow(col));

  // Continuous segmented (inc*2) elliptical arc drawing
  fillArc(xpos, ypos, ((inc * 2) % 60) * 6, 1, ypos, 80, 30, rainbow(col));

  // Circle drawing using arc with arc width = radius
  fillArc(xpos, ypos, inc * 6, 1, 42, 42, 42, rainbow(col));

  inc++;
  col += 1;
  if (col > 191) col = 0;
  if (inc > 59) inc = 0;

  delay(LOOP_DELAY);
}

// #########################################################################
// Draw a circular or elliptical arc with a defined thickness
// #########################################################################

// x,y == coords of centre of arc
// start_angle = 0 - 359
// seg_count = number of 6 degree segments to draw (60 => 360 degree arc)
// rx = x axis outer radius
// ry = y axis outer radius
// w  = width (thickness) of arc in pixels
// colour = 16-bit colour value
// Note if rx and ry are the same then an arc of a circle is drawn

void fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
{

  byte seg = 6; // Segments are 3 degrees wide = ypos segments for 360 degrees
  byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Calculate first pair of coordinates for seg_w start
  float sx = cos((start_angle - 210) * DEG2RAD);
  float sy = sin((start_angle - 210) * DEG2RAD);
  uint16_t x0 = sx * (rx - w) + x;
  uint16_t y0 = sy * (ry - w) + y;
  uint16_t x1 = sx * rx + x;
  uint16_t y1 = sy * ry + y;

  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {

    // Calculate pair of coordinates for seg_w end
    float sx2 = cos((i + seg - 210) * DEG2RAD);
    float sy2 = sin((i + seg - 210) * DEG2RAD);
    int x2 = sx2 * (rx - w) + x;
    int y2 = sy2 * (ry - w) + y;
    int x3 = sx2 * rx + x;
    int y3 = sy2 * ry + y;

    tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

    // Copy seg_w end to seg_w start for next seg_w
    x0 = x2;
    y0 = y2;
    x1 = x3;
    y1 = y3;
  }
}

//#########################################################################
void diagram(GrafDispl grafDispl){
    char tempStr[10]; // Буфер для строки температуры
    byte seg_w = 20;
    long tmpval0,tmpval1, maxtemp, mintemp;
    switch (grafDispl.sector)
    {
    case 0: xpos = grafDispl.radius+5; ypos = grafDispl.radius+5; break;
    case 1: xpos = tft.width()-grafDispl.radius-5; ypos = grafDispl.radius+5; break;
    case 2: xpos = grafDispl.radius+5; ypos = tft.height()-grafDispl.radius-5; break;
    case 3: xpos = tft.width()-grafDispl.radius-5; ypos = tft.height()-grafDispl.radius-5; break;
    default: xpos = grafDispl.radius+5; ypos = grafDispl.radius+5; break;
    }
    maxtemp = grafDispl.redValue + grafDispl.redValue/5;
    mintemp = grafDispl.greenValue - grafDispl.greenValue/2;
    tmpval1 = map(grafDispl.greenValue, mintemp,maxtemp,0,240);
    tmpval0 = tmpval1-5;
    fillArc(xpos, ypos, 0, tmpval1/6, grafDispl.radius, grafDispl.radius, seg_w, TFT_BLUE);
    tmpval1 = map(grafDispl.yellowValue, mintemp,maxtemp,0,240);
    fillArc(xpos, ypos, tmpval0, (tmpval1-tmpval0)/6, grafDispl.radius, grafDispl.radius, seg_w, TFT_GREEN);
    tmpval0 = tmpval1-5;
    tmpval1 = map(grafDispl.redValue, mintemp,maxtemp,0,240);
    fillArc(xpos, ypos, tmpval0, (tmpval1-tmpval0)/6, grafDispl.radius, grafDispl.radius, seg_w, TFT_YELLOW);
    tmpval0 = tmpval1-5;
    tmpval1 = map(maxtemp, mintemp,maxtemp,0,240);
    fillArc(xpos, ypos, tmpval0, (tmpval1-tmpval0)/6, grafDispl.radius, grafDispl.radius, seg_w, TFT_RED);

    fillArc(xpos, ypos, 0, 40, grafDispl.radius-20, grafDispl.radius-20, grafDispl.radius/2, TFT_BLACK);
    tmpval0 = grafDispl.value*10;
    if(tmpval0 < mintemp) tmpval0 = mintemp;
    else if(tmpval0 > maxtemp) tmpval0 = maxtemp;
    tmpval1 = map(tmpval0, mintemp,maxtemp,0,240);
    fillArc(xpos, ypos, tmpval1, 1, grafDispl.radius-10, grafDispl.radius-10, grafDispl.radius/2, TFT_WHITE);
    
    tft.setTextSize(1);
    tft.fillRect(xpos-25, ypos-2, 50, 25, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    // dtostrf(grafDispl.value, 3, 0, tempStr);
    itoa(grafDispl.value, tempStr, 10); // Преобразовать в десятичную строку
    strcat(tempStr, " C");
    tft.setTextColor(TFT_WHITE);
    tft.drawString(tempStr, xpos, ypos+10, 4);
}

// #########################################################################
// Return the 16-bit colour with brightness 0-100%
// #########################################################################
unsigned int brightness(unsigned int colour, int brightness)
{
  byte red   = colour >> 11;
  byte green = (colour & 0x7E0) >> 5;
  byte blue  = colour & 0x1F;

  blue =  (blue * brightness) / 100;
  green = (green * brightness) / 100;
  red =   (red * brightness) / 100;

  return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// Return a 16-bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  switch (state) {
    case 0:
      green ++;
      if (green == 64) {
        green = 63;
        state = 1;
      }
      break;
    case 1:
      red--;
      if (red == 255) {
        red = 0;
        state = 2;
      }
      break;
    case 2:
      blue ++;
      if (blue == 32) {
        blue = 31;
        state = 3;
      }
      break;
    case 3:
      green --;
      if (green == 255) {
        green = 0;
        state = 4;
      }
      break;
    case 4:
      red ++;
      if (red == 32) {
        red = 31;
        state = 5;
      }
      break;
    case 5:
      blue --;
      if (blue == 255) {
        blue = 0;
        state = 0;
      }
      break;
  }
  return red << 11 | green << 5 | blue;
}