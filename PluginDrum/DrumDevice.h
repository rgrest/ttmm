/**
* @file DrumDevice.h
* @author FH-Minden Musikinformatik 2015, Drumgroup (Ricco Schoenebeck, Holger Heeren, Philipp Kroos)
* @brief Handles the MIDI-Input-Device
*
*/
#ifndef DRUMDEVICE_H
#define DRUMDEVICE_H

#include <string>
#include <windows.h>

#include "Device.h"
#include "DrumMidiEvent.h"
#include "Buffer.h"
#include "Types.h"
#include "DrumMusician.h"

namespace ttmm
{
/**
 * @class DrumDevice
 * @brief Handles the MIDI-Input-Device
 *
 * @see DrumMusician, DrumMidiEvent
 */
class DrumDevice : public Device<DrumMidiEvent>
{

public:
    using DataType = DrumMidiEvent;

    /**
     * This Method checks if a Midi-In-Device is opened and then Close this if opened
     */
    void closeDevice();

    /**
     * Opens a Midi-Device with the specified Device-Name
     * The Midi-Device must be started and plugged in before Juce Starts!
     *
     * @param deviceName - Name of the Device to be opened, this must be exact the same Name of the Device-Midi-Driver
     * @return bool - true if the Device could successfull opened, false if the Device could not be openend (wrong Name, wrong Plug-/Start-Timing)
     */
    bool openDevice(std::wstring deviceName);

    /**
     * This Method checks if a Midi-In-Device is opened
     */
    bool isOpen() const override final 
	{ 
		
		if (lastOpenResult == MMSYSERR_NOERROR) { ttmm::logfileDrum->write("Gerät meldet MMSYSERR_NOERROR"); }
		if (lastOpenResult == MMSYSERR_ALLOCATED) { ttmm::logfileDrum->write("Gerät meldet MMSYSERR_ALLOCATED"); }
		if (lastOpenResult == MMSYSERR_BADDEVICEID) { ttmm::logfileDrum->write("Gerät meldet MMSYSERR_BADDEVICEID"); }
		if (lastOpenResult == MMSYSERR_INVALFLAG) { ttmm::logfileDrum->write("Gerät meldet MMSYSERR_INVALFLAG"); }
		if (lastOpenResult == MMSYSERR_INVALPARAM) { ttmm::logfileDrum->write("Gerät meldet MMSYSERR_INVALPARAM"); }
		if (lastOpenResult == MMSYSERR_NOMEM) { ttmm::logfileDrum->write("Gerät meldet MMSYSERR_NOMEM"); }
		return lastOpenResult == MMSYSERR_NOERROR; 
	}

    /**
     * Construction logic
     */
    static DrumDevice& drumDevice() { return DrumDevice::device; }
    DrumDevice(DrumDevice const&) = delete;
    ~DrumDevice();

    /**
     * Adds the DrumMusician vector to this Module
     * This is Neccessery to use the Musician-Vector in the static CallBack-Method
     *
     * @param musicians - Vectorliste vom Typ ttmm::DrumMusician
     */
    void addMusicians(std::vector<DrumMusician>* musicians);

private:
    /**
     * Musicians List, will be initialized in the construcor calling
     */
    std::vector<DrumMusician>* drumMusicians = nullptr;

    /**
     * Constructor
     */
    DrumDevice();

    using DeviceHandle = HMIDIIN;
    /**
     * This DeviceHandle will be filled with a handle identifying the opened MIDI input device
     */
    DeviceHandle midiDevice;

    using DeviceStatus = int16_t;
    static const auto DEVICE_CLOSED = DeviceStatus(MMSYSERR_BASE) - 1;
    DeviceStatus lastOpenResult = DEVICE_CLOSED;

    static DrumDevice device; ///< Singleton instance

    /**
     * Implements the callback for incoming data (of a before opened Midi-Device) in the Windows Midi-API.
     * It only accepts "Midi-Channel-Messages" (MIM_DATA).
     * The Midi-Channel-Messages must contain Status= NoteOne, Midi-Channel = 10 and the Velocity must be greater than 0.
     * This Method Add/Create new DrumMusician if the DrumDevice wich is played is not regristered in the Musicians-List and pushes the Midi-Message to his Buffer
     * If the DrumMusician is already regristered, it pushes directly the Midi-Message to his Buffer
     * Every DrumMusician is one Instrument. Every Instrument have its own Note-Value, so The DrumMusician is flagged by its own constant Note-Value
     */
    static void CALLBACK midiDataCallback(DeviceHandle midiHandle,
        UINT midiMessage, DWORD unusedParameter,
        DWORD messageData,
        DWORD timeSinceMidiStartCall);
};
}

#endif