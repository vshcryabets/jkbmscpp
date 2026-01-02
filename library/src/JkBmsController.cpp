#include "JkBmsController.h"

namespace JkBmsCpp
{
    
JkBmsController::JkBmsController()
{
}

JkBmsController::~JkBmsController()
{
}

void JkBmsController::start(JkBmsSource* source)
{
    this->source = source;
    this->source->connect();
}

void JkBmsController::end()
{
    if (this->source)
    {
        this->source->disconnect();
        this->source = nullptr;
    }
}

} // namespace JkBmsCpp