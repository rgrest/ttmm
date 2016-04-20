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

//==============================================================================
class ttmm::MainHostWindow::PluginListWindow : public juce::DocumentWindow {
public:
  PluginListWindow(MainHostWindow &owner_,
                   juce::AudioPluginFormatManager &formatManager)
      : juce::DocumentWindow("Available Plugins", juce::Colours::white,
                             juce::DocumentWindow::minimiseButton |
                                 juce::DocumentWindow::closeButton),
        owner(owner_) {
    const juce::File deadMansPedalFile(
        getAppProperties().getUserSettings()->getFile().getSiblingFile(
            "RecentlyCrashedPluginsList"));

    setContentOwned(new juce::PluginListComponent(
                        formatManager, owner.knownPluginList, deadMansPedalFile,
                        getAppProperties().getUserSettings()),
                    true);

    setResizable(true, false);
    setResizeLimits(300, 400, 800, 1500);
    setTopLeftPosition(60, 60);

    restoreWindowStateFromString(
        getAppProperties().getUserSettings()->getValue("listWindowPos"));
    setVisible(true);
  }

  ~PluginListWindow() {
    getAppProperties().getUserSettings()->setValue("listWindowPos",
                                                   getWindowStateAsString());

    clearContentComponent();
  }

  void closeButtonPressed() { owner.pluginListWindow = nullptr; }

private:
  MainHostWindow &owner;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListWindow)
};

//==============================================================================
ttmm::MainHostWindow::MainHostWindow()
    : DocumentWindow(juce::JUCEApplication::getInstance()->getApplicationName(),
                     juce::Colours::lightgrey, DocumentWindow::allButtons) {
  formatManager.addDefaultFormats();
  formatManager.addFormat(new InternalPluginFormat());

  juce::ScopedPointer<juce::XmlElement> savedAudioState(
      getAppProperties().getUserSettings()->getXmlValue("audioDeviceState"));

  deviceManager.initialise(256, 256, savedAudioState, true);

  setResizable(true, false);
  setResizeLimits(500, 400, 10000, 10000);
  centreWithSize(800, 600);

  setContentOwned(new GraphDocumentComponent(formatManager, &deviceManager),
                  false);

  restoreWindowStateFromString(
      getAppProperties().getUserSettings()->getValue("mainWindowPos"));

  setVisible(true);

  InternalPluginFormat internalFormat;
  internalFormat.getAllTypes(internalTypes);

  juce::ScopedPointer<juce::XmlElement> savedPluginList(
      getAppProperties().getUserSettings()->getXmlValue("pluginList"));

  if (savedPluginList != nullptr)
    knownPluginList.recreateFromXml(*savedPluginList);

  pluginSortMethod =
      (juce::KnownPluginList::SortMethod)getAppProperties()
          .getUserSettings()
          ->getIntValue("pluginSortMethod",
                        juce::KnownPluginList::sortByManufacturer);

  knownPluginList.addChangeListener(this);

  addKeyListener(getCommandManager().getKeyMappings());

  juce::Process::setPriority(juce::Process::HighPriority);

#if JUCE_MAC
  setMacMainMenu(this);
#else
  setMenuBar(this);
#endif

  getCommandManager().setFirstCommandTarget(this);
}

ttmm::MainHostWindow::~MainHostWindow() {
  pluginListWindow = nullptr;

#if JUCE_MAC
  setMacMainMenu(nullptr);
#else
  setMenuBar(nullptr);
#endif

  knownPluginList.removeChangeListener(this);

  getAppProperties().getUserSettings()->setValue("mainWindowPos",
                                                 getWindowStateAsString());
  clearContentComponent();
}

void ttmm::MainHostWindow::closeButtonPressed() { tryToQuitApplication(); }

bool ttmm::MainHostWindow::tryToQuitApplication() {
  PluginWindow::closeAllCurrentlyOpenWindows();

  if (getGraphEditor() == nullptr ||
      getGraphEditor()->graph.saveIfNeededAndUserAgrees() ==
          juce::FileBasedDocument::savedOk) {
    juce::JUCEApplication::quit();
    return true;
  }

  return false;
}

