#pragma once

#include <vector>

#include "usecases/BleUseCases.h"
#include "viewmodels/ViewModel.h"

struct UiLabel
{
  char title[48];
  char subtitle[24];
};

class ItempProvider
{
public:
  virtual void getItem(int index, UiLabel &out) const = 0;
};

struct ScanScreenViewState
{
  uint16_t itemCount = 0;
  uint16_t listOffset = 0;
  const ItempProvider *itemProvider = nullptr;
};

class ScanScreenViewModel : public BleScanner::Listener, public ItempProvider
{
private:
  StartScanUseCase &startScanUseCase;
  StopScanUseCase &stopScanUseCase;
  std::vector<BleScanner::ScanResult> items;
  ScanScreenViewState uiState{};
  SemaphoreHandle_t stateMutex = nullptr;
  ViewModel::Observer *observer = nullptr;

public:
  explicit ScanScreenViewModel(
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

  void getItem(int index, UiLabel &out) const override;
};