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
#include "GraphEditorPanel.h"
#include "InternalFilters.h"
#include "MainHostWindow.h"

//==============================================================================
class ttmm::PluginWindow;
static juce::Array<ttmm::PluginWindow *> activePluginWindows;

ttmm::PluginWindow::PluginWindow(juce::Component *const pluginEditor,
                                 juce::AudioProcessorGraph::Node *const o,
                                 WindowFormatType t)
    : juce::DocumentWindow(pluginEditor->getName(), juce::Colours::lightblue,
                           juce::DocumentWindow::minimiseButton |
                               juce::DocumentWindow::closeButton),
      owner(o), type(t) {
  setSize(400, 300);

  setContentOwned(pluginEditor, true);

  setTopLeftPosition(
      owner->properties.getWithDefault(
          "uiLastX", juce::Random::getSystemRandom().nextInt(500)),
      owner->properties.getWithDefault(
          "uiLastY", juce::Random::getSystemRandom().nextInt(500)));
  setVisible(true);

  activePluginWindows.add(this);
}

void ttmm::PluginWindow::closeCurrentlyOpenWindowsFor(
    const juce::uint32 nodeId) {
  for (int i = activePluginWindows.size(); --i >= 0;)
    if (activePluginWindows.getUnchecked(i)->owner->nodeId == nodeId)
      delete activePluginWindows.getUnchecked(i);
}

void ttmm::PluginWindow::closeAllCurrentlyOpenWindows() {
  if (activePluginWindows.size() > 0) {
    for (int i = activePluginWindows.size(); --i >= 0;)
      delete activePluginWindows.getUnchecked(i);

    juce::Component dummyModalComp;
    dummyModalComp.enterModalState();
    juce::MessageManager::getInstance()->runDispatchLoopUntil(50);
  }
}

//==============================================================================
class ProcessorProgramPropertyComp : public juce::PropertyComponent,
                                     private juce::AudioProcessorListener {
public:
  ProcessorProgramPropertyComp(const juce::String &name,
                               juce::AudioProcessor &p, int index_)
      : juce::PropertyComponent(name), owner(p), index(index_) {
    owner.addListener(this);
  }

  ~ProcessorProgramPropertyComp() { owner.removeListener(this); }

  void refresh() {}
  void audioProcessorChanged(juce::AudioProcessor *) {}
  void audioProcessorParameterChanged(juce::AudioProcessor *, int, float) {}

private:
  juce::AudioProcessor &owner;
  const int index;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorProgramPropertyComp)
};

class ProgramAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
  ProgramAudioProcessorEditor(juce::AudioProcessor *const p)
      : AudioProcessorEditor(p) {
    jassert(p != nullptr);
    setOpaque(true);

    addAndMakeVisible(panel);

    juce::Array<juce::PropertyComponent *> programs;

    const int numPrograms = p->getNumPrograms();
    int totalHeight = 0;

    for (int i = 0; i < numPrograms; ++i) {
      juce::String name(p->getProgramName(i).trim());

      if (name.isEmpty())
        name = "Unnamed";

      ProcessorProgramPropertyComp *const pc =
          new ProcessorProgramPropertyComp(name, *p, i);
      programs.add(pc);
      totalHeight += pc->getPreferredHeight();
    }

    panel.addProperties(programs);

    setSize(400, juce::jlimit(25, 400, totalHeight));
  }

  void paint(juce::Graphics &g) { g.fillAll(juce::Colours::grey); }

  void resized() { panel.setBounds(getLocalBounds()); }

private:
  juce::PropertyPanel panel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgramAudioProcessorEditor)
};

