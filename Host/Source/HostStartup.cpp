/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainHostWindow.h"
#include "InternalFilters.h"

#if !(JUCE_PLUGINHOST_VST || JUCE_PLUGINHOST_VST3 || JUCE_PLUGINHOST_AU)
#error                                                                         \
    "If you're building the audio plugin host, you probably want to enable VST and/or AU support"
#endif

//==============================================================================
class PluginHostApp : public juce::JUCEApplication {
public:
  PluginHostApp() {}

  void initialise(const juce::String &commandLine) override {
    // initialise our settings file..

    juce::PropertiesFile::Options options;
    options.applicationName = "Trommel-Tanz-Musik-Maschine Host";
    options.filenameSuffix = "settings";
    options.osxLibrarySubFolder = "Preferences";

    appProperties = new juce::ApplicationProperties();
    appProperties->setStorageParameters(options);

    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);

    mainWindow = new ttmm::MainHostWindow();
    mainWindow->setUsingNativeTitleBar(true);

    commandManager.registerAllCommandsForTarget(this);
    commandManager.registerAllCommandsForTarget(mainWindow);

    mainWindow->menuItemsChanged();

    if (commandLine.isNotEmpty() && !commandLine.trimStart().startsWith("-") &&
        mainWindow->getGraphEditor() != nullptr)
      mainWindow->getGraphEditor()->graph.loadFrom(
          juce::File::getCurrentWorkingDirectory().getChildFile(commandLine),
          true);
  }

  void shutdown() override {
    mainWindow = nullptr;
    appProperties = nullptr;
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
  }

  void systemRequestedQuit() override {
    if (mainWindow != nullptr)
      mainWindow->tryToQuitApplication();
    else
      JUCEApplicationBase::quit();
  }

  const juce::String getApplicationName() override {
    return "Juce Plug-In Host";
  }
  const juce::String getApplicationVersion() override {
    return ProjectInfo::versionString;
  }
  bool moreThanOneInstanceAllowed() override { return true; }

  juce::ApplicationCommandManager commandManager;
  juce::ScopedPointer<juce::ApplicationProperties> appProperties;
  juce::LookAndFeel_V3 lookAndFeel;

private:
  juce::ScopedPointer<ttmm::MainHostWindow> mainWindow;
};

static PluginHostApp &getApp() {
  return *dynamic_cast<PluginHostApp *>(juce::JUCEApplication::getInstance());
}
juce::ApplicationCommandManager &getCommandManager() {
  return getApp().commandManager;
}
juce::ApplicationProperties &getAppProperties() {
  return *getApp().appProperties;
}

// This kicks the whole thing off..
//START_JUCE_APPLICATION(PluginHostApp)

static juce::JUCEApplicationBase* juce_CreateApplication() { return new PluginHostApp(); }
extern "C" JUCE_MAIN_FUNCTION
{
juce::JUCEApplicationBase::createInstance = &juce_CreateApplication;
return juce::JUCEApplicationBase::main(JUCE_MAIN_FUNCTION_ARGS);
}
