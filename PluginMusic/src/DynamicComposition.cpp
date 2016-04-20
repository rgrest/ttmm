#include "DynamicComposition.h"
#include "TimeTools.h"
#include "MusicPluginEditor.h"

using namespace ttmm;

//constructor for this plugin
ttmm::DynamicComposition::DynamicComposition()
    : GeneralPluginProcessor("Dynamic Composition", 0)
    , connection{ this, true }
{
}

//destructor for this plugin
ttmm::DynamicComposition::~DynamicComposition()
{
	// Disconnecting would be obvious and shouldn't do harm - but this causes a memory leak.
	// The connection is closed anyway in the destructor of IPCConnection.
	// connection.disconnect();
	//this->song.readAllTracks();
	//ttmm::logfileMusic->write("printed out the song structure after processing");
}

//this function helps us to contact with host
void ttmm::DynamicComposition::processAudioAndMidiSignals(
    AudioBuffer& audioBuffer,
    juce::MidiBuffer& midiMessages)
{
    //call this for each block to merge IPC musicians to local musician
    //then get this result for updating the notes
    this->mergeMusicians();

    TIMED_BLOCK("processAudioAndMidiSignals")
    /*======================================================================================*/
    //
    //	Iterate through the events with a timeStamp within this processBlock i.e.
    //	timeStampOfMidiEvent >= playTime && timeStamp < playTime + numSecondsInThisBlock,
    //	then add this/those midievent to the MidiBuffer of the processBlock
    //	 <midiMesasages.addEvent(midiMessage, ((timeStampOfMidiEvent - playTime)/numSecondsInThisBlock)*buffersize);>
    //	You repeat this until you don't have any more events for this processBlock.
    //	Then you wait for the next invocation (where playTime have incremented by the amount below)
    //	and continues the iteration of your Midibuffer or MidimessageSequence from the previous point
    //	playTime += numSecondsInThisBlock;
    //
    /*======================================================================================*/
    //get Buffersize of block from Audiobuffer
    auto numSamples = audioBuffer.samples;
    //set number of second for each block
    double numSecondsInThisBlock = (double)numSamples / this->samplerate;
    /*======================================================================================*/
    auto lol = this->song.getTrackp(0);
#ifdef DEBUG
    ttmm::logfileMusic->write("New Playtime: ", this->playTime);
    /*======================================================================================*/
    ttmm::logfileMusic->write(std::to_string(this->songInfo.musician().Get(0).accuracy()));
    ttmm::logfileMusic->write(std::to_string(this->songInfo.musician().Get(0).tune()));
#endif
    /*======================================================================================*/
    mHandler.processTrackToNewMidiBuffer(lol, midiMessages,
        playTime, numSecondsInThisBlock, numSamples, this->songInfo.musician().Get(0));

    if (midiMessages.getNumEvents() > 0)
    {
        //ttmm::logfileMusic->write("Playtime: ", this->playTime);
        //ttmm::logfileMusic->write("Number of events in Buffer: ", midiMessages.getNumEvents());
        MidiMessage m;
        int sampleposition;
        for (MidiBuffer::Iterator i(midiMessages); i.getNextEvent(m, sampleposition);)
        {
            if (m.isNoteOn())
            {
				((MusicPluginEditor*)this->getActiveEditor())->addContent("Spiele Note auf Channel: " + std::to_string(m.getChannel()) + " mit Wert: " + std::to_string(m.getNoteNumber()));
                //ttmm::logfileMusic->write("Note On: ", m.getChannel(), m.getNoteNumber(), unsigned(m.getVelocity()), m.getTimeStamp(), sampleposition);
            }
            else if (m.isNoteOff())
            {
                //ttmm::logfileMusic->write("Note Off: ", m.getChannel(), m.getNoteNumber(), m.getTimeStamp(), sampleposition);
            }
        }
    }
}

//merge the input from the others to local
void ttmm::DynamicComposition::mergeMusicians()
{
    IPCSongInfo_IPCMusician m;
    auto local = this->localmusician;
    for (auto const& m : musiciansToMerge)
    {
        local->set_accuracy(m.accuracy());
#ifdef DEBUG
        ttmm::logfileMusic->write("tune received: " + std::to_string(m.tune()));
#endif
        if (m.tune() != IPCSongInfo_IPCMusician_Tune::IPCSongInfo_IPCMusician_Tune_NONE)
        {
            local->set_tune(m.tune());
        }
        if (m.volumech1() != -1)
        {
            local->set_volumech1(m.volumech1());
        }
        if (m.volumech2() != -1)
        {
            local->set_volumech2(m.volumech2());
        }
        if (m.volumech3() != -1)
        {
            local->set_volumech3(m.volumech3());
        }
        //todo channel 4
    }
    musiciansToMerge.clear();
}

//this function is called by ipc connection, when a message has arrived
void ttmm::DynamicComposition::receivedIPC(IPCSongInfo object)
{
    ttmm::logfileMusic->write("received message");
    //push the new inputs to the local list of IPCMusician
    for (auto m : object.musician())
    {
        this->musiciansToMerge.push_back(m);
    }
}

//this function creates the plugins GUI
juce::AudioProcessorEditor* ttmm::DynamicComposition::createEditor()
{
	return new MusicPluginEditor(*this);
}

// This macro-call creates the required instantiation method for the plugin
TTMM_REGISTER_PLUGIN(ttmm::DynamicComposition)