//==============================================================================
ttmm::PluginWindow *
ttmm::PluginWindow::getWindowFor(juce::AudioProcessorGraph::Node *const node,
                                 WindowFormatType type) {
  jassert(node != nullptr);

  for (int i = activePluginWindows.size(); --i >= 0;)
    if (activePluginWindows.getUnchecked(i)->owner == node &&
        activePluginWindows.getUnchecked(i)->type == type)
      return activePluginWindows.getUnchecked(i);

  juce::AudioProcessor *processor = node->getProcessor();
  juce::AudioProcessorEditor *ui = nullptr;

  if (type == Normal) {
    ui = processor->createEditorIfNeeded();

    if (ui == nullptr)
      type = Generic;
  }

  if (ui == nullptr) {
    if (type == Generic || type == Parameters)
      ui = new juce::GenericAudioProcessorEditor(processor);
    else if (type == Programs)
      ui = new ProgramAudioProcessorEditor(processor);
  }

  if (ui != nullptr) {
    if (juce::AudioPluginInstance *const plugin =
            dynamic_cast<juce::AudioPluginInstance *>(processor))
      ui->setName(plugin->getName());

    return new PluginWindow(ui, node, type);
  }

  return nullptr;
}

ttmm::PluginWindow::~PluginWindow() {
  activePluginWindows.removeFirstMatchingValue(this);
  clearContentComponent();
}

void ttmm::PluginWindow::moved() {
  owner->properties.set("uiLastX", getX());
  owner->properties.set("uiLastY", getY());
}

void ttmm::PluginWindow::closeButtonPressed() { delete this; }

//==============================================================================
namespace ttmm {
class PinComponent : public juce::Component,
                     public juce::SettableTooltipClient {
public:
  PinComponent(FilterGraph &graph_, const juce::uint32 filterID_,
               const int index_, const bool isInput_)
      : filterID(filterID_), index(index_), isInput(isInput_), graph(graph_) {
    if (const juce::AudioProcessorGraph::Node::Ptr node =
            graph.getNodeForId(filterID_)) {
      juce::String tip;

      if (index_ == FilterGraph::midiChannelNumber) {
        tip = isInput ? "MIDI Input" : "MIDI Output";
      } else {
        if (isInput)
          tip = node->getProcessor()->getInputChannelName(index_);
        else
          tip = node->getProcessor()->getOutputChannelName(index_);

        if (tip.isEmpty())
          tip = (isInput ? "Input " : "Output ") + juce::String(index_ + 1);
      }

      setTooltip(tip);
    }

    setSize(16, 16);
  }

  void paint(juce::Graphics &g) {
    const float w = (float)getWidth();
    const float h = (float)getHeight();

    juce::Path p;
    p.addEllipse(w * 0.25f, h * 0.25f, w * 0.5f, h * 0.5f);

    p.addRectangle(w * 0.4f, isInput ? (0.5f * h) : 0.0f, w * 0.2f, h * 0.5f);

    g.setColour(index == FilterGraph::midiChannelNumber ? juce::Colours::red
                                                        : juce::Colours::green);
    g.fillPath(p);
  }

  void mouseDown(const juce::MouseEvent &e) {
    getGraphPanel()->beginConnectorDrag(isInput ? 0 : filterID, index,
                                        isInput ? filterID : 0, index, e);
  }

  void mouseDrag(const juce::MouseEvent &e) {
    getGraphPanel()->dragConnector(e);
  }

  void mouseUp(const juce::MouseEvent &e) {
    getGraphPanel()->endDraggingConnector(e);
  }

  const juce::uint32 filterID;
  const int index;
  const bool isInput;

private:
  FilterGraph &graph;

  GraphEditorPanel *getGraphPanel() const noexcept {
    return findParentComponentOfClass<ttmm::GraphEditorPanel>();
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PinComponent)
};
}
//==============================================================================

namespace ttmm {
class FilterComponent : public juce::Component {
public:
  FilterComponent(ttmm::FilterGraph &graph_, const juce::uint32 filterID_)
      : graph(graph_), filterID(filterID_), numInputs(0), numOutputs(0),
        pinSize(16), font(13.0f, juce::Font::bold), numIns(0), numOuts(0) {
    shadow.setShadowProperties(juce::DropShadow(
        juce::Colours::black.withAlpha(0.5f), 3, juce::Point<int>(0, 1)));
    setComponentEffect(&shadow);

    setSize(150, 60);
  }

  ~FilterComponent() { deleteAllChildren(); }

