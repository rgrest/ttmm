#include <windows.h>
#include <string>

#include "DrumDevice.h"
#include "DrumMidiEvent.h"
#include "TimeTools.h"

ttmm::DrumDevice ttmm::DrumDevice::device;

ttmm::DrumDevice::DrumDevice()
{
    ttmm::logfileDrum->write("DrumDevice:  Object init");
}

ttmm::DrumDevice::~DrumDevice()
{
    ttmm::logger.write("DrumDevice:  Object destroyed");
    closeDevice();
}

void ttmm::DrumDevice::addMusicians(std::vector<DrumMusician>* musicians)
{
    drumMusicians = musicians;
}

bool ttmm::DrumDevice::openDevice(std::wstring deviceName)
{
    if (isOpen())
    {
        closeDevice();
    }

    auto inputCapabilities = MIDIINCAPS{};

    for (size_t i = 0; i < midiInGetNumDevs(); ++i)
    {
        auto const deviceNumber = i;

        if (!midiInGetDevCaps(deviceNumber, &inputCapabilities,
                sizeof(MIDIINCAPS)))
        {
            auto const nameOfCurrentDevice = std::wstring(inputCapabilities.szPname);
			std::string s(nameOfCurrentDevice.begin(), nameOfCurrentDevice.end());
			ttmm::logfileDrum->write("Device gefunden, Name: " + s);
			if (nameOfCurrentDevice == deviceName)
            {

                auto const callbackFunction = DWORD_PTR(ttmm::DrumDevice::midiDataCallback);
                auto const unusedParameter = DWORD_PTR(0);
                lastOpenResult = midiInOpen(&midiDevice, deviceNumber, callbackFunction,
                    unusedParameter, CALLBACK_FUNCTION);
				ttmm::logfileDrum->write("verbinde mit " + s);
                break; // \todo Second device with same name -- possible?
            }
        }
    }

    if (isOpen())
    {
        ttmm::logger.write("DrumDevice:  MidiPort Opened");
        midiInStart(midiDevice); // Starts the Midi Input
        ttmm::logger.write("DrumDevice:  MidiIn Started");
        return true;
    }
    ttmm::logger.write("DrumDevice:  MidiPort failed!");
    return false;
}

void ttmm::DrumDevice::closeDevice()
{
    if (isOpen())
    {
        midiInStop(midiDevice);
        midiInClose(midiDevice);
        lastOpenResult = DrumDevice::DEVICE_CLOSED;
        ttmm::logger.write("DrumDevice:  MidiPort Closed");
    }
}

void CALLBACK
ttmm::DrumDevice::midiDataCallback(DeviceHandle midiHandle, UINT midiMessage,
    DWORD unusedParameter, DWORD messageData,
    DWORD timeSinceMidiStartCall)
{
    if (drumDevice().drumMusicians == nullptr)
    {
        return;
    }
	TIMED_BLOCK("midiDataCallback")

	using namespace std::chrono;
	system_clock::time_point now = system_clock::now();
	time_t tt;
	tt = system_clock::to_time_t(now);
	char buff[20];
	auto const& ti = TimeInfo::timeInfo();
    
    auto const midiNoteOnStatus = uint8_t(0x99);
    auto const midiNoteOffStatus = uint8_t(0x00);
    auto const status = ttmm::MidiStatus(LOBYTE(LOWORD(messageData)));
    auto const note = ttmm::MidiNote(HIBYTE(LOWORD(messageData)));
    auto const velocity = ttmm::MidiVelocity(LOBYTE(HIWORD(messageData)));


    // Check Is the Midimessage correct (3Byte NoteOne Message)?
    if ((midiMessage == MIM_DATA) && (status == midiNoteOnStatus) && (velocity != midiNoteOffStatus))
    {
        auto& Musicians = *(drumDevice().drumMusicians);

        // Check if DrumMusician/NoteFlag exist, When Note-specified DrumMusician dont exist, then Create the DrumMusician and Push The Midi-Data to this DrumMusician
        auto m = std::find_if(Musicians.begin(), Musicians.end(), [&note](DrumMusician const& musician)
            {
                return musician.getFlag() == note;
            });
        if (!(m != Musicians.end()))
        {
            // Musician Create when it doesnt exists
            //ttmm::logger->write("DrumDevice:  Create Musician: " + std::to_string(note));
            drumDevice().drumMusicians->push_back(DrumMusician(1.0, note, 126));
        }

        // Iterate thru the DrumMusicians, when Note-specified DrumMusician is found Then Push Data to this DrumMusician
        for (auto& i : Musicians)
        {
            // use note as the flag!
            if (i.getFlag() == note)
            {
                i.pushData({ status, note, velocity });
				ttmm::logger.write("MIDI Event arrived at " + strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&tt)));
                break;
            }
        }
    }
}