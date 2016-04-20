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
#include "InternalFilters.h"
#include "FilterGraph.h"

//==============================================================================
ttmm::InternalPluginFormat::InternalPluginFormat() {
  {
    juce::AudioProcessorGraph::AudioGraphIOProcessor p(
        juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
    p.fillInPluginDescription(audioOutDesc);
  }

  {
    juce::AudioProcessorGraph::AudioGraphIOProcessor p(
        juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
    p.fillInPluginDescription(audioInDesc);
  }

  {
    juce::AudioProcessorGraph::AudioGraphIOProcessor p(
        juce::AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
    p.fillInPluginDescription(midiInDesc);
  }

  {
	juce::AudioProcessorGraph::AudioGraphIOProcessor p(
		juce::AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode);
	p.fillInPluginDescription(midiOutDesc);
  }
}

juce::AudioPluginInstance *
ttmm::InternalPluginFormat::createInstanceFromDescription(
    const juce::PluginDescription &desc, double /*sampleRate*/,
    int /*blockSize*/) {
  if (desc.name == audioOutDesc.name)
    return new juce::AudioProcessorGraph::AudioGraphIOProcessor(
        juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);

  if (desc.name == audioInDesc.name)
    return new juce::AudioProcessorGraph::AudioGraphIOProcessor(
        juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);

  if (desc.name == midiInDesc.name)
    return new juce::AudioProcessorGraph::AudioGraphIOProcessor(
        juce::AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);

  if (desc.name == midiOutDesc.name)
	return new juce::AudioProcessorGraph::AudioGraphIOProcessor(
	juce::AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode);
  return 0;
}

const juce::PluginDescription *
ttmm::InternalPluginFormat::getDescriptionFor(const InternalFilterType type) {
  switch (type) {
  case audioInputFilter:
    return &audioInDesc;
  case audioOutputFilter:
    return &audioOutDesc;
  case midiInputFilter:
    return &midiInDesc;
  case midiOutputFilter:      
	  return &midiOutDesc;
  default:
    break;
  }

  return 0;
}

void ttmm::InternalPluginFormat::getAllTypes(
    juce::OwnedArray<juce::PluginDescription> &results) {
  for (int i = 0; i < (int)endOfFilterTypes; ++i)
    results.add(
        new juce::PluginDescription(*getDescriptionFor((InternalFilterType)i)));
}