  void mouseDown(const juce::MouseEvent &e) {
    originalPos = localPointToGlobal(juce::Point<int>());

    toFront(true);

    if (e.mods.isPopupMenu()) {
      juce::PopupMenu m;
      m.addItem(1, "Delete this filter");
      m.addItem(2, "Disconnect all pins");
      m.addSeparator();
      m.addItem(3, "Show plugin UI");
      m.addItem(4, "Show all programs");
      m.addItem(5, "Show all parameters");
      m.addItem(6, "Test state save/load");

      const int r = m.show();

      if (r == 1) {
        graph.removeFilter(filterID);
        return;
      } else if (r == 2) {
        graph.disconnectFilter(filterID);
      } else {
        if (juce::AudioProcessorGraph::Node::Ptr f =
                graph.getNodeForId(filterID)) {
          juce::AudioProcessor *const processor = f->getProcessor();
          jassert(processor != nullptr);

          if (r == 6) {
            juce::MemoryBlock state;
            processor->getStateInformation(state);
            processor->setStateInformation(state.getData(),
                                           (int)state.getSize());
          } else {
            ttmm::PluginWindow::WindowFormatType type =
                processor->hasEditor() ? ttmm::PluginWindow::Normal
                                       : ttmm::PluginWindow::Generic;

            switch (r) {
            case 4:
              type = ttmm::PluginWindow::Programs;
              break;
            case 5:
              type = ttmm::PluginWindow::Parameters;
              break;

            default:
              break;
            };

            if (ttmm::PluginWindow *const w =
                    ttmm::PluginWindow::getWindowFor(f, type))
              w->toFront(true);
          }
        }
      }
    }
  }

  void mouseDrag(const juce::MouseEvent &e) {
    if (!e.mods.isPopupMenu()) {
      juce::Point<int> pos(originalPos +
                           juce::Point<int>(e.getDistanceFromDragStartX(),
                                            e.getDistanceFromDragStartY()));

      if (getParentComponent() != nullptr)
        pos = getParentComponent()->getLocalPoint(nullptr, pos);

      graph.setNodePosition(
          filterID, (pos.getX() + getWidth() / 2) / (double)getParentWidth(),
          (pos.getY() + getHeight() / 2) / (double)getParentHeight());

      getGraphPanel()->updateComponents();
    }
  }

  void mouseUp(const juce::MouseEvent &e) {
    if (e.mouseWasClicked() && e.getNumberOfClicks() == 2) {
      if (const juce::AudioProcessorGraph::Node::Ptr f =
              graph.getNodeForId(filterID))
        if (ttmm::PluginWindow *const w =
                ttmm::PluginWindow::getWindowFor(f, ttmm::PluginWindow::Normal))
          w->toFront(true);
    } else if (!e.mouseWasClicked()) {
      graph.setChangedFlag(true);
    }
  }

  bool hitTest(int x, int y) {
    for (int i = getNumChildComponents(); --i >= 0;)
      if (getChildComponent(i)->getBounds().contains(x, y))
        return true;

    return x >= 3 && x < getWidth() - 6 && y >= pinSize &&
           y < getHeight() - pinSize;
  }

  void paint(juce::Graphics &g) {
    g.setColour(juce::Colours::lightgrey);

    const int x = 4;
    const int y = pinSize;
    const int w = getWidth() - x * 2;
    const int h = getHeight() - pinSize * 2;

    g.fillRect(x, y, w, h);

    g.setColour(juce::Colours::black);
    g.setFont(font);
    g.drawFittedText(getName(), getLocalBounds().reduced(4, 2),
                     juce::Justification::centred, 2);

    g.setColour(juce::Colours::grey);
    g.drawRect(x, y, w, h);
  }

  void resized() {
    for (int i = 0; i < getNumChildComponents(); ++i) {
      if (PinComponent *const pc =
              dynamic_cast<PinComponent *>(getChildComponent(i))) {
        const int total = pc->isInput ? numIns : numOuts;
        const int index = pc->index == ttmm::FilterGraph::midiChannelNumber
                              ? (total - 1)
                              : pc->index;

        pc->setBounds(
            proportionOfWidth((1 + index) / (total + 1.0f)) - pinSize / 2,
            pc->isInput ? 0 : (getHeight() - pinSize), pinSize, pinSize);
      }
    }
  }

