#include "MicRecorder.h"

MicRecorder::MicRecorder(juce::AudioDeviceManager &manager)
    : deviceManager(manager) {
  writerThread.startThread();
}

MicRecorder::~MicRecorder() {
  stop();
  writerThread.stopThread(2000);
}

juce::String MicRecorder::start(const juce::File &targetFile) {
  if (recording.load())
    return "Already recording";

  // Turn on the first input channel of the shared device. Playback keeps
  // running through its own callback.
  auto setup = deviceManager.getAudioDeviceSetup();
  previousInputChannels = setup.inputChannels;
  previousUseDefaultInputs = setup.useDefaultInputChannels;
  inputWasEnabled = !setup.inputChannels.isZero();

  setup.useDefaultInputChannels = false;
  setup.inputChannels.clear();
  setup.inputChannels.setBit(0);
  auto error = deviceManager.setAudioDeviceSetup(setup, true);
  if (error.isNotEmpty()) {
    restoreDeviceSetup();
    return error;
  }

  auto *device = deviceManager.getCurrentAudioDevice();
  if (device == nullptr || device->getActiveInputChannels().isZero()) {
    restoreDeviceSetup();
    return "No microphone input is available";
  }

  sampleRate = device->getCurrentSampleRate();
  if (sampleRate <= 0.0) {
    restoreDeviceSetup();
    return "The audio device reported no sample rate";
  }

  targetFile.getParentDirectory().createDirectory();
  targetFile.deleteFile();

  std::unique_ptr<juce::OutputStream> stream =
      std::make_unique<juce::FileOutputStream>(targetFile);
  if (!static_cast<juce::FileOutputStream *>(stream.get())->openedOk()) {
    restoreDeviceSetup();
    return "Could not create " + targetFile.getFullPathName();
  }

  juce::WavAudioFormat wav;
  auto writer = wav.createWriterFor(stream, juce::AudioFormatWriterOptions{}
                                                .withSampleRate(sampleRate)
                                                .withNumChannels(1)
                                                .withBitsPerSample(24));
  if (writer == nullptr) {
    restoreDeviceSetup();
    return "Could not create a WAV writer";
  }

  {
    const juce::ScopedLock sl(writerLock);
    threadedWriter = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(
        writer.release(), writerThread, 32768);
    activeWriter.store(threadedWriter.get());
  }

  currentFile = targetFile;
  samplesWritten.store(0);
  peakLevel.store(0.0f);
  recording.store(true);
  deviceManager.addAudioCallback(this);
  return {};
}

juce::File MicRecorder::stop() {
  if (!recording.exchange(false))
    return {};

  deviceManager.removeAudioCallback(this);

  {
    const juce::ScopedLock sl(writerLock);
    activeWriter.store(nullptr);
    // Destroying the ThreadedWriter flushes the remaining samples and closes
    // the file.
    threadedWriter.reset();
  }

  restoreDeviceSetup();
  return currentFile;
}

double MicRecorder::getElapsedSeconds() const {
  return sampleRate > 0.0
             ? static_cast<double>(samplesWritten.load()) / sampleRate
             : 0.0;
}

void MicRecorder::restoreDeviceSetup() {
  if (inputWasEnabled)
    return;

  auto setup = deviceManager.getAudioDeviceSetup();
  setup.inputChannels = previousInputChannels;
  setup.useDefaultInputChannels = previousUseDefaultInputs;
  deviceManager.setAudioDeviceSetup(setup, true);
}

void MicRecorder::audioDeviceIOCallbackWithContext(
    const float *const *inputChannelData, int numInputChannels,
    float *const *outputChannelData, int numOutputChannels, int numSamples,
    const juce::AudioIODeviceCallbackContext &context) {
  juce::ignoreUnused(context);

  // This callback only records; the playback callback fills the outputs.
  for (int ch = 0; ch < numOutputChannels; ++ch)
    if (outputChannelData[ch] != nullptr)
      juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);

  if (numInputChannels <= 0 || inputChannelData[0] == nullptr)
    return;

  const juce::ScopedTryLock sl(writerLock);
  if (!sl.isLocked())
    return;

  auto *writer = activeWriter.load();
  if (writer == nullptr)
    return;

  const float *mono[] = {inputChannelData[0]};
  if (writer->write(mono, numSamples))
    samplesWritten.fetch_add(numSamples);

  const auto range =
      juce::FloatVectorOperations::findMinAndMax(inputChannelData[0], numSamples);
  const float peak = std::max(std::abs(range.getStart()), std::abs(range.getEnd()));
  float previous = peakLevel.load();
  while (peak > previous && !peakLevel.compare_exchange_weak(previous, peak)) {
  }
}

void MicRecorder::audioDeviceAboutToStart(juce::AudioIODevice *device) {
  if (device != nullptr)
    sampleRate = device->getCurrentSampleRate();
}

void MicRecorder::audioDeviceStopped() {}
