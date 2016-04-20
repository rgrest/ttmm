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
#include "FilterGraph.h"
#include "InternalFilters.h"
#include "GraphEditorPanel.h"

//#include "GeneralPluginProcessor.h"

//==============================================================================
const int ttmm::FilterGraph::midiChannelNumber = 0x1000;

ttmm::FilterGraph::FilterGraph(juce::AudioPluginFormatManager &formatManager_)
    : juce::FileBasedDocument(filenameSuffix, filenameWildcard,
                              "Load a filter graph", "Save a filter graph"),
      formatManager(formatManager_), lastUID(0) {
  InternalPluginFormat internalFormat;

  addFilter(
      internalFormat.getDescriptionFor(InternalPluginFormat::audioInputFilter),
      0.5f, 0.1f);
  addFilter(
      internalFormat.getDescriptionFor(InternalPluginFormat::midiInputFilter),
      0.25f, 0.1f);
  addFilter(
      internalFormat.getDescriptionFor(InternalPluginFormat::audioOutputFilter),
      0.5f, 0.9f);

  setChangedFlag(false);
}

ttmm::FilterGraph::~FilterGraph() { graph.clear(); }

juce::uint32 ttmm::FilterGraph::getNextUID() noexcept { return ++lastUID; }

//==============================================================================
int ttmm::FilterGraph::getNumFilters() const noexcept {
  return graph.getNumNodes();
}

const juce::AudioProcessorGraph::Node::Ptr
ttmm::FilterGraph::getNode(const int index) const noexcept {
  return graph.getNode(index);
}

const juce::AudioProcessorGraph::Node::Ptr
ttmm::FilterGraph::getNodeForId(const juce::uint32 uid) const noexcept {
  return graph.getNodeForId(uid);
}

void ttmm::FilterGraph::addFilter(const juce::PluginDescription *desc, double x,
						   double y) {

  auto processor = (juce::AudioProcessor*)(nullptr);

  if (desc != nullptr) {
    juce::AudioProcessorGraph::Node *node = nullptr;

    juce::String errorMessage;

    if (juce::AudioPluginInstance *instance =
            formatManager.createPluginInstance(*desc, graph.getSampleRate(),
                                               graph.getBlockSize(),
                                               errorMessage))
      node = graph.addNode(instance);

    if (node != nullptr) {
      node->properties.set("x", x);
      node->properties.set("y", y);
      changed();

    } else {
      juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon,
                                        TRANS("Couldn't create filter"),
                                        errorMessage);
    }
  }
}

void ttmm::FilterGraph::removeFilter(const juce::uint32 id) {
  PluginWindow::closeCurrentlyOpenWindowsFor(id);

  if (graph.removeNode(id))
    changed();
}

void ttmm::FilterGraph::disconnectFilter(const juce::uint32 id) {
  if (graph.disconnectNode(id))
    changed();
}

void ttmm::FilterGraph::removeIllegalConnections() {
  if (graph.removeIllegalConnections())
    changed();
}

void ttmm::FilterGraph::setNodePosition(const int nodeId, double x, double y) {
  const juce::AudioProcessorGraph::Node::Ptr n(graph.getNodeForId(nodeId));

  if (n != nullptr) {
    n->properties.set("x", juce::jlimit(0.0, 1.0, x));
    n->properties.set("y", juce::jlimit(0.0, 1.0, y));
  }
}

void ttmm::FilterGraph::getNodePosition(const int nodeId, double &x,
                                        double &y) const {
  x = y = 0;

  const juce::AudioProcessorGraph::Node::Ptr n(graph.getNodeForId(nodeId));

  if (n != nullptr) {
    x = (double)n->properties["x"];
    y = (double)n->properties["y"];
  }
}

//==============================================================================
int ttmm::FilterGraph::getNumConnections() const noexcept {
  return graph.getNumConnections();
}

const juce::AudioProcessorGraph::Connection *
ttmm::FilterGraph::getConnection(const int index) const noexcept {
  return graph.getConnection(index);
}

const juce::AudioProcessorGraph::Connection *
ttmm::FilterGraph::getConnectionBetween(juce::uint32 sourceFilterUID,
                                        int sourceFilterChannel,
                                        juce::uint32 destFilterUID,
                                        int destFilterChannel) const noexcept {
  return graph.getConnectionBetween(sourceFilterUID, sourceFilterChannel,
                                    destFilterUID, destFilterChannel);
}

bool ttmm::FilterGraph::canConnect(juce::uint32 sourceFilterUID,
                                   int sourceFilterChannel,
                                   juce::uint32 destFilterUID,
                                   int destFilterChannel) const noexcept {
  return graph.canConnect(sourceFilterUID, sourceFilterChannel, destFilterUID,
                          destFilterChannel);
}

bool ttmm::FilterGraph::addConnection(juce::uint32 sourceFilterUID,
                                      int sourceFilterChannel,
                                      juce::uint32 destFilterUID,
                                      int destFilterChannel) {
  const bool result = graph.addConnection(sourceFilterUID, sourceFilterChannel,
                                          destFilterUID, destFilterChannel);

  if (result)
    changed();

  return result;
}

void ttmm::FilterGraph::removeConnection(const int index) {
  graph.removeConnection(index);
  changed();
}

void ttmm::FilterGraph::removeConnection(juce::uint32 sourceFilterUID,
                                         int sourceFilterChannel,
                                         juce::uint32 destFilterUID,
                                         int destFilterChannel) {
  if (graph.removeConnection(sourceFilterUID, sourceFilterChannel,
                             destFilterUID, destFilterChannel))
    changed();
}

