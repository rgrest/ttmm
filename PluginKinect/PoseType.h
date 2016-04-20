/**
* @file PoseType.h
* @author FH-Minden Musikinformatik 2015, KinectPlugin (Tine, Philipp, Isabell)
* @brief PoseType determines the positon of hands and feet.
*/

#if !defined(__DancePlugin_PoseType_h)
#define __DancePlugin_PoseType_h
#include <iostream>

namespace ttmm
{
/**
	* @class PoseType
	* @brief Saves the determinded poses of Kinect musicians.
	Use the bitwise OR operator several types can be set at a time (for example, gives TOP_LEFT and TOP_RIGHT
	is equal to TOP_MIDDLE) and evaluated using the bitwise AND operator.
	*/
enum class PoseType
{
    TOP_LEFT = 1,
    TOP_RIGHT = 2,
    MIDDLE_LEFT = 4,
    MIDDLE_RIGHT = 8,
    BOTTOM_LEFT = 16,
    BOTTOM_RIGHT = 32
};

/**
	* override the &-operator
	*
	* @param os a reference to ostram
	* @param pt a reference to PoseType
	*/
std::ostream &operator<<(std::ostream &os, PoseType const &pt);

/**
	* override the bitweise or-operator
	*
	* @param a First parameter is set
	* @param b Second parameter is set
	*/
inline PoseType operator|(PoseType a, PoseType b)
{
    return static_cast<PoseType>(static_cast<int>(a) | static_cast<int>(b));
}

/**
	* override the bitwise and-operator
	*
	* @param a First parameter is set
	* @param b Second paramter is set
	*/
inline PoseType operator&(const PoseType a, const PoseType b)
{
    return PoseType(((int)a) & ((int)b));
}

// Utility functions

bool leftFootUp(PoseType const &pose);

bool rightFootUp(PoseType const &pose);

bool leftFootDown(PoseType const &pose);

bool rightFootDown(PoseType const &pose);

bool bothFeetDown(PoseType const &pose);

bool bothFeetUp(PoseType const &pose);
}
#endif