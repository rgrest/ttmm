/**
* @file MidiReader.cpp
* @author FH-Minden Musikinformatik 2015, Musicgroup (Marcel, Patrick, Scott)
* @brief The class is used to read the Mididata to a list of MidiBuffers or a Song structure
*
*/
#ifndef TTMM_MIDI_READER_H
#define TTMM_MIDI_READER_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <Windows.h>
#include <string>
#include <sstream>

#include "FileWriter.h"
#include "../Model/Song.h"
#include "../../TTMM/JuceLibraryCode/JuceHeader.h"

using std::string;
using std::ifstream;
namespace ttmm
{
/**
	* @class MidiReader
	* @brief The class is used to read the Mididata to a list of MidiBuffers or a Song structure
	*
	* @see Song, MidiBuffer, MidiFile, MidiSequence, MidiMessage
	*/
class MidiReader
{
public:
    /**
		* Constructor: create a MidiReader
		*/
    MidiReader();
    /**
		* Constructor: create a MidiReader
		*
		* @param songname the name of the song file
		*/
    MidiReader(string songname);
    /**
		* Destructor
		*/
    ~MidiReader();
    /**
		* read the mididata and write to the Song-reference
		* 
		* @param song a reference to song object
		*/
    String readToSong(Song& song);

    /**
	*
	*/
private:
    juce::File* file = nullptr; ///<a file object
    string songname; ///<a name of a song file
    double minuteToSeconds = 60;
    double start = 0.001;
    double tolerantOfTone = 0.001;
    double tolerantOfNote = 0.01;
    juce::MidiFile fileMidi; ///<a midifile object
    /**
		* Read the KeySignature (tone), TimeSignature (StrokeType), TempoSignature and put into the references respectively
		* 
		* @param seq a const pointer of MidiMessageSequence
		* @param kS a reference to key
		* @param mm a reference to major/minor
		* @param nm a reference to nominator
		* @param dn a reference to denominator
		* @param tl a reference to tick length of tempo meta event
		* @param spqn a reference to seconds per quarternote of Musicfile
		* @param timeformat a short const reference timeformat bit per minute 
		*/
    void readSignatureInfos(juce::MidiMessageSequence const* seq,
        int& kS, int& mm, int& nm, int& dn,
        double& tl, double& spqn, short const& timeformat);
    /**
		* Convert the Midievents to the node/ aftertouch events in Channel and put into the Stroke-reference
		* 
		* @param seq a reference to MidiMessageSequence
		* @param track a reference to track
		* @param secondperqn seconds per quarter note based on Musicfile
		*/
    void convertToTrack(juce::MidiMessageSequence const& seq, Track& track, double secondperqn);
};
}
#endif