void ttmm::FilterGraph::clear() {
  PluginWindow::closeAllCurrentlyOpenWindows();

  graph.clear();
  changed();
}

//==============================================================================
juce::String ttmm::FilterGraph::getDocumentTitle() {
  if (!getFile().exists())
    return "Unnamed";

  return getFile().getFileNameWithoutExtension();
}

juce::Result ttmm::FilterGraph::loadDocument(const juce::File &file) {
  juce::XmlDocument doc(file);
  juce::ScopedPointer<juce::XmlElement> xml(doc.getDocumentElement());

  if (xml == nullptr || !xml->hasTagName("FILTERGRAPH"))
    return juce::Result::fail("Not a valid filter graph file");

  restoreFromXml(*xml);
  return juce::Result::ok();
}

juce::Result ttmm::FilterGraph::saveDocument(const juce::File &file) {
  juce::ScopedPointer<juce::XmlElement> xml(createXml());

  if (!xml->writeToFile(file, juce::String::empty))
    return juce::Result::fail("Couldn't write to the file");

  return juce::Result::ok();
}

juce::File ttmm::FilterGraph::getLastDocumentOpened() {
  juce::RecentlyOpenedFilesList recentFiles;
  recentFiles.restoreFromString(
      getAppProperties().getUserSettings()->getValue("recentFilterGraphFiles"));

  return recentFiles.getFile(0);
}

void ttmm::FilterGraph::setLastDocumentOpened(const juce::File &file) {
  juce::RecentlyOpenedFilesList recentFiles;
  recentFiles.restoreFromString(
      getAppProperties().getUserSettings()->getValue("recentFilterGraphFiles"));

  recentFiles.addFile(file);

  getAppProperties().getUserSettings()->setValue("recentFilterGraphFiles",
                                                 recentFiles.toString());
}

//==============================================================================
static juce::XmlElement *
createNodeXml(juce::AudioProcessorGraph::Node *const node) noexcept {
  juce::AudioPluginInstance *plugin =
      dynamic_cast<juce::AudioPluginInstance *>(node->getProcessor());

  if (plugin == nullptr) {
    jassertfalse;
    return nullptr;
  }

  juce::XmlElement *e = new juce::XmlElement("FILTER");
  e->setAttribute("uid", (int)node->nodeId);
  e->setAttribute("x", node->properties["x"].toString());
  e->setAttribute("y", node->properties["y"].toString());
  e->setAttribute("uiLastX", node->properties["uiLastX"].toString());
  e->setAttribute("uiLastY", node->properties["uiLastY"].toString());

  juce::PluginDescription pd;
  plugin->fillInPluginDescription(pd);

  e->addChildElement(pd.createXml());

  juce::XmlElement *state = new juce::XmlElement("STATE");

  juce::MemoryBlock m;
  node->getProcessor()->getStateInformation(m);
  state->addTextElement(m.toBase64Encoding());
  e->addChildElement(state);

  return e;
}

void ttmm::FilterGraph::createNodeFromXml(const juce::XmlElement &xml) {
  juce::PluginDescription pd;

  forEachXmlChildElement(xml, e) {
    if (pd.loadFromXml(*e))
      break;
  }

  juce::String errorMessage;

  juce::AudioPluginInstance *instance = formatManager.createPluginInstance(
      pd, graph.getSampleRate(), graph.getBlockSize(), errorMessage);

  if (instance == nullptr) {
    // xxx handle ins + outs
  }

  if (instance == nullptr)
    return;

  juce::AudioProcessorGraph::Node::Ptr node(
      graph.addNode(instance, xml.getIntAttribute("uid")));

  if (const juce::XmlElement *const state = xml.getChildByName("STATE")) {
    juce::MemoryBlock m;
    m.fromBase64Encoding(state->getAllSubText());

    node->getProcessor()->setStateInformation(m.getData(), (int)m.getSize());
  }

  node->properties.set("x", xml.getDoubleAttribute("x"));
  node->properties.set("y", xml.getDoubleAttribute("y"));
  node->properties.set("uiLastX", xml.getIntAttribute("uiLastX"));
  node->properties.set("uiLastY", xml.getIntAttribute("uiLastY"));
}

juce::XmlElement *ttmm::FilterGraph::createXml() const {
  juce::XmlElement *xml = new juce::XmlElement("FILTERGRAPH");

  for (int i = 0; i < graph.getNumNodes(); ++i)
    xml->addChildElement(createNodeXml(graph.getNode(i)));

  for (int i = 0; i < graph.getNumConnections(); ++i) {
    const juce::AudioProcessorGraph::Connection *const fc =
        graph.getConnection(i);

    juce::XmlElement *e = new juce::XmlElement("CONNECTION");

    e->setAttribute("srcFilter", (int)fc->sourceNodeId);
    e->setAttribute("srcChannel", fc->sourceChannelIndex);
    e->setAttribute("dstFilter", (int)fc->destNodeId);
    e->setAttribute("dstChannel", fc->destChannelIndex);

    xml->addChildElement(e);
  }

  return xml;
}

void ttmm::FilterGraph::restoreFromXml(const juce::XmlElement &xml) {
  clear();

  forEachXmlChildElementWithTagName(xml, e, "FILTER") {
    createNodeFromXml(*e);
    changed();
  }

  forEachXmlChildElementWithTagName(xml, e, "CONNECTION") {
    addConnection((juce::uint32)e->getIntAttribute("srcFilter"),
                  e->getIntAttribute("srcChannel"),
                  (juce::uint32)e->getIntAttribute("dstFilter"),
                  e->getIntAttribute("dstChannel"));
  }

  graph.removeIllegalConnections();
}
