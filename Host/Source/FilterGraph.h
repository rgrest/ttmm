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

#include<vector>
#include<string>

//#include "SongInfo.h"

#ifndef __FILTERGRAPH_JUCEHEADER__
#define __FILTERGRAPH_JUCEHEADER__

namespace ttmm {

class FilterInGraph;
class FilterGraph;

const char *const filenameSuffix = ".filtergraph";
const char *const filenameWildcard = "*.filtergraph";

//==============================================================================
/**
        A collection of filters and some connections between them.
        */
class FilterGraph : public juce::FileBasedDocument {
public:
  //==============================================================================
  FilterGraph(juce::AudioPluginFormatManager &formatManager);
  ~FilterGraph();

  //==============================================================================
  juce::AudioProcessorGraph &getGraph() noexcept { return graph; }

  int getNumFilters() const noexcept;
  const juce::AudioProcessorGraph::Node::Ptr getNode(const int index) const
      noexcept;
  const juce::AudioProcessorGraph::Node::Ptr
  getNodeForId(const juce::uint32 uid) const noexcept;

  void addFilter(const juce::PluginDescription *desc, double x, double y);

  void removeFilter(const juce::uint32 filterUID);
  void disconnectFilter(const juce::uint32 filterUID);

  void removeIllegalConnections();

  void setNodePosition(const int nodeId, double x, double y);
  void getNodePosition(const int nodeId, double &x, double &y) const;

  //==============================================================================
  int getNumConnections() const noexcept;
  const juce::AudioProcessorGraph::Connection *
  getConnection(const int index) const noexcept;

  const juce::AudioProcessorGraph::Connection *
  getConnectionBetween(juce::uint32 sourceFilterUID, int sourceFilterChannel,
                       juce::uint32 destFilterUID, int destFilterChannel) const
      noexcept;

  bool canConnect(juce::uint32 sourceFilterUID, int sourceFilterChannel,
                  juce::uint32 destFilterUID, int destFilterChannel) const
      noexcept;

  bool addConnection(juce::uint32 sourceFilterUID, int sourceFilterChannel,
                     juce::uint32 destFilterUID, int destFilterChannel);

  void removeConnection(const int index);

  void removeConnection(juce::uint32 sourceFilterUID, int sourceFilterChannel,
                        juce::uint32 destFilterUID, int destFilterChannel);

  void clear();

  //==============================================================================

  juce::XmlElement *createXml() const;
  void restoreFromXml(const juce::XmlElement &xml);

  //==============================================================================
  juce::String getDocumentTitle();
  juce::Result loadDocument(const juce::File &file);
  juce::Result saveDocument(const juce::File &file);
  juce::File getLastDocumentOpened();
  void setLastDocumentOpened(const juce::File &file);

  /** The special channel index used to refer to a filter's midi channel.
  */
  static const int midiChannelNumber;

private:
  //// Stupid VS2013 doesn't support cpp11-initializer-syntax causing this
  //// workaround
  //std::vector<std::string> ttmmPlugins =
  //    std::vector<std::string>{"PluginDrum", "PluginMusic", "PluginKinect"};
  // SongInfo& songInfo = SongInfo::getInstance(); ///< Made available available to all ttmm-plugins in
  /// createNewPlugin

  //==============================================================================
  juce::AudioPluginFormatManager &formatManager;
  juce::AudioProcessorGraph graph;

  juce::uint32 lastUID;
  juce::uint32 getNextUID() noexcept;

  void createNodeFromXml(const juce::XmlElement &xml);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterGraph)
};
}
#endif // __FILTERGRAPH_JUCEHEADER__
