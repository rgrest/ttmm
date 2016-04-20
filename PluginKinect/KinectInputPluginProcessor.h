/**
* @file KinectInputPluginProcessor.h
* @author FH-Minden Musikinformatik 2015, KinectPlugin (Tine, Philipp, Isabell)
* @brief
*/

#ifndef KINECT_INPUT_PLUGIN_PROCESSOR
#define KINECT_INPUT_PLUGIN_PROCESSOR

#include <vector>
#include <chrono>

#include "DeviceInputPluginProcessor.h"

#include "Musician.h"
#include "KinectDevice.h"

namespace ttmm
{

namespace
{
    const static size_t BUFFER_SIZE = 1000; //<specifies the buffer size
}

/**
	* @class KinectInputPluginProcessor
	* @brief KinectInputPluginProcessor is a derivation of the template DeviceInputPluginProcessor.
	It specifies the template paramters with Kinect specific types.
	* @see DeviceInputPluginProcessor, KinectDevice, KinectMusician
	*/
class KinectInputPluginProcessor
    : public DeviceInputPluginProcessor<KinectDevice, KinectMusician, BUFFER_SIZE>
{
public:
    using DataType = KinectDevice::DataType;

    /**
		* Defaultconstructor: Create a new KinectInputPluginProcessor
		*/
    KinectInputPluginProcessor()
        : DeviceInputPluginProcessor<KinectDevice, KinectMusician, BUFFER_SIZE>("Kinect", device, 1)
        , device(musicians)
    {
    }

	enum class ParameterType
	{
		FOOT_TOL,
		HAND_TOL,
		MATCH_TOL
	};

	void updateParam(ParameterType type, int paramValue)
	{
		switch (type) 
		{
		case (ParameterType::FOOT_TOL):
			for (auto& m : musicians) {
				m.setFootTolerance(paramValue);
			}
			break;
		case (ParameterType::HAND_TOL) :
			for (auto& m : musicians) {
				m.setHandTolerance(paramValue);
			}
			break;
		case (ParameterType::MATCH_TOL) :
			matchTolerance = paramValue;
			break;
		}
	}

	bool isConnected() {
		return device.isOpen();
	}

	D2D1_POINT_2F getFloorPosition() {
		return device.getFloor2D();
	}

	int getParam(ParameterType type)
	{
		int tol = 0;
		switch (type)
		{
		case (ParameterType::FOOT_TOL) :
			for (auto& m : musicians) {
				tol = m.getFootTolerance();
			}
			break;
		case (ParameterType::HAND_TOL) :
			for (auto& m : musicians) {
				tol = m.getHandTolerance();
			}
			break;
		case (ParameterType::MATCH_TOL) :
			return matchTolerance;
			break;
		}
		return tol;
	}

protected:
    int getAudioMainChannel() const override
    {
        return 2;
    }
    int getAudioSideChannel() const override
    {
        return 3;
    }
    int getDefaultToleranceNoteValue() const override {
        return matchTolerance;
    }

    void makeCustomGuiParameter(size_t index, GUIParameter& parameter) override {
        switch (index) {
        case 0: 
            parameter = { 20, 40, 40, "Feet Tolerance", "Stamp sensibility" };
            break;
        }
    }
    void setCustomGuiParameter(size_t index, int value) override {
        switch (index) {
        case 0:
            for (auto& m : musicians) {
                m.setFootTolerance(value);
            }
        default:
            break;
        }
    }
    bool canSend() { return true; }
	
	juce::AudioProcessorEditor* createEditor() override; //<create custom UI for drum plugin
	bool hasEditor() const override { return true; } 

private:
	int matchTolerance = 16;
    KinectDevice device; //<The associated KinectDevice
    Vector4* floor; //<Saves the ground where the musicians stays

    /**
		 * Return true, when the device and the samplerate is initialised.
		 * @param device The KinectDevice to which a connection is to be established
		 * @param sampleRate The sample rate of the tune
		 */
    bool init(KinectDevice& device, Samplerate sampleRate) override final { return true; }

    /**
		 * Close connection to KinectDevice
		 * @param device The KinectDevice to which the connection is to terminated.
		 */
    void close(KinectDevice& device) override final {}

    /**
		 * Determined from the musicians, the current values and stores them in musi.
		 * @param m Specifies the musician, whose information is determined.
		 * @param musi Saves the information of m
		 */
    void calculateCustomValues(KinectMusician& m, IPCSongInfo::IPCMusician* musi) override final;
};
}

#endif