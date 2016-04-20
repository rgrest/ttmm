#include "KinectMusician.h"

ttmm::KinectMusician::KinectMusician(Body *b) : bodyData(b) {}

void ttmm::KinectMusician::createNewEventsToAdd(D2D1_POINT_2F const &ground, std::vector<ttmm::PoseEvent> &events)
{
    if (bodyData)
    {
        checkHands(events);
        checkFoots(ground, events);
    }
}

void ttmm::KinectMusician::setBody(Body *b) { bodyData = b; }

/**
 * Calculate the handpositions.
 * Valid positions are:
 * TOP_LEFT
 * TOP_RIGHT
 * TOP_LEFT | TOP_RIGHT = TOP_MIDDLE
 * MIDDLE_LEFT
 * MIDDLE_RIGHT
 * MIDDLE_LEFT | MIDDLE_RIGHT = MIDDLE_MIDDLE
 * BOTTOM_LEFT
 * BOTTOM_RIGHT
 * BOTTOM_LEFT | BOTTOM_RIGHT = BOTTOM_MIDDLE
 */
void ttmm::KinectMusician::checkHands(std::vector<ttmm::PoseEvent> &events)
{
    D2D1_POINT_2F handLeft = bodyData->getJointPosition(JointType_HandLeft);
    D2D1_POINT_2F handRight = bodyData->getJointPosition(JointType_HandRight);
    D2D1_POINT_2F spineShoulder =
        bodyData->getJointPosition(JointType_SpineShoulder);

    int handsValue = static_cast<int>(std::round((handLeft.y + handRight.y) / 2.0 - spineShoulder.y));

    //For all poses above the shoulder
	if ((handLeft.y < (spineShoulder.y - handTolerance)) && (handRight.y < (spineShoulder.y - handTolerance)))
    {
        Direction d = getHandDirection(handLeft, handRight, spineShoulder);
        switch (d)
        {
        case ttmm::KinectMusician::LEFT:
            if (poseChanged(PoseType::TOP_LEFT, BodyPart::ARMS))
            {
                ttmm::logger.write("createNewEventsToAdd: TOP_LEFT, ARMS, " + std::to_string(handsValue));
                events.emplace_back(PoseType::TOP_LEFT, BodyPart::ARMS, handsValue);
                lastArmsPose = PoseType::TOP_LEFT;
            }
			else 
            {
				//ttmm::logfileKinect->write("Pose did not change: TOP_RIGHT, ARMS, " + std::to_string(handsValue));
				// \todo: change value of pose
            }
            break;
        case ttmm::KinectMusician::RIGHT:
            if (poseChanged(PoseType::TOP_RIGHT, BodyPart::ARMS))
            {
                ttmm::logger.write("createNewEventsToAdd: TOP_RIGHT, ARMS, " + std::to_string(handsValue));
                events.emplace_back(PoseType::TOP_RIGHT, BodyPart::ARMS, handsValue);
                lastArmsPose = PoseType::TOP_RIGHT;
            }
            else
            {
                //ttmm::logfileKinect->write("Pose did not change: TOP_RIGHT, ARMS, " + std::to_string(handsValue));
                // \todo: change value of pose
            }
            break;
        case ttmm::KinectMusician::MIDDLE:
            if (poseChanged(PoseType::TOP_LEFT | PoseType::TOP_RIGHT, BodyPart::ARMS))
            {
                ttmm::logger.write("createNewEventsToAdd: TOP_MIDDLE, ARMS, " + std::to_string(handsValue));
                events.emplace_back(PoseType::TOP_LEFT | PoseType::TOP_RIGHT, BodyPart::ARMS, handsValue);
                lastArmsPose = PoseType::TOP_LEFT | PoseType::TOP_RIGHT;
            }
            else
            {
                //ttmm::logfileKinect->write("Pose did not change: TOP_MIDDLE, ARMS, " + std::to_string(handsValue));
                // \todo: change value of pose
            }
            break;
        }
    }
    //Shoulder height
    else if (((handLeft.y >= (spineShoulder.y - 10)) && (handLeft.y <= (spineShoulder.y + 10))) && ((handRight.y >= (spineShoulder.y - 10)) && (handRight.y <= (spineShoulder.y + 10))))
    {
        Direction d = getHandDirection(handLeft, handRight, spineShoulder);
        switch (d)
        {
        case ttmm::KinectMusician::LEFT:
            if (poseChanged(PoseType::MIDDLE_LEFT, BodyPart::ARMS))
            {
                ttmm::logger.write("createNewEventsToAdd: MIDDLE_LEFT, ARMS, " + std::to_string(handsValue));
                events.emplace_back(PoseType::MIDDLE_LEFT, BodyPart::ARMS, handsValue);
                lastArmsPose = PoseType::MIDDLE_LEFT;
            }
            else
            {
                //ttmm::logfileKinect->write("Pose did not change: MIDDLE_LEFT, ARMS, " + std::to_string(handsValue));
                // \todo: change value of pose
            }
            break;
        case ttmm::KinectMusician::RIGHT:
            if (poseChanged(PoseType::MIDDLE_RIGHT, BodyPart::ARMS))
            {
                ttmm::logger.write("createNewEventsToAdd: MIDDLE_RIGHT, ARMS, " + std::to_string(handsValue));
                events.emplace_back(PoseType::MIDDLE_RIGHT, BodyPart::ARMS, handsValue);
                lastArmsPose = PoseType::MIDDLE_RIGHT;
            }
            else
            {
                // ttmm::logfileKinect->write("Pose did not change: MIDDLE_RIGHT, ARMS, " + std::to_string(handsValue));
                // \todo: change value of pose
            }
            break;
        case ttmm::KinectMusician::MIDDLE:
            if (poseChanged(PoseType::MIDDLE_LEFT | PoseType::MIDDLE_RIGHT, BodyPart::ARMS))
            {
                ttmm::logger.write("createNewEventsToAdd: MIDDLE_MIDDLE, ARMS, " + std::to_string(handsValue));
                events.emplace_back(PoseType::MIDDLE_LEFT | PoseType::MIDDLE_RIGHT, BodyPart::ARMS, handsValue);
                lastArmsPose = PoseType::MIDDLE_LEFT | PoseType::MIDDLE_RIGHT;
            }
            else
            {
                //ttmm::logfileKinect->write("Pose did not change: MIDDLE_MIDDLE, ARMS, " + std::to_string(handsValue));
                // \todo: change value of pose
            }
            break;
        }
    }
    //For all poses below the shoulder
    else if ((handLeft.y > (spineShoulder.y + 10)) && (handRight.y > (spineShoulder.y + 10)))
    {
        Direction d = getHandDirection(handLeft, handRight, spineShoulder);
        switch (d)
        {
        case ttmm::KinectMusician::LEFT:
            if (poseChanged(PoseType::BOTTOM_LEFT, BodyPart::ARMS))
            {
                ttmm::logger.write("createNewEventsToAdd: BOTTOM_LEFT, ARMS, " + std::to_string(handsValue));
                events.emplace_back(PoseType::BOTTOM_LEFT, BodyPart::ARMS, handsValue);
                lastArmsPose = PoseType::BOTTOM_LEFT;
            }
            else
            {
                //ttmm::logfileKinect->write("Pose did not change: BOTTOM_LEFT, ARMS, " + std::to_string(handsValue));
                // \todo: change value of pose
            }
            break;
        case ttmm::KinectMusician::RIGHT:
            if (poseChanged(PoseType::BOTTOM_RIGHT, BodyPart::ARMS))
            {
                ttmm::logger.write("createNewEventsToAdd: BOTTOM_RIGHT, ARMS, " + std::to_string(handsValue));
                events.emplace_back(PoseType::BOTTOM_RIGHT, BodyPart::ARMS, handsValue);
                lastArmsPose = PoseType::BOTTOM_RIGHT;
            }
            else
            {
                //ttmm::logfileKinect->write("Pose did not change: BOTTOM_RIGHT, ARMS, " + std::to_string(handsValue));
                // \todo: change value of pose
            }
            break;
        case ttmm::KinectMusician::MIDDLE:
            if (poseChanged(PoseType::BOTTOM_LEFT | PoseType::BOTTOM_RIGHT, BodyPart::ARMS))
            {
                ttmm::logger.write("createNewEventsToAdd: BOTTOM_MIDDLE, ARMS, " + std::to_string(handsValue));
                events.emplace_back(PoseType::BOTTOM_LEFT | PoseType::BOTTOM_RIGHT, BodyPart::ARMS, handsValue);
                lastArmsPose = PoseType::BOTTOM_LEFT | PoseType::BOTTOM_RIGHT;
            }
            else
            {
                //ttmm::logfileKinect->write("Pose did not change: BOTTOM_MIDDLE, ARMS, " + std::to_string(handsValue));
                // \todo: change value of pose
            }
            break;
        }
    }
}

