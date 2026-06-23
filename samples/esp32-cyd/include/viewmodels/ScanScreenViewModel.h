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

struct ScanScreenState
{
  uint16_t itemCount = 0;
  uint16_t listOffset = 0;
  const ItempProvider *itemProvider = nullptr;
};

class ScanScreenViewModel : public ViewModelAbstract<ScanScreenState>,
                            public BleScanner::Listener,
                            public ItempProvider
{
private:
  StartScanUseCase &startScanUseCase;
  StopScanUseCase &stopScanUseCase;
  std::vector<BleScanner::ScanResult> items;

public:
  explicit ScanScreenViewModel(
      StartScanUseCase &startScanUseCase,
      StopScanUseCase &stopScanUseCase);

  void scrollDown();
  void scrollUp();
  void onDeviceSelected();
  void onDevicesScanned(const std::vector<BleScanner::ScanResult> &results) override;
  void begin() override;
  void end() override;

  void getItem(int index, UiLabel &out) const override;
};