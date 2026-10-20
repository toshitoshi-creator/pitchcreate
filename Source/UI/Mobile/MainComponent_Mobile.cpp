// Mobile-only parts of MainComponent: the action bar, microphone recording,
// importing picked documents into the sandbox and sharing exports.

#include "../MainComponent.h"

#if PITCHNET_MOBILE_UI

#include "../Components/StyledMessageBox.h"
#include "MobilePermissions.h"

juce::File MainComponent::getMobileFolder(const juce::String &name)
{
  // Documents is visible in the Files app because the bundle sets
  // UIFileSharingEnabled / LSSupportsOpeningDocumentsInPlace.
  auto folder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                    .getChildFile(name);
  folder.createDirectory();
  return folder;
}

void MainComponent::setupMobileUi()
{
  addAndMakeVisible(mobileBar);

  mobileBar.onRecord = [this] { toggleMobileRecording(); };
  mobileBar.onOpen = [this] { openFile(); };
  mobileBar.onSave = [this] { saveProject(); };
  mobileBar.onShare = [this]
  {
    if (commandManager)
      commandManager->invokeDirectly(CommandIDs::exportAudio, true);
  };
  mobileBar.onMenu = [this] { showMobileMenu(); };
}

void MainComponent::updateMobileBar()
{
  if (micRecorder && micRecorder->isRecording())
    mobileBar.setRecordingStatus(micRecorder->getElapsedSeconds(),
                                 micRecorder->takePeakLevel());

  const auto *project = getProject();
  mobileBar.setProjectActionsEnabled(
      project != nullptr && project->getAudioData().waveform.getNumSamples() > 0);
}

void MainComponent::showMobileMenu()
{
  if (!menuHandler)
    return;

  // Rebuild the desktop menu bar as one popup with a submenu per top-level
  // menu. Command items invoke themselves; the rest (recent files, language)
  // go back through the model with the index of the menu they came from.
  juce::PopupMenu menu;
  juce::Array<juce::PopupMenu> subMenus;
  const auto names = menuHandler->getMenuBarNames();
  for (int i = 0; i < names.size(); ++i)
  {
    auto subMenu = menuHandler->getMenuForIndex(i, names[i]);
    menu.addSubMenu(names[i], subMenu);
    subMenus.add(subMenu);
  }

  juce::Component::SafePointer<MainComponent> safeThis(this);
  menu.showMenuAsync(
      juce::PopupMenu::Options()
          .withTargetComponent(&mobileBar.getMenuButton())
          .withStandardItemHeight(36),
      [safeThis, subMenus](int result)
      {
        if (safeThis == nullptr || result == 0 || !safeThis->menuHandler)
          return;

        for (int i = 0; i < subMenus.size(); ++i)
        {
          for (juce::PopupMenu::MenuItemIterator it(subMenus.getReference(i), true);
               it.next();)
          {
            if (it.getItem().itemID == result)
            {
              safeThis->menuHandler->menuItemSelected(result, i);
              return;
            }
          }
        }
      });
}

void MainComponent::toggleMobileRecording()
{
  if (micRecorder && micRecorder->isRecording())
  {
    finishMobileRecording();
    return;
  }

  juce::Component::SafePointer<MainComponent> safeThis(this);
  MobilePermissions::requestMicrophone([safeThis](bool granted)
  {
    if (safeThis == nullptr)
      return;

    if (!granted)
    {
      StyledMessageBox::show(safeThis.getComponent(), TR("mobile.record_failed"),
                             TR("mobile.mic_denied"),
                             StyledMessageBox::WarningIcon);
      return;
    }

    safeThis->startMobileRecording();
  });
}

void MainComponent::startMobileRecording()
{
  auto *audioEngine = editorController ? editorController->getAudioEngine() : nullptr;
  if (audioEngine == nullptr)
    return;

  if (isPlaying)
    stop();

  if (!micRecorder)
    micRecorder = std::make_unique<MicRecorder>(audioEngine->getDeviceManager());

  const auto name = "Recording " +
                    juce::Time::getCurrentTime().formatted("%Y-%m-%d %H.%M.%S");
  const auto file = getMobileFolder("Recordings").getChildFile(name + ".wav");

  const auto error = micRecorder->start(file);
  if (error.isNotEmpty())
  {
    StyledMessageBox::show(this, TR("mobile.record_failed"), error,
                           StyledMessageBox::WarningIcon);
    return;
  }

  mobileBar.setRecording(true);
}

void MainComponent::finishMobileRecording()
{
  if (!micRecorder)
    return;

  const auto file = micRecorder->stop();
  mobileBar.setRecording(false);

  // A take shorter than a block or two has nothing to analyse.
  if (file.existsAsFile() && file.getSize() > 4096)
  {
    addRecentFile(file);
    loadAudioFile(file);
  }
  else
  {
    file.deleteFile();
  }
}

void MainComponent::importAndOpen(const juce::URL &url)
{
  const auto name = url.isLocalFile() ? url.getLocalFile().getFileName()
                                      : url.getFileName();
  if (name.isEmpty())
    return;

  auto target = getMobileFolder("Imported").getChildFile(name);

  // Already inside the sandbox (e.g. a previous recording): open in place.
  const auto documents =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
  if (url.isLocalFile() && url.getLocalFile().isAChildOf(documents))
  {
    target = url.getLocalFile();
  }
  else
  {
    std::unique_ptr<juce::InputStream> input(url.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)));
    if (input == nullptr)
    {
      StyledMessageBox::show(this, TR("mobile.open"),
                             TR("mobile.import_failed"),
                             StyledMessageBox::WarningIcon);
      return;
    }

    target = target.getNonexistentSibling(false);
    juce::FileOutputStream output(target);
    if (!output.openedOk() || output.writeFromInputStream(*input, -1) <= 0)
    {
      output.flush();
      target.deleteFile();
      StyledMessageBox::show(this, TR("mobile.open"),
                             TR("mobile.import_failed"),
                             StyledMessageBox::WarningIcon);
      return;
    }
    output.flush();
  }

  addRecentFile(target);
  if (target.hasFileExtension("pitchnet"))
    openProjectFile(target);
  else
    loadAudioFile(target);
}

void MainComponent::shareExportedFile(const juce::File &file)
{
  if (!file.existsAsFile())
    return;

#if JUCE_CONTENT_SHARING
  shareSession = juce::ContentSharer::shareFilesScoped(
      {juce::URL(file)}, [](bool, const juce::String &) {}, this);
#else
  // Desktop builds of the mobile layout have no share sheet.
  file.revealToUser();
#endif
}

#endif // PITCHNET_MOBILE_UI
