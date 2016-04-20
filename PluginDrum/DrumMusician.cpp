#include <windows.h>

#include "DrumMusician.h"
#include "Musician.h"
#include "DrumMidiEvent.h"

ttmm::DrumMusician::DrumMusician(float const timeTolerance = 1.0, int const noteFlag = 0, int veloLevel = 126)
{
    volumeLevel = veloLevel;
    rythmMatchTolerance = timeTolerance;
    flag = noteFlag;
    volumeAccuracy = 0;
    ttmm::logfileDrum->write("DrumMusician:    Musician Created!");
}

ttmm::DrumMusician::~DrumMusician()
{
    ttmm::logger.write("DrumMusician:  Object destroyed");
}

bool ttmm::DrumMusician::checkRythmMatchTolerance()
{
    ttmm::logger.write("DrumMusician:    Tolerance Check");
    return 0;
}

void ttmm::DrumMusician::setRythmMatchTolerance(int const rythmTimeTolerance)
{
    rythmMatchTolerance = (float)(rythmTimeTolerance / 100);
    ttmm::logger.write("DrumMusician:    Set Tolerance to " + std::to_string(rythmMatchTolerance));
}

int ttmm::DrumMusician::getFlag() const
{
    return flag;
}

ttmm::DrumMidiEvent const* ttmm::DrumMusician::getEvent()
{
    return this->getHistory().getLatestEvent();
}

int ttmm::DrumMusician::getBufferSize()
{
    return this->getHistory().currentSize();
}

void ttmm::DrumMusician::calcVolume()
{

    int drumHits = 10, i = 0, divisor = 0, size = getBufferSize();

    if (size > 0)
    {
        volumeAverage = 0;
        std::vector<DrumMidiEvent> testVector;

        // get the actual regristered Drum hits
        this->getHistory().getCopyOfAll(testVector);

        // Are there minimum 10 Drum hits avialable?
        if (size < drumHits)
        {
            // If not, than take the actual numbers of events
            divisor = size;
        }
        else
        {
            // If 10 or more are available, then take the last 10 DrumHits to calculate the volume
            divisor = drumHits;
            i = size - drumHits;
        }

        // get the velocitys
        for (int index = i; index < size; index++)
        {
            volumeAverage += testVector.at(index).velocity;
        }

        // calculate the average of Velocitys to determine the volume
        volumeAverage = (int)volumeAverage / divisor;
        ttmm::logger.write("DrumMusician:	   VolumeAverage: " + std::to_string(volumeAverage));
    }
}

void ttmm::DrumMusician::checkVolumeAccuracy()
{
    auto latestEvent = getEvent();
    if (latestEvent != nullptr)
    {
        int velo = latestEvent->velocity;
        //ttmm::logger->write("DrumMusician:    Velocity " + std::to_string(velo));
        // Takes the actual Event and checks how the velocity matches the specified Volume-Level/Volume-Target
        if (velo < (volumeLevel - VOLUMETOLERANCE))
        {
            // If the Drum-Hit velocity is lower than the specified Volume-Level + tolerance --> volumeAccuracy--_
            if (volumeAccuracy > VOLUMEACCURACYEDGELOWER)
            {
                volumeAccuracy--;
            }
        }
        else if (velo > (volumeLevel + VOLUMETOLERANCE))
        {
            // If the Drum-Hit velocity is lower than the specified Volume-Level + tolerance --> volumeAccuracy--_
            if (volumeAccuracy < VOLUMEACCURACYEDGEUPPER)
            {
                volumeAccuracy++;
            }
        }
        ttmm::logger.write("DrumMusician:	   VolumeAccuracy: " + std::to_string(volumeAccuracy));
    }
}

int ttmm::DrumMusician::getVolumeAccuracy()
{
    return volumeAccuracy;
}

int ttmm::DrumMusician::getVolumeAverage()
{
    return volumeAverage;
}