  void getPinPos(const int index, const bool isInput, float &x, float &y) {
    for (int i = 0; i < getNumChildComponents(); ++i) {
      if (PinComponent *const pc =
              dynamic_cast<PinComponent *>(getChildComponent(i))) {
        if (pc->index == index && isInput == pc->isInput) {
          x = getX() + pc->getX() + pc->getWidth() * 0.5f;
          y = getY() + pc->getY() + pc->getHeight() * 0.5f;
          break;
        }
      }
    }
  }

  void update() {
    const juce::AudioProcessorGraph::Node::Ptr f(graph.getNodeForId(filterID));

    if (f == nullptr) {
      delete this;
      return;
    }

    numIns = f->getProcessor()->getNumInputChannels();
    if (f->getProcessor()->acceptsMidi())
      ++numIns;

    numOuts = f->getProcessor()->getNumOutputChannels();
    if (f->getProcessor()->producesMidi())
      ++numOuts;

    int w = 100;
    int h = 60;

    w = juce::jmax(w, (juce::jmax(numIns, numOuts) + 1) * 20);

    const int textWidth = font.getStringWidth(f->getProcessor()->getName());
    w = juce::jmax(w, 16 + juce::jmin(textWidth, 300));
    if (textWidth > 300)
      h = 100;

    setSize(w, h);

    setName(f->getProcessor()->getName());

    {
      double x, y;
      graph.getNodePosition(filterID, x, y);
      setCentreRelative((float)x, (float)y);
    }

    if (numIns != numInputs || numOuts != numOutputs) {
      numInputs = numIns;
      numOutputs = numOuts;

      deleteAllChildren();

      int i;
      for (i = 0; i < f->getProcessor()->getNumInputChannels(); ++i)
        addAndMakeVisible(new PinComponent(graph, filterID, i, true));

      if (f->getProcessor()->acceptsMidi())
        addAndMakeVisible(new PinComponent(
            graph, filterID, ttmm::FilterGraph::midiChannelNumber, true));

      for (i = 0; i < f->getProcessor()->getNumOutputChannels(); ++i)
        addAndMakeVisible(new PinComponent(graph, filterID, i, false));

      if (f->getProcessor()->producesMidi())
        addAndMakeVisible(new PinComponent(
            graph, filterID, ttmm::FilterGraph::midiChannelNumber, false));

      resized();
    }
  }

  ttmm::FilterGraph &graph;
  const juce::uint32 filterID;
  int numInputs, numOutputs;

private:
  int pinSize;
  juce::Point<int> originalPos;
  juce::Font font;
  int numIns, numOuts;
  juce::DropShadowEffect shadow;

  ttmm::GraphEditorPanel *getGraphPanel() const noexcept {
    return findParentComponentOfClass<ttmm::GraphEditorPanel>();
  }