/**
 * Calculate the foot Position
 * Valid PoseTypes are:
 * BOTTOM_LEFT | BOTTOM_RIGHT = Both feet on the ground
 * BOTTOM_LEFT | TOP_RIGHT = Left foot on the ground, right in the air
 * TOP_LEFT | BOTTOM_RIGHT = Left foot in the air, right one on the ground
 * TOP_LEFT | TOP_RIGHT = Both feet in the air
 */
void ttmm::KinectMusician::checkFoots(D2D1_POINT_2F const &ground, std::vector<ttmm::PoseEvent> &events)
{
    int leftFootValue = static_cast<int>(std::round(bodyData->getJointPosition(JointType_FootLeft).y));
    int rightFootValue = static_cast<int>(std::round(bodyData->getJointPosition(JointType_FootRight).y));
    int differenzInHeight = leftFootValue - rightFootValue;
    //ttmm::logger.write("Checking feet for ", leftFootValue, rightFootValue, ground.y);

    //Both feet on the ground
    if ((leftFootValue >= (ground.y - footTolerance)) &&
        (rightFootValue >= (ground.y - footTolerance)))
    {
        if (poseChanged(PoseType::BOTTOM_LEFT | PoseType::BOTTOM_RIGHT, BodyPart::FOOTS))
        {
            ttmm::logger.write("createNewEventsToAdd: DOWN, FEET, " + std::to_string(differenzInHeight));
            ttmm::logger.write("ground Y = " + std::to_string(ground.y) + " leftFoot Y = " + std::to_string(leftFootValue) + " rightFoot Y = " + std::to_string(rightFootValue));
            events.emplace_back(PoseType::BOTTOM_LEFT | PoseType::BOTTOM_RIGHT, BodyPart::FOOTS, differenzInHeight);
            lastFeetPose = PoseType::BOTTOM_LEFT | PoseType::BOTTOM_RIGHT;
			lastFootHeight = 0;
        }
    }
    //Right foot above ground
    else if ((leftFootValue >= (ground.y - footTolerance)) && (rightFootValue < (ground.y - footTolerance))
		&& ((differenzInHeight > 10) || (differenzInHeight <-10)))
    {
        if (poseChanged(PoseType::BOTTOM_LEFT | PoseType::TOP_RIGHT, BodyPart::FOOTS))
        {
            ttmm::logger.write("createNewEventsToAdd: LEFT_DOWN & RIGHT_TOP, FEET, " + std::to_string(differenzInHeight));
            ttmm::logger.write("ground Y = " + std::to_string(ground.y) + " leftFoot Y = " + std::to_string(leftFootValue) + " rightFoot Y = " + std::to_string(rightFootValue));
            events.emplace_back(PoseType::BOTTOM_LEFT | PoseType::TOP_RIGHT, BodyPart::FOOTS, differenzInHeight);
            lastFeetPose = PoseType::BOTTOM_LEFT | PoseType::TOP_RIGHT;
			lastFootHeight = ground.y - rightFootValue;
			calculateVolumeFactor();
        }
		else if (ground.y - rightFootValue > lastFootHeight)
        {
			lastFootHeight = ground.y - rightFootValue;
			calculateVolumeFactor();
        }
    }
    //Left foot above ground
    else if ((leftFootValue < (ground.y - footTolerance)) && (rightFootValue >= (ground.y - footTolerance))
		&&((differenzInHeight > 10) || (differenzInHeight <- 10)))
    {
        if (poseChanged(PoseType::TOP_LEFT | PoseType::BOTTOM_RIGHT, BodyPart::FOOTS))
        {
            ttmm::logfileKinect->write("createNewEventsToAdd: LEFT_TOP & RIGHT_DOWN, FEET " + std::to_string(differenzInHeight));
            ttmm::logger.write("ground Y = " + std::to_string(ground.y) + " leftFoot Y = " + std::to_string(leftFootValue) + " rightFoot Y = " + std::to_string(rightFootValue));
            events.emplace_back(PoseType::TOP_LEFT | PoseType::BOTTOM_RIGHT, BodyPart::FOOTS, differenzInHeight);
            lastFeetPose = PoseType::TOP_LEFT | PoseType::BOTTOM_RIGHT;
			lastFootHeight = ground.y - leftFootValue;
			calculateVolumeFactor();
        }
		else if (ground.y - leftFootValue > lastFootHeight)
        {
			lastFootHeight = ground.y - leftFootValue;
			calculateVolumeFactor();
        }
    }

    //Jump (both feet above ground)
	else if ((leftFootValue < (ground.y - footTolerance)) &&
		(rightFootValue < (ground.y - footTolerance)) && 
		(differenzInHeight < 30) && (differenzInHeight > -30))
    {
        if (poseChanged(PoseType::TOP_LEFT | PoseType::TOP_RIGHT, BodyPart::FOOTS))
        {
            ttmm::logger.write("createNewEventsToAdd: TOP, FEET, " + std::to_string(differenzInHeight));
            ttmm::logger.write("ground Y = " + std::to_string(ground.y) + " leftFoot Y = " + std::to_string(leftFootValue) + " rightFoot Y = " + std::to_string(rightFootValue));
            events.emplace_back(PoseType::TOP_LEFT | PoseType::TOP_RIGHT, BodyPart::FOOTS, differenzInHeight);
            lastFeetPose = PoseType::TOP_LEFT | PoseType::TOP_RIGHT;
			lastFootHeight = (ground.y - leftFootValue) > (ground.y - rightFootValue) ? ground.y - leftFootValue : ground.y - rightFootValue;
			calculateVolumeFactor();
        }
		else if ((ground.y - leftFootValue > lastFootHeight) || (ground.y - rightFootValue > lastFootHeight))
        {
			lastFootHeight = (ground.y - leftFootValue) > (ground.y - rightFootValue) ? ground.y - leftFootValue : ground.y - rightFootValue;
			calculateVolumeFactor();
        }
    }
}

