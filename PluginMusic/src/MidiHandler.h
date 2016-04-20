/**
* @file MidiHandler.cpp
* @author FH-Minden Musikinformatik 2015, Musicgroup (Marcel, Patrick, Scott)
* @brief The class is used to calculate the new Midievents based on the new inputs from Kinect- & Drum groups
*
*/
#ifndef TTMM_MIDI_HANDLER_H
#define TTMM_MIDI_HANDLER_H

#include <iostream>
#include <fstream>
#include <chrono>

#include "../Model/Song.h"
#include "../../TTMM/JuceLibraryCode/JuceHeader.h"
#include "DataExchange.pb.h"

namespace ttmm
{
using std::vector;
using std::map;

/**
	* Enum type for Tune including the Main, FirstAccompany, SecondAccompany
	*/
enum Tune
{
    Main,
    FirstAccompany,
    SecondAccompany
};

/**
	* @class MidiHandler
	* @brief The class is used to calculate the new Midievents based on the new inputs from Kinect- & Drum groups
	*
	* @see MidiBuffer, MidiMessage, Song, Globalobject
	*/
class MidiHandler
{
public:
    /**
		* Constructor: create a MidiHandler
		*/
    MidiHandler();
    /**
		* Destructor
		*/
    ~MidiHandler();
    /**
		* Read the new inputs from Intern Process Protocol
		*/
    void getInputs();
    /**
		* Update the new inputs to Midistructurs and add to MidiBuffer.
		* Then send it to the another group
		* Update timestamp of event (plus the real time)
		*/
    void setMidiBufferOutputs();
    /*
		* @param tune which tune has to be played
		* @param isOn Indicator whether noteOn or noteOff is executed
		* @param key The key of the current stroke (Tonart)
		* @param The note that has to be played
		* @param midiBuffer a reference to a MidiBuffer
		* @param channelNr The channelnumber
		* @param playtime a double value at the currenttime on host
		* @param numSamples buffersize
		* @param newVelocity the new value of Velocity for updating the volume of Node
		* @param secondOfBlock nummer seconds of each block based on the buffernumber and samplerate
		*/
    void MidiHandler::playNote(Tune tune, bool isOn, int key, Node note, juce::MidiBuffer& midiBuffer,
        int channelNr, double& playtime, int numSamples, int newVelocity, double secondOfBlock);
    /**
		* Read a track in Song and update it to MidiBuffer for exchange with the another group
		* 
		* @param stroke a reference to a track in Song
		* @param newMidiBuffer a reference to a MidiBuffer
		* @param playtime a double value at the currenttime on host
		* @param secondEachBlock a double value about the number of seconds for a block (buffersize/samplerate, e.g: 2560 / 44100)
		* @param numSamples buffersize
		* @param musician a object of IPCSongInfo_IPCMusician
		*/
    void processTrackToNewMidiBuffer(Track* track, juce::MidiBuffer& newMidiBuffer,
        double& playtime, double secondEachBlock, int numSamples, IPCSongInfo_IPCMusician musician);

    /**
	*
	*/
private:
    bool isStartOfMetronom = false; //keep the status whether the startTime for metronom is determined
    double startOfMetronom = 0; //keep value of the startTime of Metronom
    double durationOfQuarter = 0;

    int sample2Notes = 0;
    double time2Notes = 0;

    int count = 0; ///<keep the status whether noteOff of Tone is added to buffer
};
}
#endif