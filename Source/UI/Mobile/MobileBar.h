#pragma once

#include "../../JuceHeader.h"
#include "../../Utils/UI/Theme.h"
#include "../../Utils/Localization.h"
#include "../Components/AppFont.h"
#include <functional>

/**
 * Touch-sized action bar shown above the toolbar in the mobile build. It
 * replaces the desktop menu bar: the common file actions get their own
 * buttons and everything else stays reachable through the Menu button.
 */
class MobileBar : public juce::Component {
public:
  MobileBar();

  void paint(juce::Graphics &g) override;
  void resized() override;

  // Switches the record button between Record and Stop and shows the
  // elapsed time and input level while recording.
  void setRecording(bool isRecording);
  void setRecordingStatus(double elapsedSeconds, float peakLevel);
  void setProjectActionsEnabled(bool enabled);
  void refreshText();

  juce::Component &getMenuButton() { return menuButton; }

  std::function<void()> onRecord;
  std::function<void()> onOpen;
  std::function<void()> onSave;
  std::function<void()> onShare;
  std::function<void()> onMenu;

  static constexpr int kHeight = 44;

private:
  void styleButton(juce::TextButton &button, juce::Colour colour);

  juce::TextButton recordButton;
  juce::TextButton openButton;
  juce::TextButton saveButton;
  juce::TextButton shareButton;
  juce::TextButton menuButton;

  bool recording = false;
  double elapsed = 0.0;
  float level = 0.0f;
  juce::Rectangle<int> statusArea;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MobileBar)
};
