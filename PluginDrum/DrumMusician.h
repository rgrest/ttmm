/**
* @file DrumMusician.h
* @author FH-Minden Musikinformatik 2015, Drumgroup (Ricco Schoenebeck, Holger Heeren)
* @brief Holds the data of a Drum-Device
*
*/

#ifndef DRUM_MUSICIAN_H
#define DRUM_MUSICIAN_H

#include <windows.h>

#include "Musician.h"
#include "DrumMidiEvent.h"
#include "FileWriter.h"

namespace ttmm
{
namespace
{
    /**
	* Buffer/Buffer-Size for incoming MIDI-Events
	*
	*/
    const static size_t EventBufferSize{ 10 };
    using Buffer = RingBuffer<DrumMidiEvent, EventBufferSize>;
}
/**
* @class DrumMusician
* @brief Holds the data of a Drum-Device
*
* @see DrumDevice, DrumMidiEvent, DrumInputPluginProcessor
*/
class DrumMusician : public Musician<DWORD, Buffer>
{

public:
    /**
	* Default Constructor
	*
	*/
    DrumMusician() = default;

    /**
	* Constructor
	*
	* @param timeTolerance - Tolerance-Value for Rythm-Match Tolerance
	* @param noteFlag - the Flag/Note-Value of the played Instrument
	*/
    DrumMusician(float timeTolerance, int noteFlag, int veloLevel);

    /**
	* Destructor
	*/
    ~DrumMusician();

    /**
	* Checks if the Drummer is in Tolerance-Range
	*
	* @return bool - "true" if the Drummer is in the Tolerance Range and "false" if not
	*/
    bool checkRythmMatchTolerance();

    /**
	* Function Description:
	* Returns the Accurcy of the volume of this DrumMusician over the time
	*
	* @return volumeAccuracy - The Accuracy of the DrumMusician in reference of volume
	*/
    int ttmm::DrumMusician::getVolumeAccuracy();

    /**
	* Function Description:
	* Returns the volumeAverage of this DrumMusician
	*
	* @return volumeAverage - The Average of the volume, i.e. to set the actual Volume of the played instrument
	*/
    int ttmm::DrumMusician::getVolumeAverage();

    /**
	* Returns the Note-Value-Flag, for example to identifiy wich DrumInstrument is this
	* Every Drum-Instrument have its own Note-Value, so The DrumMusician is flagged by its own Note-Value
	* @return falg - note-Flag
	*/
    int getFlag() const;

    /**
	* Sets the Tolerance-Value between Metronome and Drummer
	*
	* @param timeTolerance - the Tolerance value in wich the Drummer can be play, for example, In the GUI you can enter a value between 0 - 100
	*/
    void setRythmMatchTolerance(int const timeTolerance);

    /**
	* Returns the latest DrumEvent from buffer
	*
	* @return latestEvent - the latest DrumEvent in buffer
	*/
    DrumMidiEvent const* getEvent();

    /**
	* Returns the actual Buffer-Size
	*
	* @return bufferSize - The actual Buffer-Size
	*/
    int getBufferSize();

    /**
	* Pushes the Midi-Data in the DrumMusicians DrumBuffer
	* Pushes only Midi-Events when they are DeBounced
	* When the Midi-Event is valid (DeBounced) then it will pushed and afterwards the Volume & VolumeAccuracy will be checked
	* @param preparedData - The midi Event to push to the DrumMusicians Drum-Event Buffer
	*/
    BufferData* pushData(BufferData const& preparedData) override final
    {
        BufferData* returnValue;
        if (getBufferSize() > 0)
        {
            auto diff = (preparedData.timestamp - getEvent()->timestamp).count();
            long long test = 1000000; // 1ms
            //ttmm::logfileDrum->write("DrumMusician:	   diff: " + std::to_string(diff));
            if (diff > test)
            {
                //ttmm::logger.write("DrumDevice:  Push Data to Musician: " + std::to_string(preparedData.note));

                returnValue = Musician::pushData(preparedData);
            }
            else
            {
                //ttmm::logger.write("DrumMusician:				GEPRELLT!");
                return nullptr;
            }
        }
        else
        {
            //ttmm::logger.write("DrumDevice:  Push Data to Musician: " + toString(preparedData));
            returnValue = Musician::pushData(preparedData);
        }
        //ttmm::logger.write("DrumDevice:  Velocity: " + std::to_string(preparedData.velocity));
        checkVolumeAccuracy();
        calcVolume();
        return returnValue;
    }

private:
    /**
	* This Functions calculates the colume changes over the last (maximal 10) Drum-hits
	*
	*/
    void ttmm::DrumMusician::calcVolume();

    /**
	* FUnction Description:
	* Background of this function is to find out weather the child is hitting the pad
	* near the given level (VolumeLevel). Each time it hits to strong the value of
	* volumeAccuracy is incremented. In cases of to weak punshes it is decremented.
	* The value should be about 0 (maybe we should use a tolerance for volume (VOlUME-IS-HIT: 5%))
	* 5% of 127 is 0.635 --> VOLUME-IS-HIT is 1
	* i.e. i child should hit with volume 80. Our System reads 79 --> All Ok
	*										  Our System reads 81 --> All ok
	*                                         Our System reads 82 --> To hard --> volumeAccuracy++
	*                                         Our System reads 78 --> To Low --> volumeAccuracy--
	* The defines: VOLUMEACCURACYEDGELOWER & VOLUMEACCURACYEDGEUPPER sets the precision of the evaluation!
	*/
    void ttmm::DrumMusician::checkVolumeAccuracy();

    /**
	*flag is defined by the note of the drum pads
	*/
    int flag = 0;

    /**
	* Tolerance Value for the rythm-Match Algorithmus
	*/
    float rythmMatchTolerance = 0.0;

    /**
	* Volume Average
	*/
    int volumeAverage = 0;

    /**
	* Volume Accuracy for evulation the drummers Volume over the time
	*/
    int volumeAccuracy;

    /**
	* Area for Volume Level -> can be Specified by user (i.e.GUI; default: 50)
	* i.e. volumeLevel given by User is 80. the child should now reach this level 
	* as close as possible.
	*/
    int volumeLevel;

/**
 Volume Accuracy evaluation area, Upper Bound/Edge
*/
#define VOLUMEACCURACYEDGEUPPER 10

/**
 Volume Accuracy evaluation area, Lower Bound/Edge
*/
#define VOLUMEACCURACYEDGELOWER -10

/**
 Volume-Accuracy Tolerance-Value
*/
#define VOLUMETOLERANCE 5
};
}

#endif
