/**
 * @file GeneralPluginProcessor.h
 * @author philipp.kroos@fh-bielefeld.de
 * @brief Declaration of the class @code GeneralPluginProcessor
 */

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "Types.h"
#include "TimeTools.h"
#include "DataExchange.pb.h"
#include "GuiParameter.h"

#define TTMM_REGISTER_PLUGIN(qualified_classname)            \
    juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() \
    {                                                        \
        return new qualified_classname();                    \
    }

#define UNUSED(var) (void) var

namespace ttmm
{

//==============================================================================
/**
*/
class GeneralPluginProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    explicit GeneralPluginProcessor(const std::string name, size_t guiParameters);
    virtual ~GeneralPluginProcessor()
    {
        // Google is not cleaning their mess up. See
        // https://developers.google.com/protocol-buffers/docs/cpptutorial
        // search for memory leak.
        google::protobuf::ShutdownProtobufLibrary();
    }

    //
    // Following: the interface to be implemented by concrete plugins
    //
    virtual void processAudioAndMidiSignals(AudioBuffer& audioBuffer,
        MidiBuffer& midiBuffer) = 0;

    void initialize(Samplerate samplerate) {
        initializePlugin(samplerate);
        for (size_t i = 0; i < guiParameters.size(); ++i) {
            makeGuiParameter(i, guiParameters[i]);
        }
    }
    virtual void initializePlugin(Samplerate samplerate) = 0;

    virtual void receivedIPC(IPCSongInfo object) {}
    virtual void shutdown() = 0;

    // Optionally these can be overwritten
    virtual void reinitialize()
    {
        shutdown();
        initialize(samplerate);
    }
    virtual const juce::String
    getInputChannelName(int channelIndex) const override
    {
        return juce::String(channelIndex + 1);
    }
    virtual const juce::String
    getOutputChannelName(int channelIndex) const override
    {
        return juce::String(channelIndex + 1);
    }

    /**
   * Create the default editor which just prints the plugins name.
   */
    virtual juce::AudioProcessorEditor* createEditor() override;

    //
    // Following: the implementation of juce::AudioProcessor
    //

    /**
   * Prepare the ressources this plugin needs to produce a continuous audio
   *stream.
   * From the documentation of juce::AudioProcessor:
   * "Called before playback starts, to let the filter prepare itself [..]"
   * \todo Verify that prepareToPlay is called only once as indicated by the
   *class AudioProcessor.
   * From the documentation of juce::AudioProcessor:
   * "The prepareToPlay() method is guaranteed to be called at least once [..]
   *  Note that this method could be called more than once in succession without
   *a matching call to releaseResources() [..]"
   *
   * \param[in] sampleRate The sample rate to use when producing audio
   * \param[in] samplesPerBlock The expected number of samples to be produced
   */
    void prepareToPlay(double samplerate, int samplesPerBlock) override;
    void releaseResources() override;

    /**
   * Produce the next block of audio samples.
   * The size of sampleBuffer might be more or less as set by prepareToPlay.
   * The number of channels contained may be greater than the maximum of the
   *plugins
   * input/output-channel configuration. Implementation MUST write as much
   *channels as
   * they defined for their output. If no audio will be produced for some
   * channels, they have to be cleared. But, implementations MUST NOT
   * write to a channel greater or equal than it's output-channel
   * configuration.
   * \note This is executed on the audio-thread, so be fast and do not interact
   * with other threads especially not the GUI-thread.
   *
   * \param[in,out] sampleBuffer The rw-buffer containing the audio samples.
   * \param[in,out] midiBuffer The rw-buffer containing the midi data. The
   * buffer will contain the midi messages related to the sampleBuffer-block,
   * with a timestamp corresponding to an offset from the beginning of the
   * block. Any content of the buffer after this message is considered
   * the plugins output.
   */
    void processBlock(juce::AudioSampleBuffer& sampleBuffer,
        juce::MidiBuffer& midiBuffer) override final;

    //==============================================================================

    bool hasEditor() const override { return false; }

    //==============================================================================
    const juce::String getName() const override final { return name; }

    bool isInputChannelStereoPair(int index) const override final { return true; }
    bool isOutputChannelStereoPair(int index) const override final
    {
        return true;
    }

