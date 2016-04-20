/**
 * @file DeviceInputPluginProcessor.h
 * @author some@fh-bielefeld.de
 * @brief Declaration of @code MidiNoteMessage and @code DeviceInputPluginProcessor
 */

#ifndef DEVICE_INPUT_PLUGIN_PROCESSOR_H
#define DEVICE_INPUT_PLUGIN_PROCESSOR_H

#include <thread>
#include <mutex>
#include <Windows.h>
#include "DataExchange.pb.h"
#include "IPCConnection.h"
#include "Device.h"
#include "Buffer.h"
#include "TimeTools.h"
#include "FileWriter.h"

namespace ttmm
{

/**
 @class MidiNoteMessage
 @brief Structure used to store Metronome Midinotes.
 */
struct MidiNoteMessage : public BufferData
{
    juce::MidiMessage msg;
    MidiNoteMessage() = default;
    MidiNoteMessage(Timestamp t, juce::MidiMessage const& msg)
        : msg(msg)
    {
        timestamp = t;
    }
};

/**
 * @class DeviceInputPluginProcessor
 * @brief Abstract the commonalities of ttmm-InputPlugins.
 *
 * This is a template which is to be instantiated with a realization
 * of the abstract @code Device, the abstract @code Musician and a size
 * of the buffer, setting the possible amount of Musicians useable.
 */
template <typename Device, typename Musician, size_t musiciansBufferSize>
class DeviceInputPluginProcessor : public GeneralPluginProcessor
{

public:
    /**
     * c'tor.
     * @param[in] pluginName Name of the plugin to be created.
     * @param[inout] inputDevice a ref to the device managed.
     * @param[additionalParameters] Number of GUI-parameters to be created for this Plug.
     */
    DeviceInputPluginProcessor(std::string pluginName, Device& inputDevice, size_t additionalParameters)
        : GeneralPluginProcessor(pluginName, guiParametersSize() + additionalParameters)
        , inputDevice(inputDevice)
        , connection{ this }
    {
        bool openedOk = connection.connectToPipe(PIPE_NAME, 1000);
        if (openedOk)
        {
            ttmm::logfileGeneral->write("Established IPC!");
        }
        else
        {
            ttmm::logfileGeneral->write("## Error: Establishing IPC failed!");
        }
    }

    /**
     * d'tor. No dynamic vars, nothing to do.
     */
    ~DeviceInputPluginProcessor()
    {
        // Disconnecting would be obvious and shouldn't do harm - but this causes a memory leak.
        // The connection is closed anyway in the destructor of IPCConnection.
        // connection.disconnect();
    }

    /**
     * @brief Check whether the managed Musicians played correct.
     * This uses the internal @code noteHistory (actually, only the last element
     * with the current implementation) to check the latest events generated
     * by the @code musicians.
     * @note side-effects: 
     *   - This sets @code Musician::playedCorrect
     *   - If there was a considerable change in at least one Musician,
     *     the change will be sent via IPC.
     */
    void compareEventWithMusic();

    /**
     * Add a Musician to the managed list @code musicians.
     */
    void addMusician() { musicians.push_back(); }

    /**
     * @see GeneralPluginProcessor.
     */
    void initializePlugin(ttmm::Samplerate sampleRate) override final
    {
        if (!(init(inputDevice, sampleRate) && inputDevice.isOpen()))
        {
            // Log an error, notify the user, something.
            ttmm::logfileGeneral->write(
                "## Error: Initializing PluginProcessor failed!");
            shutdown();
        }
        else
        {
            ttmm::logfileGeneral->write("Initialized PluginProcessor ");
        }

        uhrenvergleich();
    }

    void shutdown() override final
    {
        close(inputDevice);
        if (inputDevice.isOpen())
        {
            // Log an error, notify the user, something.
            ttmm::logfileGeneral->write(
                "## Error: Shutting down PluginProcessor failed!");
        }
    }

    void processAudioAndMidiSignals(AudioBuffer& audioBuffer,
        juce::MidiBuffer& midiBuffer) override final;

	std::vector<Musician> getMusicians() {
		return musicians;
	}

protected:
    RingBuffer<MidiNoteMessage, 30> noteHistory; ///< deprecated, only the last element is ever used.
    std::vector<Musician> musicians;
    Device& inputDevice;
    virtual void calculateCustomValues(Musician& m,
        IPCSongInfo::IPCMusician* musi) {}

    /**
      Optionally concrete Plugins can define additional parameters.
      @see makeGuiParameter and setGuiParameter.
    */
    virtual void makeCustomGuiParameter(size_t index, GUIParameter& parameter) {}
    virtual void setCustomGuiParameter(size_t index, int newValue) {}