  FilterComponent(const FilterComponent &);
  FilterComponent &operator=(const FilterComponent &);
};
}
//==============================================================================
namespace ttmm {
class ConnectorComponent : public juce::Component,
                           public juce::SettableTooltipClient {
public:
  ConnectorComponent(ttmm::FilterGraph &graph_)
      : sourceFilterID(0), destFilterID(0), sourceFilterChannel(0),
        destFilterChannel(0), graph(graph_), lastInputX(0), lastInputY(0),
        lastOutputX(0), lastOutputY(0) {
    setAlwaysOnTop(true);
  }

  void setInput(const juce::uint32 sourceFilterID_,
                const int sourceFilterChannel_) {
    if (sourceFilterID != sourceFilterID_ ||
        sourceFilterChannel != sourceFilterChannel_) {
      sourceFilterID = sourceFilterID_;
      sourceFilterChannel = sourceFilterChannel_;
      update();
    }
  }

  void setOutput(const juce::uint32 destFilterID_,
                 const int destFilterChannel_) {
    if (destFilterID != destFilterID_ ||
        destFilterChannel != destFilterChannel_) {
      destFilterID = destFilterID_;
      destFilterChannel = destFilterChannel_;
      update();
    }
  }

  void dragStart(int x, int y) {
    lastInputX = (float)x;
    lastInputY = (float)y;
    resizeToFit();
  }

  void dragEnd(int x, int y) {
    lastOutputX = (float)x;
    lastOutputY = (float)y;
    resizeToFit();
  }

  void update() {
    float x1, y1, x2, y2;
    getPoints(x1, y1, x2, y2);

    if (lastInputX != x1 || lastInputY != y1 || lastOutputX != x2 ||
        lastOutputY != y2) {
      resizeToFit();
    }
  }

  void resizeToFit() {
    float x1, y1, x2, y2;
    getPoints(x1, y1, x2, y2);

    const juce::Rectangle<int> newBounds(
        (int)juce::jmin(x1, x2) - 4, (int)juce::jmin(y1, y2) - 4,
        (int)std::abs(x1 - x2) + 8, (int)std::abs(y1 - y2) + 8);

    if (newBounds != getBounds())
      setBounds(newBounds);
    else
      resized();

    repaint();
  }

  void getPoints(float &x1, float &y1, float &x2, float &y2) const {
    x1 = lastInputX;
    y1 = lastInputY;
    x2 = lastOutputX;
    y2 = lastOutputY;

    if (ttmm::GraphEditorPanel *const hostPanel = getGraphPanel()) {
      if (ttmm::FilterComponent *srcFilterComp =
              hostPanel->getComponentForFilter(sourceFilterID))
        srcFilterComp->getPinPos(sourceFilterChannel, false, x1, y1);

      if (ttmm::FilterComponent *dstFilterComp =
              hostPanel->getComponentForFilter(destFilterID))
        dstFilterComp->getPinPos(destFilterChannel, true, x2, y2);
    }
  }

  void paint(juce::Graphics &g) {
    if (sourceFilterChannel == ttmm::FilterGraph::midiChannelNumber ||
        destFilterChannel == ttmm::FilterGraph::midiChannelNumber) {
      g.setColour(juce::Colours::red);
    } else {
      g.setColour(juce::Colours::green);
    }

    g.fillPath(linePath);
  }

  bool hitTest(int x, int y) {
    if (hitPath.contains((float)x, (float)y)) {
      double distanceFromStart, distanceFromEnd;
      getDistancesFromEnds(x, y, distanceFromStart, distanceFromEnd);

      // avoid clicking the connector when over a pin
      return distanceFromStart > 7.0 && distanceFromEnd > 7.0;
    }

    return false;
  }

  void mouseDown(const juce::MouseEvent &) { dragging = false; }

  void mouseDrag(const juce::MouseEvent &e) {
    if ((!dragging) && !e.mouseWasClicked()) {
      dragging = true;

      graph.removeConnection(sourceFilterID, sourceFilterChannel, destFilterID,
                             destFilterChannel);

      double distanceFromStart, distanceFromEnd;
      getDistancesFromEnds(e.x, e.y, distanceFromStart, distanceFromEnd);
      const bool isNearerSource = (distanceFromStart < distanceFromEnd);

      getGraphPanel()->beginConnectorDrag(
          isNearerSource ? 0 : sourceFilterID, sourceFilterChannel,
          isNearerSource ? destFilterID : 0, destFilterChannel, e);
    } else if (dragging) {
      getGraphPanel()->dragConnector(e);
    }
  }

  void mouseUp(const juce::MouseEvent &e) {
    if (dragging)
      getGraphPanel()->endDraggingConnector(e);
  }

  void resized() {
    float x1, y1, x2, y2;
    getPoints(x1, y1, x2, y2);

    lastInputX = x1;
    lastInputY = y1;
    lastOutputX = x2;
    lastOutputY = y2;

    x1 -= getX();
    y1 -= getY();
    x2 -= getX();
    y2 -= getY();

    linePath.clear();
    linePath.startNewSubPath(x1, y1);
    linePath.cubicTo(x1, y1 + (y2 - y1) * 0.33f, x2, y1 + (y2 - y1) * 0.66f, x2,
                     y2);

    juce::PathStrokeType wideStroke(8.0f);
    wideStroke.createStrokedPath(hitPath, linePath);

    juce::PathStrokeType stroke(2.5f);
    stroke.createStrokedPath(linePath, linePath);

    const float arrowW = 5.0f;
    const float arrowL = 4.0f;

    juce::Path arrow;
    arrow.addTriangle(-arrowL, arrowW, -arrowL, -arrowW, arrowL, 0.0f);

    arrow.applyTransform(
        juce::AffineTransform::identity.rotated(juce::float_Pi * 0.5f -
                                                (float)atan2(x2 - x1, y2 - y1))
            .translated((x1 + x2) * 0.5f, (y1 + y2) * 0.5f));

    linePath.addPath(arrow);
    linePath.setUsingNonZeroWinding(true);
  }

  juce::uint32 sourceFilterID, destFilterID;
  int sourceFilterChannel, destFilterChannel;

private:
  ttmm::FilterGraph &graph;
  float lastInputX, lastInputY, lastOutputX, lastOutputY;
  juce::Path linePath, hitPath;
  bool dragging;

  ttmm::GraphEditorPanel *getGraphPanel() const noexcept {
    return findParentComponentOfClass<ttmm::GraphEditorPanel>();
  }

  void getDistancesFromEnds(int x, int y, double &distanceFromStart,
                            double &distanceFromEnd) const {
    float x1, y1, x2, y2;
    getPoints(x1, y1, x2, y2);

    distanceFromStart = juce::juce_hypot(x - (x1 - getX()), y - (y1 - getY()));
    distanceFromEnd = juce::juce_hypot(x - (x2 - getX()), y - (y2 - getY()));
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectorComponent)
};
}
//==============================================================================
ttmm::GraphEditorPanel::GraphEditorPanel(ttmm::FilterGraph &graph_)
    : graph(graph_) {
  graph.addChangeListener(this);
  setOpaque(true);
}