void ttmm::MainHostWindow::changeListenerCallback(juce::ChangeBroadcaster *) {
  menuItemsChanged();

  // save the plugin list every time it gets chnaged, so that if we're scanning
  // and it crashes, we've still saved the previous ones
  juce::ScopedPointer<juce::XmlElement> savedPluginList(
      knownPluginList.createXml());

  if (savedPluginList != nullptr) {
    getAppProperties().getUserSettings()->setValue("pluginList",
                                                   savedPluginList);
    getAppProperties().saveIfNeeded();
  }
}

juce::StringArray ttmm::MainHostWindow::getMenuBarNames() {
  const char *const names[] = {"File", "Plugins", "Options", nullptr};

  return juce::StringArray(names);
}

juce::PopupMenu
ttmm::MainHostWindow::getMenuForIndex(int topLevelMenuIndex,
                                      const juce::String & /*menuName*/) {
  juce::PopupMenu menu;

  if (topLevelMenuIndex == 0) {
    // "File" menu
    menu.addCommandItem(&getCommandManager(), CommandIDs::open);

    juce::RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString(
        getAppProperties().getUserSettings()->getValue(
            "recentFilterGraphFiles"));

    juce::PopupMenu recentFilesMenu;
    recentFiles.createPopupMenuItems(recentFilesMenu, 100, true, true);
    menu.addSubMenu("Open recent file", recentFilesMenu);

    menu.addCommandItem(&getCommandManager(), CommandIDs::save);
    menu.addCommandItem(&getCommandManager(), CommandIDs::saveAs);
    menu.addSeparator();
    menu.addCommandItem(&getCommandManager(),
                        juce::StandardApplicationCommandIDs::quit);
  } else if (topLevelMenuIndex == 1) {
    // "Plugins" menu
    juce::PopupMenu pluginsMenu;
    addPluginsToMenu(pluginsMenu);
    menu.addSubMenu("Create plugin", pluginsMenu);
    menu.addSeparator();
    menu.addItem(250, "Delete all plugins");
  } else if (topLevelMenuIndex == 2) {
    // "Options" menu

    menu.addCommandItem(&getCommandManager(), CommandIDs::showPluginListEditor);

    juce::PopupMenu sortTypeMenu;
    sortTypeMenu.addItem(200, "List plugins in default order", true,
                         pluginSortMethod ==
                             juce::KnownPluginList::defaultOrder);
    sortTypeMenu.addItem(201, "List plugins in alphabetical order", true,
                         pluginSortMethod ==
                             juce::KnownPluginList::sortAlphabetically);
    sortTypeMenu.addItem(202, "List plugins by category", true,
                         pluginSortMethod ==
                             juce::KnownPluginList::sortByCategory);
    sortTypeMenu.addItem(203, "List plugins by manufacturer", true,
                         pluginSortMethod ==
                             juce::KnownPluginList::sortByManufacturer);
    sortTypeMenu.addItem(
        204, "List plugins based on the directory structure", true,
        pluginSortMethod == juce::KnownPluginList::sortByFileSystemLocation);
    menu.addSubMenu("Plugin menu type", sortTypeMenu);

    menu.addSeparator();
    menu.addCommandItem(&getCommandManager(), CommandIDs::showAudioSettings);

    menu.addSeparator();
    menu.addCommandItem(&getCommandManager(), CommandIDs::aboutBox);
  }

  return menu;
}

