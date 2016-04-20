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

#ifndef __INTERNALFILTERS_JUCEHEADER__
#define __INTERNALFILTERS_JUCEHEADER__

#include "FilterGraph.h"

namespace ttmm {

//==============================================================================
/**
        Manages the internal plugin types.
        */
class InternalPluginFormat : public juce::AudioPluginFormat {
public:
  //==============================================================================
  InternalPluginFormat();
  ~InternalPluginFormat() {}

  //==============================================================================
  enum InternalFilterType {
    audioInputFilter = 0,
    audioOutputFilter,
    midiInputFilter,
	midiOutputFilter,

    endOfFilterTypes
  };

  const juce::PluginDescription *
  getDescriptionFor(const InternalFilterType type);

  void getAllTypes(juce::OwnedArray<juce::PluginDescription> &results);

  //==============================================================================
  juce::String getName() const override { return "Internal"; }
  bool fileMightContainThisPluginType(const juce::String &) override {
    return false;
  }
  juce::FileSearchPath getDefaultLocationsToSearch() override {
    return juce::FileSearchPath();
  }
  bool canScanForPlugins() const override { return false; }
  void findAllTypesForFile(juce::OwnedArray<juce::PluginDescription> &,
                           const juce::String &) override {}
  bool doesPluginStillExist(const juce::PluginDescription &) override {
    return true;
  }
  juce::String
  getNameOfPluginFromIdentifier(const juce::String &fileOrIdentifier) override {
    return fileOrIdentifier;
  }
  bool pluginNeedsRescanning(const juce::PluginDescription &) override {
    return false;
  }
  juce::StringArray searchPathsForPlugins(const juce::FileSearchPath &,
                                          bool) override {
    return juce::StringArray();
  }
  juce::AudioPluginInstance *
  createInstanceFromDescription(const juce::PluginDescription &, double,
                                int) override;

private:
  //==============================================================================
  juce::PluginDescription audioInDesc;
  juce::PluginDescription audioOutDesc;
  juce::PluginDescription midiInDesc;
  juce::PluginDescription midiOutDesc;
};
}
#endif // __INTERNALFILTERS_JUCEHEADER__