ttmm::GraphEditorPanel::~GraphEditorPanel() {
  graph.removeChangeListener(this);
  draggingConnector = nullptr;
  deleteAllChildren();
}

void ttmm::GraphEditorPanel::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::white);
}

void ttmm::GraphEditorPanel::mouseDown(const juce::MouseEvent &e) {
  if (e.mods.isPopupMenu()) {
    juce::PopupMenu m;

    if (ttmm::MainHostWindow *const mainWindow =
            findParentComponentOfClass<MainHostWindow>()) {

      mainWindow->addPluginsToMenu(m);
      const int r = m.show();

      createNewPlugin(mainWindow->getChosenType(r), e.x, e.y);
    }
  }
}

void ttmm::GraphEditorPanel::createNewPlugin(
    const juce::PluginDescription *desc, int x, int y) {

  graph.addFilter(desc, x / (double)getWidth(), y / (double)getHeight());
}

ttmm::FilterComponent *ttmm::GraphEditorPanel::getComponentForFilter(
    const juce::uint32 filterID) const {
  for (int i = getNumChildComponents(); --i >= 0;) {
    if (ttmm::FilterComponent *const fc =
            dynamic_cast<ttmm::FilterComponent *>(getChildComponent(i)))
      if (fc->filterID == filterID)
        return fc;
  }

  return nullptr;
}

ttmm::ConnectorComponent *ttmm::GraphEditorPanel::getComponentForConnection(
    const juce::AudioProcessorGraph::Connection &conn) const {
  for (int i = getNumChildComponents(); --i >= 0;) {
    if (ConnectorComponent *const c =
            dynamic_cast<ttmm::ConnectorComponent *>(getChildComponent(i)))
      if (c->sourceFilterID == conn.sourceNodeId &&
          c->destFilterID == conn.destNodeId &&
          c->sourceFilterChannel == conn.sourceChannelIndex &&
          c->destFilterChannel == conn.destChannelIndex)
        return c;
  }

  return nullptr;
}

ttmm::PinComponent *ttmm::GraphEditorPanel::findPinAt(const int x,
                                                      const int y) const {
  for (int i = getNumChildComponents(); --i >= 0;) {
    if (ttmm::FilterComponent *fc =
            dynamic_cast<ttmm::FilterComponent *>(getChildComponent(i))) {
      if (ttmm::PinComponent *pin = dynamic_cast<ttmm::PinComponent *>(
              fc->getComponentAt(x - fc->getX(), y - fc->getY())))
        return pin;
    }
  }

  return nullptr;
}

void ttmm::GraphEditorPanel::resized() { updateComponents(); }

