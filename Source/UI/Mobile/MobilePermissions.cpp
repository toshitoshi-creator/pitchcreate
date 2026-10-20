#include "MobilePermissions.h"

// iOS uses AVAudioSession directly (MobilePermissions.mm); JUCE's
// RuntimePermissions reports every permission as granted there.
#if !JUCE_IOS

namespace MobilePermissions
{
    void requestMicrophone(std::function<void(bool granted)> callback)
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
                                          [callback](bool granted)
                                          {
                                              if (callback)
                                                  callback(granted);
                                          });
    }
}

#endif
