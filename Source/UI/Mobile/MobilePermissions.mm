#include "MobilePermissions.h"

#if JUCE_IOS
#import <AVFoundation/AVFoundation.h>

namespace MobilePermissions
{
    void requestMicrophone(std::function<void(bool granted)> callback)
    {
        auto deliver = [callback](bool granted)
        {
            juce::MessageManager::callAsync([callback, granted]
                                            {
                                                if (callback)
                                                    callback(granted);
                                            });
        };

        [[AVAudioSession sharedInstance] requestRecordPermission:^(BOOL granted) {
            deliver(granted == YES);
        }];
    }
}

#endif
