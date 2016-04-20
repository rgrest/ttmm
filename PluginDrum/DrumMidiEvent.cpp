//*******************************
// DrumDevice					*
// Datei: DrumDevice.cpp		*
// Methoden der Klasse			*
// Ricco Schönebeck 07.05.2015	*
//*******************************

#include <cstdint>
#include <iostream>
#include <windows.h>
#include <sstream>

#include "DrumMidiEvent.h"
#include "TimeTools.h"

ttmm::DrumMidiEvent::DrumMidiEvent(ttmm::MidiStatus status, ttmm::MidiNote note,
    ttmm::MidiVelocity velocity)
    : status(status)
    , note(note)
    , velocity(velocity)
{
}

std::string ttmm::toString(DrumMidiEvent const& dme)
{
    std::stringstream stream;
    stream << std::string("Note: ") << dme.note + " Vel: " << dme.velocity << " Time: "
           << std::to_string(TimeInfo::timeInfo().ms(dme.timestamp));
    return stream.str();
}