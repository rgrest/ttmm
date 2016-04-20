#include "DrumInputPluginProcessor.h"

bool ttmm::DrumInputPluginProcessor::init(ttmm::DrumDevice& device,
    ttmm::Samplerate samplerate)
{
    //drumSound.setSamplerate(samplerate);
    return device.openDevice(inputDeviceName);
}

void ttmm::DrumInputPluginProcessor::close(ttmm::DrumDevice& device)
{
    ttmm::logger.write("DrumInputPluginProcessor:  closeDevice");
    device.closeDevice();
}

ttmm::DrumInputPluginProcessor::~DrumInputPluginProcessor()
{
    ttmm::logger.write("DrumInputPluginProcessor:  Destroyed");
}

void ttmm::DrumInputPluginProcessor::calculateCustomValues(DrumMusician& m, IPCSongInfo::IPCMusician* musi)
{
    musi->set_accuracy(m.getAccuracy());
    //Channel 3 are for Drums?
    musi->set_volumech3(m.getVolumeAverage());
}


// This macro-call creates the required instantiation method for the plugin
TTMM_REGISTER_PLUGIN(ttmm::DrumInputPluginProcessor)
