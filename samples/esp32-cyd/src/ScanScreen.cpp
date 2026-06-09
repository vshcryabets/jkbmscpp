#include "ScanScreen.h"

void DrawScanScreen(Display& display, const State& state)
{
  display.tft->fillScreen(TFT_WHITE);
  display.tft->setTextColor(TFT_BLACK, TFT_WHITE);
  display.tft->drawString("Scanning...", 16, 12, 2);
}