void ttmm::GraphEditorPanel::changeListenerCallback(juce::ChangeBroadcaster *) {
  updateComponents();
}

void ttmm::GraphEditorPanel::updateComponents() {
  for (int i = getNumChildComponents(); --i >= 0;) {
    if (ttmm::FilterComponent *const fc =
            dynamic_cast<ttmm::FilterComponent *>(getChildComponent(i)))
      fc->update();
  }

  for (int i = getNumChildComponents(); --i >= 0;) {
    ttmm::ConnectorComponent *const cc =
        dynamic_cast<ttmm::ConnectorComponent *>(getChildComponent(i));

    if (cc != nullptr && cc != draggingConnector) {
      if (graph.getConnectionBetween(cc->sourceFilterID,
                                     cc->sourceFilterChannel, cc->destFilterID,
                                     cc->destFilterChannel) == nullptr) {
        delete cc;
      } else {
        cc->update();
      }
    }
  }

  for (int i = graph.getNumFilters(); --i >= 0;) {
    const juce::AudioProcessorGraph::Node::Ptr f(graph.getNode(i));

    if (getComponentForFilter(f->nodeId) == 0) {
      ttmm::FilterComponent *const comp =
          new ttmm::FilterComponent(graph, f->nodeId);
      addAndMakeVisible(comp);
      comp->update();
    }
  }

  for (int i = graph.getNumConnections(); --i >= 0;) {
    const juce::AudioProcessorGraph::Connection *const c =
        graph.getConnection(i);

    if (getComponentForConnection(*c) == 0) {
      ttmm::ConnectorComponent *const comp =
          new ttmm::ConnectorComponent(graph);
      addAndMakeVisible(comp);

      comp->setInput(c->sourceNodeId, c->sourceChannelIndex);
      comp->setOutput(c->destNodeId, c->destChannelIndex);
    }
  }
}

void ttmm::GraphEditorPanel::beginConnectorDrag(
    const juce::uint32 sourceFilterID, const int sourceFilterChannel,
    const juce::uint32 destFilterID, const int destFilterChannel,
    const juce::MouseEvent &e) {
  draggingConnector =
      dynamic_cast<ttmm::ConnectorComponent *>(e.originalComponent);

  if (draggingConnector == nullptr)
    draggingConnector = new ttmm::ConnectorComponent(graph);

  draggingConnector->setInput(sourceFilterID, sourceFilterChannel);
  draggingConnector->setOutput(destFilterID, destFilterChannel);

  addAndMakeVisible(draggingConnector);
  draggingConnector->toFront(false);

  dragConnector(e);
}

void ttmm::GraphEditorPanel::dragConnector(const juce::MouseEvent &e) {
  const juce::MouseEvent e2(e.getEventRelativeTo(this));

  if (draggingConnector != nullptr) {
    draggingConnector->setTooltip(juce::String::empty);

    int x = e2.x;
    int y = e2.y;

    if (ttmm::PinComponent *const pin = findPinAt(x, y)) {
      juce::uint32 srcFilter = draggingConnector->sourceFilterID;
      int srcChannel = draggingConnector->sourceFilterChannel;
      juce::uint32 dstFilter = draggingConnector->destFilterID;
      int dstChannel = draggingConnector->destFilterChannel;

      if (srcFilter == 0 && !pin->isInput) {
        srcFilter = pin->filterID;
        srcChannel = pin->index;
      } else if (dstFilter == 0 && pin->isInput) {
        dstFilter = pin->filterID;
        dstChannel = pin->index;
      }

      if (graph.canConnect(srcFilter, srcChannel, dstFilter, dstChannel)) {
        x = pin->getParentComponent()->getX() + pin->getX() +
            pin->getWidth() / 2;
        y = pin->getParentComponent()->getY() + pin->getY() +
            pin->getHeight() / 2;

        draggingConnector->setTooltip(pin->getTooltip());
      }
    }

    if (draggingConnector->sourceFilterID == 0)
      draggingConnector->dragStart(x, y);
    else
      draggingConnector->dragEnd(x, y);
  }
}

