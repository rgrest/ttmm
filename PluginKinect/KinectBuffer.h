/**
* @file KinectBuffer.h
* @author FH-Minden Musikinformatik 2015, group Kinect (Tine, Isabell)
* @brief Defines class KinectBuffer which extends super-class Buffer
*
*/

#pragma once

#include "Buffer.h"
#include "PoseEvent.h"

namespace ttmm
{
namespace
{
const size_t BufferSize = 200;
using Buffer = RingBuffer<PoseEvent, BufferSize>;
}
/**
* @class KinectBuffer
* @brief Extends super-class Buffer, overrides function getLatestEvent() and
* adds defines special kinect function getLatestHandDirection()
*
* @see Buffer
*/
class KinectBuffer : public Buffer
{
    mutable std::vector<PoseEvent> data;

  public:
    /**
  * iterates backwards through buffer and searches last event, where the
  *musician stomped his feet
  *
  * @return the last PoseEvent (a stomp, both feet are on the ground after at
  *least one foot was lifted up)
  */
    PoseEvent const*getLatestEvent() const override final;
    /**
  * iterates backwards through buffer and searches last event with BodyPart = Arms
  *
  * @return direction of both arms (e.g. upper left, upper right,...)
  */
    PoseType getLatestHandDirection() const;
};
}