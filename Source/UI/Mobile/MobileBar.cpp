#include "MobileBar.h"

MobileBar::MobileBar() {
  styleButton(recordButton, juce::Colour(0xFFD83A3Au));
  styleButton(openButton, APP_COLOR_SURFACE_RAISED);
  styleButton(saveButton, APP_COLOR_SURFACE_RAISED);
  styleButton(shareButton, APP_COLOR_SURFACE_RAISED);
  styleButton(menuButton, APP_COLOR_SURFACE_RAISED);

  recordButton.onClick = [this] { if (onRecord) onRecord(); };
  openButton.onClick = [this] { if (onOpen) onOpen(); };
  saveButton.onClick = [this] { if (onSave) onSave(); };
  shareButton.onClick = [this] { if (onShare) onShare(); };
  menuButton.onClick = [this] { if (onMenu) onMenu(); };

  refreshText();
  setProjectActionsEnabled(false);
}

void MobileBar::styleButton(juce::TextButton &button, juce::Colour colour) {
  button.setColour(juce::TextButton::buttonColourId, colour);
  button.setColour(juce::TextButton::buttonOnColourId, colour.brighter(0.2f));
  button.setColour(juce::TextButton::textColourOffId, APP_COLOR_TEXT_PRIMARY);
  button.setColour(juce::TextButton::textColourOnId, APP_COLOR_TEXT_PRIMARY);
  button.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
  button.setWantsKeyboardFocus(false);
  addAndMakeVisible(button);
}

void MobileBar::refreshText() {
  recordButton.setButtonText(recording ? TR("mobile.stop_recording")
                                       : TR("mobile.record"));
  openButton.setButtonText(TR("mobile.open"));
  saveButton.setButtonText(TR("mobile.save"));
  shareButton.setButtonText(TR("mobile.share"));
  menuButton.setButtonText(TR("mobile.menu"));
}

void MobileBar::setRecording(bool isRecording) {
  recording = isRecording;
  elapsed = 0.0;
  level = 0.0f;
  openButton.setEnabled(!recording);
  menuButton.setEnabled(!recording);
  refreshText();
  repaint();
}

void MobileBar::setRecordingStatus(double elapsedSeconds, float peakLevel) {
  elapsed = elapsedSeconds;
  // Let the meter fall back gradually instead of flickering per block.
  level = std::max(peakLevel, level * 0.8f);
  repaint(statusArea);
}

void MobileBar::setProjectActionsEnabled(bool enabled) {
  saveButton.setEnabled(enabled);
  shareButton.setEnabled(enabled);
}

void MobileBar::paint(juce::Graphics &g) {
  g.fillAll(APP_COLOR_BACKGROUND);

  if (!recording || statusArea.isEmpty())
    return;

  auto area = statusArea.toFloat().reduced(4.0f, 8.0f);
  const int seconds = static_cast<int>(elapsed);
  const auto time = juce::String::formatted("%d:%02d", seconds / 60, seconds % 60);

  g.setColour(juce::Colour(0xFFD83A3Au));
  g.fillEllipse(area.removeFromLeft(10.0f).withSizeKeepingCentre(10.0f, 10.0f));
  area.removeFromLeft(6.0f);

  g.setColour(APP_COLOR_TEXT_PRIMARY);
  g.setFont(AppFont::getBoldFont(15.0f));
  g.drawText(TR("mobile.recording") + "  " + time, area.removeFromLeft(130.0f),
             juce::Justification::centredLeft);

  auto meter = area.reduced(0.0f, 8.0f);
  g.setColour(APP_COLOR_SURFACE_RAISED);
  g.fillRoundedRectangle(meter, 3.0f);
  const float clamped = juce::jlimit(0.0f, 1.0f, level);
  g.setColour(clamped > 0.9f ? juce::Colour(0xFFD83A3Au) : APP_COLOR_PRIMARY);
  g.fillRoundedRectangle(meter.withWidth(meter.getWidth() * clamped), 3.0f);
}

void MobileBar::resized() {
  auto bounds = getLocalBounds().reduced(6, 4);
  constexpr int gap = 6;
  const int buttonWidth = juce::jlimit(64, 96, bounds.getWidth() / 8);

  recordButton.setBounds(bounds.removeFromLeft(buttonWidth));
  bounds.removeFromLeft(gap);
  openButton.setBounds(bounds.removeFromLeft(buttonWidth));
  bounds.removeFromLeft(gap);
  saveButton.setBounds(bounds.removeFromLeft(buttonWidth));
  bounds.removeFromLeft(gap);
  shareButton.setBounds(bounds.removeFromLeft(buttonWidth));

  menuButton.setBounds(bounds.removeFromRight(buttonWidth));
  bounds.removeFromRight(gap);
  statusArea = bounds.reduced(gap, 0);
}
