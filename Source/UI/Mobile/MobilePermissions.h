#pragma once

#include "../../JuceHeader.h"
#include <functional>

namespace MobilePermissions
{
    // Asks for microphone access if needed. The callback runs on the message
    // thread with the result.
    void requestMicrophone(std::function<void(bool granted)> callback);
}
