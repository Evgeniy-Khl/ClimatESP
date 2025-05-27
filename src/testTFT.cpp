#include "ili9341.h"

void initTFT(void){
  Serial.println("ILI9341 & XPT2046 Touch Test");

  // Инициализация SPI (не нужна явно, т.к. tft.begin() и ts.begin() сделают это)

  // Инициализация дисплея
  tft.begin();
  tft.setRotation(SCREEN_ROTATION);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(10, 10);
  tft.println("Touch Screen Test");
  tft.setTextSize(1);
  tft.println("Натиснiть на екран!"); // Press the screen! (Ukrainian)

  // Инициализация тачскрина
  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    tft.setTextColor(ILI9341_RED);
    tft.println("Touchscreen FAILED!");
    while (1) delay(10); // Остановка, если тач не работает
  }
  ts.setRotation(SCREEN_ROTATION); // Устанавливаем ту же ориентацию!
  Serial.println("Touchscreen started.");
  tft.println("Touchscreen OK.");
}

void touchTest(){
    TS_Point p = ts.getPoint(); // Получаем точку касания

    // Выводим "сырые" координаты в Serial порт
    Serial.print("Raw X = "); Serial.print(p.x);
    Serial.print(", Y = "); Serial.print(p.y);
    Serial.print(", Pressure Z = "); Serial.println(p.z);

    // --- Преобразование "сырых" координат в координаты экрана ---
    // Используем функцию map(). ВАЖНО: TS_MIN/MAX нужно откалибровать!
    // Также учтите, что оси X/Y могут быть инвертированы или поменяны местами
    // в зависимости от ориентации и вашего экрана. Этот код - лишь пример!
    int screenX = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    int screenY = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

    // Иногда оси нужно инвертировать:
    // screenX = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    // screenY = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);

    Serial.print("Mapped X = "); Serial.print(screenX);
    Serial.print(", Y = "); Serial.println(screenY);

    // Рисуем точку (маленький круг) на экране в месте касания
    tft.fillCircle(screenX, screenY, 3, ILI9341_RED);
}