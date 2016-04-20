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

#ifndef __GRAPHEDITORPANEL_JUCEHEADER__
#define __GRAPHEDITORPANEL_JUCEHEADER__

#include "FilterGraph.h"

namespace ttmm {

class FilterComponent;
class ConnectorComponent;
class PinComponent;

//==============================================================================
/**
    A panel that displays and edits a FilterGraph.
*/
class GraphEditorPanel : public juce::Component, public juce::ChangeListener {
public:
  GraphEditorPanel(FilterGraph &graph);
  ~GraphEditorPanel();

  void paint(juce::Graphics &g);
  void mouseDown(const juce::MouseEvent &e);

  void createNewPlugin(const juce::PluginDescription *desc, int x, int y);

  FilterComponent *getComponentForFilter(juce::uint32 filterID) const;
  ConnectorComponent *getComponentForConnection(
      const juce::AudioProcessorGraph::Connection &conn) const;
  PinComponent *findPinAt(int x, int y) const;

  void resized();
  void changeListenerCallback(juce::ChangeBroadcaster *);
  void updateComponents();

  //==============================================================================
  void beginConnectorDrag(juce::uint32 sourceFilterID, int sourceFilterChannel,
                          juce::uint32 destFilterID, int destFilterChannel,
                          const juce::MouseEvent &e);
  void dragConnector(const juce::MouseEvent &e);
  void endDraggingConnector(const juce::MouseEvent &e);

  //==============================================================================
private:
  FilterGraph &graph;
  juce::ScopedPointer<ConnectorComponent> draggingConnector;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphEditorPanel)
};

//==============================================================================
/**
    A panel that embeds a GraphEditorPanel with a midi keyboard at the bottom.

    It also manages the graph itself, and plays it.
*/
class GraphDocumentComponent : public juce::Component {
public:
  //==============================================================================
  GraphDocumentComponent(juce::AudioPluginFormatManager &formatManager,
                         juce::AudioDeviceManager *deviceManager);
  ~GraphDocumentComponent();

  //==============================================================================
  void createNewPlugin(const juce::PluginDescription *desc, int x, int y);

  //==============================================================================
  FilterGraph graph;

  //==============================================================================
  void resized();

  //==============================================================================
  void changeListenerCallback(juce::ChangeBroadcaster* source);

private:
  //==============================================================================
  juce::AudioDeviceManager *deviceManager;
  juce::AudioProcessorPlayer graphPlayer;
  juce::MidiKeyboardState keyState;
  juce::MidiOutput *midiOut;

  GraphEditorPanel *graphPanel;
  juce::Component *keyboardComp;
  juce::Component *statusBar;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphDocumentComponent)
};

//==============================================================================
/** A desktop window containing a plugin's UI. */
class PluginWindow : public juce::DocumentWindow {
public:
  enum WindowFormatType { Normal = 0, Generic, Programs, Parameters };

  PluginWindow(juce::Component *pluginEditor, juce::AudioProcessorGraph::Node *,
               WindowFormatType);
  ~PluginWindow();

  static PluginWindow *getWindowFor(juce::AudioProcessorGraph::Node *,
                                    WindowFormatType);

  static void closeCurrentlyOpenWindowsFor(const juce::uint32 nodeId);
  static void closeAllCurrentlyOpenWindows();

  void moved() override;
  void closeButtonPressed() override;

private:
  juce::AudioProcessorGraph::Node *owner;
  WindowFormatType type;

  float getDesktopScaleFactor() const override { return 1.0f; }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginWindow)
};
}
#endif // __GRAPHEDITORPANEL_JUCEHEADER__
