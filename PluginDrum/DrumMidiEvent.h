/**
* @file DrumMidiEvent.h
* @author FH-Minden Musikinformatik 2015, Drumgroup (Ricco Schoenebeck, Holger Heeren, Philipp Kroos)
* @brief Holds the data of a Drum-Midi-Event (Status-Byte, Data-Byte 1, Data-Byte 2, Timestamp)
*
*/

#include "Types.h"
#include "Buffer.h"

#ifndef DRUM_MIDI_EVENT_H
#define DRUM_MIDI_EVENT_H

namespace ttmm
{
/**
 @class DrumMidiEvent
 @brief Holds the data of a Drum-Midi-Event (Status-Byte, Data-Byte 1, Data-Byte 2, Timestamp)

 @see DrumDevice, DrumMusician, DrumInputPluginProcessor
*/
struct DrumMidiEvent : public BufferData
{

    /**The Status-Byte of a Midi-Event (Contains: Midi-Command & Midi-Channel)*/
    MidiStatus status;

    /**The Data-Byte 1 / Note-Byte of a Midi-Event (Contains the specified Note of a played Drum)*/
    MidiNote note;

    /**The Data-Byte 2 / Velocity-Byte of a Midi-Event (Contains the Velocity of a played Drum)*/
    MidiVelocity velocity;

    /**
      Default Constructor
     */
    DrumMidiEvent() = default;

    /**
     Constructor
     @param status - The incoming Midi StatusByte
     @param status - The incoming Midi DataByte 2 (Velocity)
     @param status - The incoming Midi DataByte 1 (note)
    */
    DrumMidiEvent(MidiStatus status, MidiVelocity velocity, MidiNote note);
};

std::string toString(DrumMidiEvent const& event);
}
#endif