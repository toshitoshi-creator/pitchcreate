#pragma once

#include "../../JuceHeader.h"
#include <atomic>
#include <memory>

/**
 * Records the first input channel of an AudioDeviceManager to a mono WAV file.
 *
 * Used by the mobile UI to capture a vocal take that is then opened in the
 * editor. The device manager is shared with playback, so start() switches its
 * first input channel on and stop() switches it back off again.
 */
class MicRecorder : private juce::AudioIODeviceCallback {
public:
  explicit MicRecorder(juce::AudioDeviceManager &deviceManager);
  ~MicRecorder() override;

  // Returns an empty string on success, otherwise a description of the error.
  juce::String start(const juce::File &targetFile);

  // Stops recording and returns the finished file (empty if nothing was
  // recording).
  juce::File stop();

  bool isRecording() const { return recording.load(); }
  double getElapsedSeconds() const;
  // Peak input level since the last call, 0..1. Called from the UI thread.
  float takePeakLevel() { return peakLevel.exchange(0.0f); }

private:
  void audioDeviceIOCallbackWithContext(
      const float *const *inputChannelData, int numInputChannels,
      float *const *outputChannelData, int numOutputChannels, int numSamples,
      const juce::AudioIODeviceCallbackContext &context) override;
  void audioDeviceAboutToStart(juce::AudioIODevice *device) override;
  void audioDeviceStopped() override;

  void restoreDeviceSetup();

  juce::AudioDeviceManager &deviceManager;
  juce::TimeSliceThread writerThread{"PitchNet mic writer"};
  std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
  juce::CriticalSection writerLock;
  std::atomic<juce::AudioFormatWriter::ThreadedWriter *> activeWriter{nullptr};

  juce::File currentFile;
  std::atomic<bool> recording{false};
  std::atomic<juce::int64> samplesWritten{0};
  std::atomic<float> peakLevel{0.0f};
  double sampleRate = 0.0;
  bool inputWasEnabled = false;
  juce::BigInteger previousInputChannels;
  bool previousUseDefaultInputs = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MicRecorder)
};