void ttmm::GraphEditorPanel::endDraggingConnector(const juce::MouseEvent &e) {
  if (draggingConnector == nullptr)
    return;

  draggingConnector->setTooltip(juce::String::empty);

  const juce::MouseEvent e2(e.getEventRelativeTo(this));

  juce::uint32 srcFilter = draggingConnector->sourceFilterID;
  int srcChannel = draggingConnector->sourceFilterChannel;
  juce::uint32 dstFilter = draggingConnector->destFilterID;
  int dstChannel = draggingConnector->destFilterChannel;

  draggingConnector = nullptr;

  if (ttmm::PinComponent *const pin = findPinAt(e2.x, e2.y)) {
    if (srcFilter == 0) {
      if (pin->isInput)
        return;

      srcFilter = pin->filterID;
      srcChannel = pin->index;
    } else {
      if (!pin->isInput)
        return;

      dstFilter = pin->filterID;
      dstChannel = pin->index;
    }

    graph.addConnection(srcFilter, srcChannel, dstFilter, dstChannel);
  }
}

namespace ttmm {
//==============================================================================
class TooltipBar : public juce::Component, private juce::Timer {
public:
  TooltipBar() { startTimer(100); }

  void paint(juce::Graphics &g) {
    g.setFont(juce::Font(getHeight() * 0.7f, juce::Font::bold));
    g.setColour(juce::Colours::black);
    g.drawFittedText(tip, 10, 0, getWidth() - 12, getHeight(),
                     juce::Justification::centredLeft, 1);
  }

  void timerCallback() {
    juce::Component *const underMouse = juce::Desktop::getInstance()
                                            .getMainMouseSource()
                                            .getComponentUnderMouse();
    juce::TooltipClient *const ttc =
        dynamic_cast<juce::TooltipClient *>(underMouse);

    juce::String newTip;

    if (ttc != nullptr &&
        !(underMouse->isMouseButtonDown() ||
          underMouse->isCurrentlyBlockedByAnotherModalComponent()))
      newTip = ttc->getTooltip();

    if (newTip != tip) {
      tip = newTip;
      repaint();
    }
  }

private:
  juce::String tip;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TooltipBar)
};
}

//==============================================================================
ttmm::GraphDocumentComponent::GraphDocumentComponent(
    juce::AudioPluginFormatManager &formatManager,
    juce::AudioDeviceManager *deviceManager_)
    : graph(formatManager), deviceManager(deviceManager_) {
  addAndMakeVisible(graphPanel = new ttmm::GraphEditorPanel(graph));

  deviceManager->addChangeListener(graphPanel);

  graphPlayer.setProcessor(&graph.getGraph());

  keyState.addListener(&graphPlayer.getMidiMessageCollector());

  addAndMakeVisible(
      keyboardComp = new juce::MidiKeyboardComponent(
          keyState, juce::MidiKeyboardComponent::horizontalKeyboard));

  addAndMakeVisible(statusBar = new ttmm::TooltipBar());

  deviceManager->addAudioCallback(&graphPlayer);
  deviceManager->addMidiInputCallback(juce::String::empty,
                                      &graphPlayer.getMidiMessageCollector());

  graphPanel->updateComponents();
}

ttmm::GraphDocumentComponent::~GraphDocumentComponent() {
  deviceManager->removeAudioCallback(&graphPlayer);
  deviceManager->removeMidiInputCallback(
      juce::String::empty, &graphPlayer.getMidiMessageCollector());
  deviceManager->removeChangeListener(graphPanel);

  deleteAllChildren();

  graphPlayer.setProcessor(nullptr);
  keyState.removeListener(&graphPlayer.getMidiMessageCollector());

  graph.clear();
}

void ttmm::GraphDocumentComponent::resized() {
  const int keysHeight = 60;
  const int statusHeight = 20;

  graphPanel->setBounds(0, 0, getWidth(), getHeight() - keysHeight);
  statusBar->setBounds(0, getHeight() - keysHeight - statusHeight, getWidth(),
                       statusHeight);
  keyboardComp->setBounds(0, getHeight() - keysHeight, getWidth(), keysHeight);
}

void ttmm::GraphDocumentComponent::createNewPlugin(
    const juce::PluginDescription *desc, int x, int y) {
  graphPanel->createNewPlugin(desc, x, y);
}
