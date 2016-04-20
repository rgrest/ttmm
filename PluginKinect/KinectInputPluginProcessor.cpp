#include <algorithm>

#include "KinectInputPluginProcessor.h"
#include "KinectPluginEditor.h"

void ttmm::KinectInputPluginProcessor::calculateCustomValues(KinectMusician &m, IPCSongInfo::IPCMusician *musi)
{
    PoseType t = m.getHistory().getLatestHandDirection();
    switch (t)
    {
    case PoseType::BOTTOM_LEFT:
        musi->set_tune(IPCSongInfo::IPCMusician::Tune::IPCSongInfo_IPCMusician_Tune_LEFT_DOWN);
        break;
    case PoseType::BOTTOM_RIGHT:
        musi->set_tune(IPCSongInfo::IPCMusician::Tune::IPCSongInfo_IPCMusician_Tune_RIGHT_DOWN);
        break;
    case PoseType::TOP_LEFT:
        musi->set_tune(IPCSongInfo::IPCMusician::Tune::IPCSongInfo_IPCMusician_Tune_LEFT_UP);
        break;
    case PoseType::TOP_RIGHT:
        musi->set_tune(IPCSongInfo::IPCMusician::Tune::IPCSongInfo_IPCMusician_Tune_RIGHT_UP);
        break;
    default:
        if (((t & PoseType::TOP_LEFT) == PoseType::TOP_LEFT) && ((t & PoseType::TOP_RIGHT) == PoseType::TOP_RIGHT))
        {
            musi->set_tune(IPCSongInfo::IPCMusician::Tune::IPCSongInfo_IPCMusician_Tune_MIDDLE_UP);
        }
        else if (((t & PoseType::BOTTOM_LEFT) == PoseType::BOTTOM_LEFT) && ((t & PoseType::BOTTOM_RIGHT) == PoseType::BOTTOM_RIGHT))
        {
            musi->set_tune(IPCSongInfo::IPCMusician::Tune::IPCSongInfo_IPCMusician_Tune_MIDDLE_DOWN);
        }
        break;
    }
    ttmm::logger.write("Setting Tune to: " + std::to_string(musi->tune()));
}

juce::AudioProcessorEditor* ttmm::KinectInputPluginProcessor::createEditor()
{
	return new KinectPluginEditor(*this);
}

// This macro-call creates the required instantiation method for the plugin
TTMM_REGISTER_PLUGIN(ttmm::KinectInputPluginProcessor)
