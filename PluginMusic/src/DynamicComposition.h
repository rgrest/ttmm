/**
* @file DynamicComposition.cpp
* @author FH-Minden Musikinformatik 2015, Musicgroup (Marcel, Patrick, Scott)
* @brief Encapsulates a plugin
*
*/
#ifndef TTMM_Dynamic_Composition_H
#define TTMM_Dynamic_Composition_H

#include "FileWriter.h"
#include "GeneralPluginProcessor.h"
#include "../Model/Song.h"
#include "../src/MidiReader.h"
#include "../src/MidiHandler.h"
#include "IPCConnection.h"
#include "TimeTools.h"

namespace ttmm
{

/**
	* @class DynamicComposition
	* @brief inherits to the class GeneralPluginProcessor and supports to process the block events between the plugins on Host, 
				to put mididata into the structures for postprocessing,
				to update Midibuffer with the new input 
				to send new MidiBuffer to another groups
	*
	* @see MidiReader, MidiHandler, Song, MidiBuffer, GlobalObject
	*/
class DynamicComposition : public GeneralPluginProcessor
{
public:
    const std::string PIPE_NAME = "18cmPENIS";
    /**
		* Constructor: create a DynamicComposition
		*/
    DynamicComposition();
    /**
		* Destructor
		*/
    ~DynamicComposition();
    /**
		* override the virtual function of the class Audio processeor
		* 
		* @param sampleRate a double argument
		*/
    void initializePlugin(Samplerate sampleRate) final override
    {
        this->samplerate = sampleRate;
        // read Midifile to Songobj
        songname = this->mReader.readToSong(this->song);
        ttmm::logfileMusic->write("read mididata to Song structure");
        //generate track
        for (int i = 0; i < 4; i++)
        {
            this->song.generateTrack(0);
        }
        ttmm::logfileMusic->write("generated track 0th of Song");
        // for test whether or not we have read Midifile to Songobj exactly
        this->song.readAllTracks();
        ttmm::logfileMusic->write("printed out the song structure");
        ttmm::logfileMusic->write("Playtime of plugin music at: ", this->playTime);
        //std::cout << "Samplerate: " << this->samplerate << std::endl;

        //fill the songinfo object with the default values
        this->localmusician = this->songInfo.add_musician();
        auto musician = this->localmusician;
        musician->set_accuracy(0);
        musician->set_tune(IPCSongInfo_IPCMusician_Tune::IPCSongInfo_IPCMusician_Tune_MIDDLE_UP);
        musician->set_volumech1(121);
        musician->set_volumech2(77);
        musician->set_volumech3(50);
        connection.createPipe(PIPE_NAME, 1000);

        //save the playTime and system time of plugin music to a file, when pluginMusic starts
        std::ofstream fstartPluginMusic;
        fstartPluginMusic.open("bigbang.txt", std::ios::out);
        auto timebase = TimeInfo::timeInfo().realtimeInNanoseconds();
        fstartPluginMusic << timebase << " " << this->playTime << std::endl;
        fstartPluginMusic.close();
    }
    /**
		* override the virtual function of the class Audio processeor
		* 
		*/
    void shutdown() final override
    {
        // Disconnecting would be obvious and shouldn't do harm - but this causes a memory leak.
        // The connection is closed anyway in the destructor of IPCConnection.
        // connection.disconnect();
    }
    /**
		* override the virtual function of the class Audio processeor and process the block datas between the plugins on host
		* 
		* @param audioBuffer a reference to AudioBuffer
		* @param midiBuffer a reference to MidiBuffer
		*/
    void processAudioAndMidiSignals(AudioBuffer& audioBuffer,
        juce::MidiBuffer& midiBuffer) final override;
    /**
		* merge musicians from IPC to local Songinfo
		*/
    void mergeMusicians();
    /**
		* override the virtual function of the class GeneralPluginProcessor
		*
		* @param object a SongInfo object received through IPC
		*/
    void receivedIPC(IPCSongInfo object) override final;
	String getSongName() {
		return songname;
	}
	String getSongTempo() {
		return std::to_string(song.getBpm());
	}

	juce::AudioProcessorEditor* createEditor() override; //<create custom UI for drum plugin
	bool hasEditor() const override { return true; }

private:
    ttmm::MidiReader mReader; ///<a MidiReader object
	String songname; ///<the name of the current song (filename)
    ttmm::Song song = NULL; ///<a Song object
    ttmm::MidiHandler mHandler; ///< a MidiHandler object
    double playTime = -5; ///<calculate the playtime of host based on Samplenumber (Buffersize) and Samplerate (44100)
    AudioBuffer wavdata; ///<store the data of wav sound for creating metronom
    std::vector<juce::MidiMessageSequence> buffers; ///<a list of midiSequences object
    ttmm::Samplerate samplerate; ///<samplerate of host

    IPCConnection connection; ///<init a IPC connection
    IPCSongInfo songInfo; ///<Merged Parameters from other Plugins
    IPCSongInfo_IPCMusician* localmusician;
    std::vector<IPCSongInfo_IPCMusician> musiciansToMerge; ///<Musicians from IPC to merge

    TIMER_REGISTER("TimerMusic")
};
}
#endif
