#pragma once

#include <vector>

#include "usecases/BleUseCases.h"
#include "viewmodels/ViewModel.h"
#include "NavigationController.h"
// #include "ble/JkBmsSourceBLEArduinoImpl.h"
#include "JkBmsController.h"

struct DetailsScreenState
{
  char title[24];
};

class DetailsScreenViewModel : public ViewModelAbstract<DetailsScreenState>
{
private:
  NavigationController &navigationController;
  // JkBms::Source* jkbmsSource;
  JkBmsCpp::Controller jkbmsController;

public:
  explicit DetailsScreenViewModel(NavigationController &navigationController);

  void begin(void* args) override;
  void end() override;
  void onBack();
};