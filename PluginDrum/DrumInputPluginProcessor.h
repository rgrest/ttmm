/**
* @file DrumInputPluginProecessor.h
* @author FH-Minden Musikinformatik 2015, Drumgroup (Ricco Schoenebeck, Holger Heeren, Philipp Kroos)
* @brief 
*
*/

#ifndef DRUM_INPUT_PLUGIN_PROCESSOR
#define DRUM_INPUT_PLUGIN_PROCESSOR

#include <string>

#include "DeviceInputPluginProcessor.h"
#include "DrumDevice.h"
#include "DrumMusician.h"
#include "FileWriter.h"
#include "TimeTools.h"

namespace ttmm
{

namespace
{
    using DataType = DrumDevice::DataType;
    auto const BufferSize = size_t(200);
    auto const inputDeviceName = std::wstring(L"TriggerIO");
}

class DrumInputPluginProcessor
    : public DeviceInputPluginProcessor<DrumDevice, DrumMusician, BufferSize>
{
public:
    using DataType = DrumDevice::DataType;

    DrumInputPluginProcessor()
        : DeviceInputPluginProcessor<DrumDevice, DrumMusician, BufferSize>("Trommel", DrumDevice::drumDevice(), 0)
    {

        inputDevice.addMusicians(&musicians);
        // reading the time-zero timepoint from the database
        timeNow = std::chrono::system_clock::now();
    }
    ~DrumInputPluginProcessor();

protected:
    int getAudioMainChannel() const override
    {
        return 4;
    }
    int getAudioSideChannel() const override
    {
        return 5;
    }
    int getDefaultToleranceNoteValue() const override
    {
        return 32;
    }

private:
    void calculateCustomValues(DrumMusician& m, IPCSongInfo::IPCMusician* musi) override final;
    // Implementing DeviceInputPlugin
    bool init(DrumDevice& device, ttmm::Samplerate sampleRate) final override;
    void close(DrumDevice& device) final override;
    DrumMusician m;

    TIMER_REGISTER("TimerDrum")
};
}
#endif