void ttmm::MainHostWindow::menuItemSelected(int menuItemID,
                                            int /*topLevelMenuIndex*/) {
  GraphDocumentComponent *const graphEditor = getGraphEditor();

  if (menuItemID == 250) {
    if (graphEditor != nullptr)
      graphEditor->graph.clear();
  } else if (menuItemID >= 100 && menuItemID < 200) {
    juce::RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString(
        getAppProperties().getUserSettings()->getValue(
            "recentFilterGraphFiles"));

    if (graphEditor != nullptr &&
        graphEditor->graph.saveIfNeededAndUserAgrees() ==
            juce::FileBasedDocument::savedOk)
      graphEditor->graph.loadFrom(recentFiles.getFile(menuItemID - 100), true);
  } else if (menuItemID >= 200 && menuItemID < 210) {
    if (menuItemID == 200)
      pluginSortMethod = juce::KnownPluginList::defaultOrder;
    else if (menuItemID == 201)
      pluginSortMethod = juce::KnownPluginList::sortAlphabetically;
    else if (menuItemID == 202)
      pluginSortMethod = juce::KnownPluginList::sortByCategory;
    else if (menuItemID == 203)
      pluginSortMethod = juce::KnownPluginList::sortByManufacturer;
    else if (menuItemID == 204)
      pluginSortMethod = juce::KnownPluginList::sortByFileSystemLocation;

    getAppProperties().getUserSettings()->setValue("pluginSortMethod",
                                                   (int)pluginSortMethod);

    menuItemsChanged();
  } else {
    createPlugin(
        getChosenType(menuItemID),
        proportionOfWidth(0.3f +
                          juce::Random::getSystemRandom().nextFloat() * 0.6f),
        proportionOfHeight(0.3f +
                           juce::Random::getSystemRandom().nextFloat() * 0.6f));
  }
}

void ttmm::MainHostWindow::createPlugin(const juce::PluginDescription *desc,
                                        int x, int y) {
  GraphDocumentComponent *const graphEditor = getGraphEditor();

  if (graphEditor != nullptr) {
    graphEditor->createNewPlugin(desc, x, y);
  }
}

void ttmm::MainHostWindow::addPluginsToMenu(juce::PopupMenu &m) const {
  for (int i = 0; i < internalTypes.size(); ++i)
    m.addItem(i + 1, internalTypes.getUnchecked(i)->name);

  m.addSeparator();

  knownPluginList.addToMenu(m, pluginSortMethod);
}

const juce::PluginDescription *
ttmm::MainHostWindow::getChosenType(const int menuID) const {
  if (menuID >= 1 && menuID < 1 + internalTypes.size())
    return internalTypes[menuID - 1];

  return knownPluginList.getType(knownPluginList.getIndexChosenByMenu(menuID));
}

//==============================================================================
juce::ApplicationCommandTarget *ttmm::MainHostWindow::getNextCommandTarget() {
  return findFirstTargetParentComponent();
}

void ttmm::MainHostWindow::getAllCommands(
    juce::Array<juce::CommandID> &commands) {
  // this returns the set of all commands that this target can perform..
  const juce::CommandID ids[] = {
      CommandIDs::open, CommandIDs::save, CommandIDs::saveAs,
      CommandIDs::showPluginListEditor, CommandIDs::showAudioSettings,
      CommandIDs::aboutBox};

  commands.addArray(ids, juce::numElementsInArray(ids));
}

void ttmm::MainHostWindow::getCommandInfo(
    const juce::CommandID commandID, juce::ApplicationCommandInfo &result) {
  const juce::String category("General");

  switch (commandID) {
  case CommandIDs::open:
    result.setInfo("Open...", "Opens a filter graph file", category, 0);
    result.defaultKeypresses.add(
        juce::KeyPress('o', juce::ModifierKeys::commandModifier, 0));
    break;

  case CommandIDs::save:
    result.setInfo("Save", "Saves the current graph to a file", category, 0);
    result.defaultKeypresses.add(
        juce::KeyPress('s', juce::ModifierKeys::commandModifier, 0));
    break;

  case CommandIDs::saveAs:
    result.setInfo("Save As...", "Saves a copy of the current graph to a file",
                   category, 0);
    result.defaultKeypresses.add(
        juce::KeyPress('s', juce::ModifierKeys::shiftModifier |
                                juce::ModifierKeys::commandModifier,
                       0));
    break;

  case CommandIDs::showPluginListEditor:
    result.setInfo("Edit the list of available plug-Ins...",
                   juce::String::empty, category, 0);
    result.addDefaultKeypress('p', juce::ModifierKeys::commandModifier);
    break;

  case CommandIDs::showAudioSettings:
    result.setInfo("Change the audio device settings", juce::String::empty,
                   category, 0);
    result.addDefaultKeypress('a', juce::ModifierKeys::commandModifier);
    break;

  case CommandIDs::aboutBox:
    result.setInfo("About...", juce::String::empty, category, 0);
    break;

  default:
    break;
  }
}

