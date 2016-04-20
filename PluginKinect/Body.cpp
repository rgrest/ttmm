/***********************************************************************
* Module:  ttmm::Body.cpp
* Author:  Tine
* Purpose: Implementation of the class Body
***********************************************************************/

#include "Body.h"
#include <algorithm>

// constructor for Body, creates bodyData with empty joint-positions
ttmm::Body::Body()
{

    for (JointType const &t : supportedTypes)
    {
        bodyData[t] = D2D1_POINT_2F();
    }
}

// destructor for Body, cleans up joint map
ttmm::Body::~Body() { bodyData.clear(); }

// find map entry for type, set position to new position
void ttmm::Body::setJoint(float x, float y, JointType type)
{
    D2D1_POINT_2F position = D2D1::Point2F(x, y);
    bodyData.find(type)->second = position;
}

// find map entry for type & return it
D2D1_POINT_2F ttmm::Body::getJointPosition(JointType type)
{
    D2D1_POINT_2F position = bodyData.find(type)->second;
    return position;
}

// checks if a given joint type is supported by KinectMusician
bool ttmm::Body::isSupported(JointType t)
{
    if (std::find(supportedTypes.begin(), supportedTypes.end(), t) !=
        supportedTypes.end())
        return true;
    else
        return false;
}

// writes all joints (type & position) to logfile
void ttmm::Body::printDebugInfo()
{
    std::map<JointType, D2D1_POINT_2F>::iterator p;

    for (p = bodyData.begin(); p != bodyData.end(); ++p)
    {
        JointType type = p->first;
        D2D1_POINT_2F position = p->second;
        bool noType = false;

        switch (type)
        {
        case 7:
            ttmm::logfileKinect->write("JointType: HandLeft");
            break;
        case 11:
            ttmm::logfileKinect->write("JointType: HandRight");
            break;
        case 20:
            ttmm::logfileKinect->write("JointType: SpineShoulder");
            break;
        case 15:
            ttmm::logfileKinect->write("JointType: FootLeft");
            break;
        case 19:
            ttmm::logfileKinect->write("JointType: FootRight");
            break;
        default:
            noType = true;
            break;
        }

        if (!noType)
        {
            ttmm::logfileKinect->write("JointPosition: " + std::to_string(position.x) + "," + std::to_string(position.y));
        }
    }
}