    /**
      Defines the actual main channel used by the Plugin.
      As for the midi messages received, the channel convention is:
      - Channel 1: Metronome
      - Channel 2: Main channel
      - Channel 3: Side channel
      If a message for e.g. Channel 2 (Main) is received, it will be re-routed
      to the main channel as defined by the concrete Plugin.  This way, different plugins can
      use the same instrument but different channels, i.e. sounds.
      @note The GUI has sliders for the Musicians channels. These change the convention for
            channels 2 and 3 as described above.
      @return The channel this Plugin uses as main channel.
    */
    virtual int getAudioMainChannel() const = 0;
    virtual int getAudioSideChannel() const = 0;
    virtual int getDefaultToleranceNoteValue() const = 0;
    virtual bool canSend() { return false; }

private:
    // @todo: read config file for metronom
    int metronomChannel = 1; ///< The fixed channel where the Metronome is received.
    int audioMainChannel = 2; ///< The fixed main channel for received messages.
    int audioSideChannel = 3; ///< The fixed side channel for received messages.
    int toleranceNoteValue = 16; ///< Tolerance value used during comparison of musicians events

    /**
     Number of parameters defined in this abstract Plug.
     @note This is a method rather than a variable because it's needed to be passed to the superclass
     and a static var *might* not be initialized at that point.
     */
    static const size_t guiParametersSize() {
        return 3;
    }

    IPCConnection connection;
    const std::string PIPE_NAME = "18cmPENIS";
    virtual bool init(Device& device, Samplerate sampleRate) = 0;
    virtual void close(Device& device) = 0;

    // @todo: Retrieve bpm from config file
    float bpm = 120.0;

    /**
     * Utility method to evaluate precision of a musician.
     * Compare the timestamp of a musicians last event with
     * the timestamp of the last note.
     * @param m The musician to check.
     * @param timeLastNote Timestamp of the relevant note.
     * @return true if the Musician played correctly.
     */
    bool compareTimestamps(Musician& m, Timestamp timeLastNote) const;

    /**
     * Utility method used in processAudioAndMidi.
     * Filter the midiBuffer received from the Host:
     *  - If Metronome data, store the note in @code noteHistory
     *  - If instrument channel, check whether the corresponding
     *    Musician played correct. If, let the note through. Else, block it.
     *  - Else: let through.
     * @param midiBuffer The current next-to-play midi-buffer.
     */
    void filterMidiBuffer(juce::MidiBuffer& midiBuffer);

    /**
     * @brief Initialize the internal clock of this plugin.
     * Read the file 'bigbang' created by the PluginMusic, which only
     * contains a timestamp in the format of @code TimeInfo.
     * The retrieved timestamp is used as a reference point from hereon.
     * @see processAudioAndMidiSignals
     */
    void uhrenvergleich();

    /**
     * Either create a parameter if it was defined by this abstract class,
     * or pass the value to the subclass.
     *    @see GeneralPluginProcessor
     */
    void makeGuiParameter(size_t index, GUIParameter& parameter) override;

    /**
     * Either set the parameter if it was defined by this abstract class,
     * or pass the value to the subclass.
     *    @see GeneralPluginProcessor
     */
    void setGuiParameter(size_t index, int value) override;

};
}

// wird zyklisch aufgerufen
template <typename Device, typename Musician, size_t bufferSize>
void ttmm::DeviceInputPluginProcessor<Device, Musician, bufferSize>::
    processAudioAndMidiSignals(AudioBuffer& audioBuffer,
        juce::MidiBuffer& midiBuffer)
{
    TIMED_BLOCK("processAudioAndMidiSignals")
    std::this_thread::sleep_for(std::chrono::nanoseconds(500));
    std::vector<typename Device::DataType> bufferedData;
    filterMidiBuffer(midiBuffer);
    compareEventWithMusic();
}

