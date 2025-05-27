#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

// --- Пины WEMOS D1R1 для Дисплея (ILI9341) ---
#define TFT_CS   15  // D10 (GPIO15)
#define TFT_DC   16  // D2 (GPIO16)
#define TFT_RST  3   // D0 (GPIO3)
#define TOUCH_CS 0   // D8 (GPIO0) - Внимание! GPIO0 влияет на режим загрузки!
//#define TIRQ_PIN 16  // D2 (GPIO16) - Опционально, для прерываний

// --- Константы для калибровки/маппинга ---
// ВАЖНО: Эти значения ПРИМЕРНЫЕ и сильно зависят от вашего экрана и его ориентации.
// Вам НУЖНО провести калибровку для точного соответствия!
// Это минимальные/максимальные значения, которые возвращает ts.getPoint()
#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

// Устанавливаем ориентацию экрана (должна совпадать с tft.setRotation!)
#define SCREEN_ROTATION 0 // 1 = Ландшафт

extern Adafruit_ILI9341 tft;
extern XPT2046_Touchscreen ts;

void initTFT(void);
void touchTest();