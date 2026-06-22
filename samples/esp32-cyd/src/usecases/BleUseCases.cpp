#include "usecases/BleUseCases.h"

void StartScanUseCaseImpl::execute(
    uint8_t scanTimeSeconds,
    uint8_t scanPeriodSeconds,
    BleScanner::Listener *listener)
{
    bleScanner.startScan(scanTimeSeconds, scanPeriodSeconds, listener);
}

void StopScanUseCaseImpl::execute()
{
    bleScanner.stopScan();
}