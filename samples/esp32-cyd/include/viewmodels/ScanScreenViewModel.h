#pragma once

#include <vector>

#include "usecases/BleUseCases.h"
#include "viewmodels/ViewModel.h"

struct UiLabel
{
  char title[48];
  char subtitle[24];
};

struct ScanScreenViewState
{
  std::vector<UiLabel> items;
  uint16_t listOffset = 0;
};

class ScanScreenViewModel : public BleScanner::Listener
{
private:
  StartScanUseCase &startScanUseCase;
  StopScanUseCase &stopScanUseCase;

public:
  explicit ScanScreenViewModel(
      ViewModel &viewModel,
      StartScanUseCase &startScanUseCase,
      StopScanUseCase &stopScanUseCase);

  ScanScreenViewState getStateCopy() const;

  void scrollDown();
  void scrollUp();
  void onDeviceSelected();
  void onDevicesScanned(const std::vector<BleScanner::ScanResult> &results) override;
  void setObserver(ViewModel::Observer *observer);
  void begin();
  void end();

private:
  ViewModel &viewModel_;
  ScanScreenViewState uiState_{};
  SemaphoreHandle_t stateMutex_ = nullptr;
  ViewModel::Observer *observer_ = nullptr;
};