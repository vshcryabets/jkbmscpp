#pragma once

#include <TFT_eSPI.h>

class Display {
public:
    TFT_eSPI* tft;
public:
    Display(TFT_eSPI* tft) : tft(tft) {}
};