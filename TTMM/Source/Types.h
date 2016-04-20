#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <chrono>

#include "../JuceLibraryCode/JuceHeader.h"

namespace ttmm {

using AudioSignalType = float;
using Samplerate = float;
using Volume = float;

// standard clock
// using Clock = system_clock;
// greater accuracy
using Clock = std::chrono::high_resolution_clock;
using Timestamp = Clock::time_point;
using Duration = Clock::duration;

using MidiStatus = std::uint8_t;
using MidiNote = std::uint8_t;
using MidiVelocity = std::uint8_t;

using AudioBuffer = struct {
  AudioSignalType *channel1;
  AudioSignalType *channel2;
  size_t samples;
};
using MidiBuffer = juce::MidiBuffer;


}

#endif