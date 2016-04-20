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

#ifndef __MAINHOSTWINDOW_JUCEHEADER__
#define __MAINHOSTWINDOW_JUCEHEADER__

#include "FilterGraph.h"
#include "GraphEditorPanel.h"

juce::ApplicationCommandManager &getCommandManager();
juce::ApplicationProperties &getAppProperties();

namespace ttmm {

//==============================================================================
namespace CommandIDs {
static const int open = 0x30000;
static const int save = 0x30001;
static const int saveAs = 0x30002;
static const int showPluginListEditor = 0x30100;
static const int showAudioSettings = 0x30200;
static const int aboutBox = 0x30300;
}

//==============================================================================
/**
*/
class MainHostWindow : public juce::DocumentWindow,
    public juce::MenuBarModel,
    public juce::ApplicationCommandTarget,
    public juce::ChangeListener,
    public juce::FileDragAndDropTarget {
public:
    //==============================================================================
    MainHostWindow();
    ~MainHostWindow();

    //==============================================================================
    void closeButtonPressed();
    void changeListenerCallback(juce::ChangeBroadcaster *);

    bool isInterestedInFileDrag(const juce::StringArray &files);
    void fileDragEnter(const juce::StringArray &files, int, int);
    void fileDragMove(const juce::StringArray &files, int, int);
    void fileDragExit(const juce::StringArray &files);
    void filesDropped(const juce::StringArray &files, int, int);

    juce::StringArray getMenuBarNames();
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex,
        const juce::String &menuName);
    void menuItemSelected(int menuItemID, int topLevelMenuIndex);
    juce::ApplicationCommandTarget *getNextCommandTarget();
    void getAllCommands(juce::Array<juce::CommandID> &commands);
    void getCommandInfo(juce::CommandID commandID,
        juce::ApplicationCommandInfo &result);
    bool perform(const InvocationInfo &info);

    bool tryToQuitApplication();

    void createPlugin(const juce::PluginDescription *desc, int x, int y);

    void addPluginsToMenu(juce::PopupMenu &m) const;
    const juce::PluginDescription *getChosenType(const int menuID) const;

    GraphDocumentComponent *getGraphEditor() const;
    juce::KnownPluginList knownPluginList;
private:
    //==============================================================================

  juce::AudioDeviceManager deviceManager;
  juce::AudioPluginFormatManager formatManager;

  juce::OwnedArray<juce::PluginDescription> internalTypes;

  juce::KnownPluginList::SortMethod pluginSortMethod;

  class PluginListWindow;
  juce::ScopedPointer<PluginListWindow> pluginListWindow;

  void showAudioSettings();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainHostWindow)
};
}
#endif // __MAINHOSTWINDOW_JUCEHEADER__
