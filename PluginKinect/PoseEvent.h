/**
* @file PoseEvent.h
* @author FH-Minden Musikinformatik 2015, KinectPlugin (Tine, Philipp, Isabell)
* @brief Saves the specific events for Kinect musicians.
*/

#if !defined(__DancePlugin_Pose_h)
#define __DancePlugin_Pose_h

#include "PoseType.h"
#include "Buffer.h"
#include "BodyPart.h"

namespace ttmm
{
/**
	* @class PoseEvent
	* @brief Save the specific events for Kinect musicians only.
	* @see BufferData
	*/
struct PoseEvent : public BufferData
{
    /**
		* Defaultconstructor: Create a PoseEvent
		*/
    PoseEvent() = default;
    /**
		*
		* @param t Specifies the type of pose
		* @param p Specifies if the position t is an event of the hands or feet
		* @param v Bodypart "feet": how far away they are from the ground. <br />
		Bodypart "hands": how far away there are from the spine shoulder.
		*/
    PoseEvent(PoseType t, BodyPart p, int v) : type(t), bodyPart(p), value(v) {}
    PoseType type;     //<Specifies the type of pose
    BodyPart bodyPart; //<Specifies if the PoseType type is an event of the hands or feet
    int value;         //<Bodypart "feet": how far away they are from the ground. Bodypart "hands": how far away there are from the spine shoulder.
};
}
#endif