// Abstrakte Template Methode, die von zyklisch aufgerufener Methode processAudioAndMidiSignals 
// verwendet wird um Midibuffer anzupassen. Hierbei werden Noten auf Ausgabekanäle 
// der entsprechenden Plugins umgeroutet, wenn playedCorrect Wert eines Musikanten true ist,
// bzw. entfernt, wenn dieser Wert false ist.
template <typename Device, typename Musician, size_t bufferSize>
void ttmm::DeviceInputPluginProcessor<Device, Musician,
    bufferSize>::filterMidiBuffer(juce::MidiBuffer& midiBuffer)
{
	timeNow = TimeInfo::timeInfo().now(); // Converting the note-on events to our internal note-buffer, changing the relative timestamp to a real timestamp.
	juce::MidiMessage msg;
	int samplePosition;
	juce::MidiBuffer::Iterator it(midiBuffer);
	juce::MidiBuffer newBuffer;
	while (it.getNextEvent(msg, samplePosition))
	{

	    // Write all midimessages that do NOT contain NoteOn to instrument
	    if (!msg.isNoteOn())
	    {
			newBuffer.addEvent(msg, samplePosition);
			continue;
	    }

	    auto channel = msg.getChannel();
	    // All messages with Metronom Channel should be stored in internal buffer
	    // and will be compared with movement
	    if (channel == metronomChannel)
	    {
			ttmm::logger.write("metronomChannel arrives: " + std::to_string(channel) + " == " + std::to_string(metronomChannel));
			newBuffer.addEvent(msg, samplePosition);
			noteHistory.push(MidiNoteMessage(timeAfter(timeNow, samplePosition * sampleDuration), msg));
	    }

	    // @todo Let the actual plugins deicide what to do with audioMain/audioSideChannel
	    // Each Musician can play notes from one channel, if he moves correctly
	    // Therefore all notes from Musician channel will be written to instrument
	    // channel, if "gates are opened"
		
	    else if (channel == audioMainChannel)
	    {
			ttmm::logger.write("audioMainChannel arrives: " + std::to_string(channel) + " == " + std::to_string(audioMainChannel));
			if (musicians.size() > 0)
			{
				if (musicians[0].playedCorrect) // check first musician, if he is correct, notes from his channel can be passed to instrument
				{ 
					ttmm::logger.write("musician 1 played correct, open the gates of sound.."); 
					msg.setChannel(getAudioMainChannel()); //<reroute to the plug's internal main channel
					//msg.setVelocity(musicians[0].getVolumeFactor());
					newBuffer.addEvent(msg, samplePosition);
					continue;
				}
				else
				{
					ttmm::logger.write("musician 1 played false, close the gates of sound..");
				}
			}
	    }
	    else if (channel == audioSideChannel)
	    {
			ttmm::logger.write("audioSideChannel arrives: " + std::to_string(channel));
			if (musicians.size() > 1)
			{
				if (musicians[1].playedCorrect) // check second musician, if he is correct, notes from his channel can be passed to instrument
				{ 
					ttmm::logger.write("musician 2 played correct, open the gates of sound.."); 
					msg.setChannel(getAudioSideChannel()); //<reroute to the plug's internal side channel
					//msg.setVelocity(musicians[1].getVolumeFactor());
					newBuffer.addEvent(msg, samplePosition);
					continue;
				}
				else
				{
					ttmm::logger.write("musician 2 played false, close the gates of sound..");
				}
			}
			else
			{
				ttmm::logger.write("there is only one musician");
			}
	    }
	}

	// execute our changes
	midiBuffer.swapWith(newBuffer);
}


template <typename Device, typename Musician, size_t bufferSize>
void ttmm::DeviceInputPluginProcessor<Device, Musician,
    bufferSize>::compareEventWithMusic()
{
    TIMED_BLOCK("compareEventWithMusic");

    if (musicians.size() <= 0)
    {
        return;
    }

    // create serialisation object
    IPCSongInfo si;

    for (size_t i = 0; i < musicians.size(); ++i)
    {
        auto& m = musicians[i];

        // nothing to do if no event from Musician or no notes yet
        if (m.getHistory().isEmpty() || (noteHistory.isEmpty()))
        {
            continue;
        }

        auto musicianPlayedCorrect = false;
        auto lastNote = const_cast<MidiNoteMessage*>(noteHistory.getLatestEvent());
        auto timeLastNote = lastNote->timestamp;

        // If the musician created an event
        if (m.getHistory().hasChanged() && (timeLastNote != m.getLastMatchedNoteTimestamp()))
        {
            m.getHistory().setHasChanged(false);
            //  If the event matches with the last note and the last note is unmatched by earlier events
            //    - Mark the note as 'matched'
            //    - Mark musician as 'correct'
            musicianPlayedCorrect = compareTimestamps(m, timeLastNote);
        }
        // Else if new note received: (no new event but maybe last event matches now)
        else if (noteHistory.hasChanged())
        {
            //  If last event matches this note:
            //    - Mark the note as 'matched'
            //    - Mark musician as 'correct'
            musicianPlayedCorrect = compareTimestamps(m, timeLastNote);
        }
        else
        {
            continue;
        }

        // increase / decrease musicians accuracy and open /close gates of sound
        if (musicianPlayedCorrect)
        {
            m.setLastMatchedNoteTimestamp(lastNote->timestamp);
            m.increaseAccuracy();
            m.playedCorrect = true;
        }
        else
        {
            m.decreaseAccuracy();
            m.playedCorrect = false;
        }

        // store musician to SongInfo object
        IPCSongInfo::IPCMusician* musi;
        if (canSend())
        {
            musi = si.add_musician();
            musi->set_tune(
                IPCSongInfo::IPCMusician::Tune::IPCSongInfo_IPCMusician_Tune_NONE);
            calculateCustomValues(m, musi);
            musi->set_accuracy(m.getAccuracy());
        }
    }

    if (noteHistory.hasChanged())
    {
        noteHistory.setHasChanged(false);
    }

    // send SongInfo object
    if (canSend())
    {
        connection.send(si);
    }
}