    bool acceptsMidi() const override final { return true; }
    bool producesMidi() const override final { return true; }
    bool silenceInProducesSilenceOut() const override final { return false; }
    double getTailLengthSeconds() const override final { return 0.0; }

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    

    // -----------------------------------------------------------------
    // Interface for the gui. I.e the following functions are called from the GUI-Thread.
    //

    int getNumParameters() override { return guiParameters.size(); }

    /**
     * Retrieve the current value of GUI-Parameter @code index.
     * @param[in] index index of the parameter requested.
     * @return [0, 1]
     */
    float getParameter(int index) override
    {
        if ((index > 0) && (size_t(index) < guiParameters.size()))
        {
            return guiParameters[index].getValue();
        }
        return 0;
    }

    /**
     * Either set the parameter if it was defined by this abstract class,
     * or pass the value to the subclass.
     * @see setCustomParameter
     * @param[in] idx index of the parameter to set
     * @param[in] newValue Value of parameter idx
     */
    void setParameter(int idx, float newValue) override
    {
        if ((idx >= 0) && (size_t(idx) < guiParameters.size()))
        {
            auto index = size_t(idx);
            auto value = guiParameters[index].setValue(newValue); // Translate [0, 1] to internal range

            // Propagate the changed value to the actual parameter
            setGuiParameter(index, value);
        }
    }

    /**
     * Retrieve the name printed on the VST-Gui for parameter @code index
     * @param[in] index index of the parameter requested.
     * @return The name of parameter @code index
     */
    const juce::String getParameterName(int index) override
    {
        return (index >= 0) && (size_t(index) < guiParameters.size()) ? juce::String(guiParameters[index].name) : "";
    }

    /**
     * Retrieve the description printed on the VST-Gui for
     * parameter @code index
     * @param[in] index index of the parameter requested.
     * @return The description of parameter @code index
     */
    const juce::String getParameterText(int index) override
    {
        return (index >= 0) && (size_t(index) < guiParameters.size()) ? juce::String(guiParameters[index].description) : "";
    }

    // end of GUI-methods. See below for the internal interface used.
    // -----------------------------------------------------------------

protected:
    //@todo Use the units specified in TimeTools for sampleDuration
    std::chrono::duration<long double> sampleDuration; ///< Duration of one sample in seconds
    TimeInfo::Timestamp timeNow = TimeInfo::timeInfo().now(); ///< The current time in internal format


    /**
     * Optional internal interface to be overwritten if the subclass
     * defines custom parameters that should be settable from the
     * GUI. Only integer values are supported.  These methods are used by
     * the public methods called from the gui thread, see above.  It is
     * implemented this way to - be able to provide some default
     * parameters used by all deriving plugins - Provide an automatic
     * conversion from the [0,1]-range based values received from the GUI
     *
     * @note These methods extend the default VST-interface. That means
     * that the default GUI-element will be used to set all parameters
     * currrently, which is a slider ranging from 0 to 1.  Custom
     * JUCE-elements are *not* used, so this could be ported directly to
     * 'raw' VST.
     * @param[in] index Index of the parameter to be set
     * @param[in] value New value for parameter index, scaled to its own range.
     */
    virtual void setGuiParameter(size_t index, int value) {}

    /**
     * Interface to create a parameter.  Whether this method will
     * be called for a subclass currently depends on the according value
     * passed to the constructor of @code GeneralPluginProcessor.
     * If it is greater than 0, this method will be called for each index
     * from 0 to the value specified.  The subclass is responsible for the
     * correct mapping of the parameter to the index later on.
     * @see setGuiParameter, where the index has to be used correctly.
     * @param[in] index Index of the parameter to be set
     * @param[inout] parameter The parameter to create.
     */
    virtual void makeGuiParameter(size_t index, GUIParameter& parameter) {}

private:
    Samplerate samplerate = 0; ///< samplerate set during initialization
    const juce::String name; ///< name of this plugin.

    std::vector<GUIParameter> guiParameters; ///< available GUI-Parameters

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneralPluginProcessor)
};
}
#endif // PLUGINPROCESSOR_H_INCLUDED
