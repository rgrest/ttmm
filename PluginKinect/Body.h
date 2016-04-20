/**
* @file Body.h
* @author FH-Minden Musikinformatik 2015, group Kinect (Tine, Isabell)
* @brief Defines Body-Data for KinectMusician. Contains map of JointTypes with
*positions
*
*/

#pragma once

#include <map>
#include <vector>

#include "FileWriter.h"
#include "d2d1.h"
#include "Kinect.h"

/**
* Static vector with supported JointTypes for creating KinectMusician bodies.
*/
static const std::vector<JointType> supportedTypes{
    JointType_HandLeft, JointType_HandRight, JointType_SpineShoulder, JointType_SpineBase, 
	JointType_KneeLeft, JointType_KneeRight, JointType_FootLeft, JointType_FootRight, JointType_Head, 
	JointType_ElbowLeft, JointType_ElbowRight };

namespace ttmm
{

/**
* @class Body
* @brief Defines Body-Data for KinectMusician.
*
* @see KinectMusician
*/
class Body
{
  public:
    /**
  * Constructor: Create a Body
  */
    Body();
    /**
  * Destructor
  */
    ~Body();
    /**
  * Find map entry for type, set position to new position
  *
  * @param x X-Position of joint, a float value
  * @param y Y-Position of joint, a float value
  * @param type the JointType
  */
    void setJoint(float x, float y, JointType type);
    /**
  * find map entry for type & return it
  *
  * @param type the JointType
  * @return the position of the joint
  */
    D2D1_POINT_2F getJointPosition(JointType type);
    /**
  * Writes all joints of Body (JointType & position) to logfile
  */
    void printDebugInfo();
    /**
  * checks if JointType is supported
  *
  * @param type the JointType
  * @return a boolean value (true, if type is supported)
  */
    static bool isSupported(JointType type);

  private:
    std::map<JointType, D2D1_POINT_2F>
        bodyData; ///<the map for joint types and positions
};
}