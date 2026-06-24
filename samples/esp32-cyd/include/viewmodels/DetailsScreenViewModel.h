#pragma once

#include <vector>

#include "usecases/BleUseCases.h"
#include "viewmodels/ViewModel.h"
#include "NavigationController.h"

struct DetailsScreenState
{
  char title[24];
};

class DetailsScreenViewModel : public ViewModelAbstract<DetailsScreenState>
{
private:
  NavigationController &navigationController;

public:
  explicit DetailsScreenViewModel(NavigationController &navigationController);

  void begin(void* args) override;
  void end() override;
  void onBack();
};