#include "viewmodels/DetailsScreenViewModel.h"

#include "screens/UiMappers.h"

DetailsScreenViewModel::DetailsScreenViewModel(
    NavigationController &navigationController)
    : ViewModelAbstract<DetailsScreenState>(),
      navigationController(navigationController)
{
}

void DetailsScreenViewModel::begin(void* args)
{
    DetailsScreenArgs* detailsArgs = reinterpret_cast<DetailsScreenArgs*>(args);
    withStateLock([&detailsArgs](DetailsScreenState &state) {
        macAddressToString(detailsArgs->macAddress, state.title, sizeof(state.title));
    });
}

void DetailsScreenViewModel::end()
{
}

void DetailsScreenViewModel::onBack()
{
    navigationController.navigateToScanScreen();
}
