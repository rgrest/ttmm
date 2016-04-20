/***********************************************************************
* Module:  ttmm::KinectBuffer.cpp
* Author:  Tine, Isabell, Philipp
* Purpose: Implementation of the class KinectBuffer
***********************************************************************/

#include "KinectBuffer.h"
#include "FileWriter.h"

// iterates backwards through buffer and searches last event with BodyPart = Arms
ttmm::PoseType ttmm::KinectBuffer::getLatestHandDirection() const
{
    if (!isEmpty())
    {
        std::vector<PoseEvent> data;
        getReverseCopyOfAll(data);

        // iterate backwards through pose history
        for (size_t i = 0; i < data.size(); ++i)
        {
            // only look at arm poses
            if (data.at(i).bodyPart == BodyPart::ARMS)
            {
                return data.at(i).type;
            }
        }
    }
    return (PoseType::BOTTOM_LEFT | PoseType::BOTTOM_RIGHT);
}

// iterates backwards through buffer and searches last event, where the musician
// stomped his feet
ttmm::PoseEvent const *ttmm::KinectBuffer::getLatestEvent() const
{
    if (!isEmpty())
    {
        getReverseCopyOfAll(data);

        // iterate backwards through pose history
        for (size_t i = 0; i < data.size(); ++i)
        {
            // only look at foot poses
            if (data[i].bodyPart == BodyPart::FOOTS)
            {
                if (bothFeetDown(data[i].type))
                {
                    // and in some pose before at least one foot was not on the floor
                    for (int j = i + 1; j < data.size(); ++j)
                    {
                        if (bothFeetUp(data[j].type) || rightFootUp(data[j].type) || leftFootUp(data[j].type))
                        {
                            return &data[i];
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}