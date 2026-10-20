/*
    JUCE Header - Auto-generated header file for including all JUCE modules
*/

#pragma once

// Prevent Windows min/max macros from conflicting with std::min/std::max
#ifdef _WIN32
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
#endif

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

// Touch-first layout (action bar instead of the menu bar, recording, sharing
// exports). On by default for phones and tablets; a desktop build can force it
// with -DPITCHNET_FORCE_MOBILE_UI=ON to try the layout without a device.
#ifndef PITCHNET_MOBILE_UI
  #if JUCE_IOS || JUCE_ANDROID
    #define PITCHNET_MOBILE_UI 1
  #else
    #define PITCHNET_MOBILE_UI 0
  #endif
#endif