ttmm::KinectMusician::Direction ttmm::KinectMusician::getHandDirection(D2D1_POINT_2F const &left, D2D1_POINT_2F const &right, D2D1_POINT_2F const &spineShoulder)
{
	if (left.x < (spineShoulder.x - handTolerance) && right.x < (spineShoulder.x - handTolerance))
    {
        return Direction::LEFT;
    }
	else if ((left.x >= (spineShoulder.x - handTolerance) && (left.x <= (spineShoulder.x + handTolerance))) && (right.x >= (spineShoulder.x - handTolerance) && (right.x <= (spineShoulder.x + handTolerance))))
    {
        return Direction::MIDDLE;
    }
	else if ((left.x >(spineShoulder.x + handTolerance)) && (right.x >(spineShoulder.x + handTolerance)))
    {
        return Direction::RIGHT;
    }
    return Direction::NOTHING;
}

bool ttmm::KinectMusician::poseChanged(ttmm::PoseType type, ttmm::BodyPart part)
{
    if (part == BodyPart::ARMS)
    {
        if (lastArmsPose == type)
            return false;
        else
            return true;
    }
    else if (part == BodyPart::FOOTS)
    {
        if (lastFeetPose == type)
            return false;
        else
            return true;
    }
    return true;
}

void ttmm::KinectMusician::calculateVolumeFactor()
{
	if (lastFootHeight > maxFootHeight)
	{
		maxFootHeight = lastFootHeight;
		volumefactor = 1.0;
	} 
	else
	{
		volumefactor = lastFootHeight / maxFootHeight;
	}
}