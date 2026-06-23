#pragma once

#include "TouchController.h"
#include "Display.h"
#include "viewmodels/ViewModel.h"

class InputController: public TouchController::Listener {
public:
    virtual ~InputController() = default;
    virtual bool consumeUiUpdateFlag() = 0;
};

class Screen{
public:
  virtual void begin() = 0;
  virtual void end() = 0;
  virtual void draw() = 0;
  virtual InputController* getTouchHandler() = 0;
};