// Abstrakte Template Methode, die von sowohl Kinect- als auch Drum-Plugin 
// genutzt werden kann um letzten Aktionszeitpunkt eines Musikanten 
// mit Zeitpunkt der Midi-Note zu vergleichen
template <typename Device, typename Musician, size_t bufferSize>
bool ttmm::DeviceInputPluginProcessor<Device, Musician,
    bufferSize>::compareTimestamps(Musician& m, Timestamp timeLastNote) const
{
    auto latestEvent = m.getHistory().getLatestEvent();
    if (latestEvent == nullptr)
    {
        return false;
    }
    auto timeLatestEvent = latestEvent->timestamp;

    //auto maxTolerance = noteLengthAtBPMInSeconds(bpm, getToleranceNoteValue());
    auto maxTolerance = noteLengthAtBPMInSeconds(bpm, toleranceNoteValue);
    auto upperBound = timeAfter(timeLastNote, maxTolerance);
    auto lowerBound = timeBefore(timeLastNote, maxTolerance);

    // if musician has play correct, return true
    return (timeLatestEvent > lowerBound) && (timeLatestEvent < upperBound);
}

// Helfer-Methoder für Synchronisation der Plugins auf gemeinsame Startzeit
template <typename Device, typename Musician, size_t bufferSize>
void ttmm::DeviceInputPluginProcessor<Device, Musician, bufferSize>::
uhrenvergleich()
{
    auto realtime = TimeInfo::timeInfo().realtimeInNanoseconds();

    std::ifstream clockfile;
    clockfile.open("bigbang.txt");
    long long offsetOfSong, startOfMusicPlugin;
    if (clockfile.is_open())
    {

	logger.write("Realtime now: " + std::to_string(realtime));

	clockfile >> startOfMusicPlugin;
	clockfile >> offsetOfSong;

	auto passed = realtime - startOfMusicPlugin;

	///@todo Using a non-portable number here; might fail on some systems.
	TimeInfo::timeInfo().setTimeZeroFromSeconds(
	    startOfMusicPlugin / 1000000000.0 + std::abs(offsetOfSong));

	logger.write("NS after big bang: " + std::to_string(passed));

	auto zero = TimeInfo::timeInfo().ns(TimeInfo::timeInfo().getTimeZero());
	auto now = TimeInfo::timeInfo().ns(TimeInfo::timeInfo().now());
	logger.write("Set timeZero to " + std::to_string(zero));
	logger.write("Now it is " + std::to_string(now));

	clockfile.close();
    }
    else
    {
	logger.write("Could not read the clockfile");
    }
}

// Erstellt simple Schieber-GUI um Parameter der Plugins einzustellen,
// wird nur benutzt, wenn kein Editor angelegt und aktiviert wurde
template <typename Device, typename Musician, size_t bufferSize>
void ttmm::DeviceInputPluginProcessor<Device, Musician, bufferSize>::
makeGuiParameter(size_t index, GUIParameter& parameter) {

    switch (index) {
    case 0:
	parameter = { 2, 3, 2, "Main Channel", "Channel for Musician 1" };
	break;
    case 1:
	parameter = { 2, 3, 3, "Side Channel", "Channel for Musician 2" };
	break;
    case 2:
	// slider: left is 64th so lowest tolerance
	parameter = { 64, 16, getDefaultToleranceNoteValue(), "Tolerance", "Accepted inaccuracy" };
	break;
    default:
	// This is still an abstract Plugin, so delegate to implementations.
	index = index - guiParametersSize();
	makeCustomGuiParameter(index, parameter);
	break;
    }
}

template <typename Device, typename Musician, size_t bufferSize>
void ttmm::DeviceInputPluginProcessor<Device, Musician, bufferSize>::
setGuiParameter(size_t index, int value) {
    switch (index) {
    case 0:
	audioMainChannel = value;
	break;
    case 1:
	audioSideChannel = value;
	break;
    case 2:
	toleranceNoteValue = value;
    default:
	index = index - guiParametersSize();
	setCustomGuiParameter(index, value);
	break;
    }
}

#endif
