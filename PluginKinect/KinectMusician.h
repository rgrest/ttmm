/**
* @file KinectMusican.h
* @author FH-Minden Musikinformatik 2015, KinectPlugin (Tine, Philipp, Isabell)
* @brief Declaration of KinectMusician (child)
*/
#pragma once

#include "FileWriter.h"
#include "Kinect.h"
#include "Body.h"
#include "PoseEvent.h"
#include "Musician.h"
#include "KinectBuffer.h"

namespace ttmm
{

/**
	* @class KinectMusician
	* @brief Holds the information of a single musician.
	* @see Musician
	*/
class KinectMusician : public Musician<D2D1_POINT_2F, KinectBuffer>
{

  public:
    /**
	* Defaultconstuctor: Create a KinectMusician
	*/
    KinectMusician() = default;
    /*
  * Constructor: Create a KinectMusician
  * @param b Set the body information of the child
  */
    KinectMusician(Body *b);

    /*
   * Destructor
   */
    ~KinectMusician() = default;

    /*
  * Set the current body of the child
  * @param b The body data of the child
  */
    void setBody(Body *b);

    /**
  * Function queries whether already been assigned to the bodies of musician.
  */
    bool hasBody() { return bodyData != nullptr; };

    /**
     Set the sensitivity of feet detection.
    */
    void setFootTolerance(int tolerance) {
        footTolerance = tolerance;
    }

	/**
	Get the current sensitivity of feet detection.
	*/
	int getFootTolerance()
	{
		return footTolerance;
	}

	/**
	Get the current sensitivity of hand detection.
	*/
	int getHandTolerance()
	{
		return handTolerance;
	}

	/**
	Set the sensitivity of hand detection.
	*/
	void setHandTolerance(int tolerance) {
		handTolerance = tolerance;
	}
	Body* getBody() {
		return bodyData;
	}

  private:
    Body *bodyData = nullptr; //<Saves the body information of the child
    enum Direction
    {
        LEFT,
        RIGHT,
        MIDDLE,
        NOTHING
    }; //<auxiliary enum to check the hand position

    /**
  * Calculates the current foot and arm position of the child and stores this in events.
  * @param ground The ground on which the child stays
  * @param events The reference of the Ringbuffer to stores the new positions
  */
    void createNewEventsToAdd(D2D1_POINT_2F const &ground, std::vector<PoseEvent> &events) override final;

    int footTolerance = 30; //<Indicates the tolerance with which a foot is still on the ground.
	int handTolerance = 10; //<Indicates the tolerance with which a hand is still in middle position.

    /**
  * Calculated from the current body data the positions of the hands.
  * @param events Specifies the ring buffer in which the poses are stored
  */
    void checkHands(std::vector<ttmm::PoseEvent> &events);
    /**
  * Calculated from the current body data the positions of the food.
  * @param events Specifies the ring buffer in which the poses are stored
  * @param ground Specified the ground on which the child standing
  */
    void checkFoots(D2D1_POINT_2F const &ground, std::vector<ttmm::PoseEvent> &events);
    /**
  * Calculates whether the hands are left, center or right from the midsection.
  * @param left 2D point left hand
  * @param right 2D point right Hand
  * @param spineShoulder 2D point of the spine shoulder
  */
    Direction getHandDirection(D2D1_POINT_2F const &left, D2D1_POINT_2F const &right, D2D1_POINT_2F const &spineShoulder);

    /**
   * Return true, when the type is not the last lastArmPose or lastFeetPose (depended on part)
     Return false otherwise
   * @param type The type with the previous position be to compared
   * @param part Indicates from which part of the body the previous pose be compared
   */
    bool poseChanged(ttmm::PoseType type, ttmm::BodyPart part);

	void calculateVolumeFactor();

    PoseType lastArmsPose;	//<Saves the last arm pose to quickly access it
    PoseType lastFeetPose;	//<Saves the last feet pose to quickly access it
	int lastFootHeight;		//<Saves the last foot height to quickly access it
	int maxFootHeight = 0;	//<Saves the highest foot height the musician has reached during runtime
};
}