bool ttmm::MainHostWindow::perform(const InvocationInfo &info) {
  GraphDocumentComponent *const graphEditor = getGraphEditor();

  switch (info.commandID) {
  case CommandIDs::open:
    if (graphEditor != nullptr &&
        graphEditor->graph.saveIfNeededAndUserAgrees() ==
            juce::FileBasedDocument::savedOk)
      graphEditor->graph.loadFromUserSpecifiedFile(true);

    break;

  case CommandIDs::save:
    if (graphEditor != nullptr)
      graphEditor->graph.save(true, true);
    break;

  case CommandIDs::saveAs:
    if (graphEditor != nullptr)
      graphEditor->graph.saveAs(juce::File::nonexistent, true, true, true);
    break;

  case CommandIDs::showPluginListEditor:
    if (pluginListWindow == nullptr)
      pluginListWindow = new PluginListWindow(*this, formatManager);

    pluginListWindow->toFront(true);
    break;

  case CommandIDs::showAudioSettings:
    showAudioSettings();
    break;

  case CommandIDs::aboutBox:
    // TODO
    break;

  default:
    return false;
  }

  return true;
}

void ttmm::MainHostWindow::showAudioSettings() {
  juce::AudioDeviceSelectorComponent audioSettingsComp(
      deviceManager, 0, 256, 0, 256, true, true, true, false);

  audioSettingsComp.setSize(500, 450);

  juce::DialogWindow::LaunchOptions o;
  o.content.setNonOwned(&audioSettingsComp);
  o.dialogTitle = "Audio Settings";
  o.componentToCentreAround = this;
  o.dialogBackgroundColour = juce::Colours::azure;
  o.escapeKeyTriggersCloseButton = true;
  o.useNativeTitleBar = false;
  o.resizable = false;

  o.runModal();

  juce::ScopedPointer<juce::XmlElement> audioState(
      deviceManager.createStateXml());

  getAppProperties().getUserSettings()->setValue("audioDeviceState",
                                                 audioState);
  getAppProperties().getUserSettings()->saveIfNeeded();

  GraphDocumentComponent *const graphEditor = getGraphEditor();

  if (graphEditor != nullptr)
    graphEditor->graph.removeIllegalConnections();
}

bool ttmm::MainHostWindow::isInterestedInFileDrag(const juce::StringArray &) {
  return true;
}

void ttmm::MainHostWindow::fileDragEnter(const juce::StringArray &, int, int) {}

void ttmm::MainHostWindow::fileDragMove(const juce::StringArray &, int, int) {}

void ttmm::MainHostWindow::fileDragExit(const juce::StringArray &) {}

void ttmm::MainHostWindow::filesDropped(const juce::StringArray &files, int x,
                                        int y) {
  GraphDocumentComponent *const graphEditor = getGraphEditor();

  if (graphEditor != nullptr) {
    if (files.size() == 1 &&
        juce::File(files[0]).hasFileExtension(filenameSuffix)) {
      if (graphEditor->graph.saveIfNeededAndUserAgrees() ==
          juce::FileBasedDocument::savedOk)
        graphEditor->graph.loadFrom(juce::File(files[0]), true);
    } else {
      juce::OwnedArray<juce::PluginDescription> typesFound;
      knownPluginList.scanAndAddDragAndDroppedFiles(formatManager, files,
                                                    typesFound);

      juce::Point<int> pos(
          graphEditor->getLocalPoint(this, juce::Point<int>(x, y)));

      for (int i = 0; i < juce::jmin(5, typesFound.size()); ++i)
        createPlugin(typesFound.getUnchecked(i), pos.getX(), pos.getY());
    }
  }
}

ttmm::GraphDocumentComponent *ttmm::MainHostWindow::getGraphEditor() const {
  return dynamic_cast<GraphDocumentComponent *>(getContentComponent());
}
