#include "DanceDisplay.h"

namespace ttmm 
{
	/**
	* Constructor for DanceDisplay
	*/
	DanceDisplay::DanceDisplay(KinectInputPluginProcessor& p) : Component(), proc(p)
	{
	}

	void DanceDisplay::paint(Graphics& g)
	{
		if (proc.isConnected())
		{
			// set the font size and draw text to the screen
			g.setColour(Colours::black);
			g.fillRect(g.getClipBounds());

			//  draw musicians
			bool thereIsSomeone = drawMusicians(g);

			// draw floor (only if there are musicians)
			if (thereIsSomeone)
			{
				drawFloor(g);
			}
		}
		else
		{
			// set the font size and draw text to the screen
			g.setColour(Colours::black);
			g.fillRect(g.getClipBounds());
			g.setColour(Colours::white);
			g.setFont(15.0f);
			g.drawText("not connected", g.getClipBounds().getCentreX() - 50, g.getClipBounds().getCentreY() - 15, 100, 30, Justification::centred, true);
		}
	}

	/**
	* Function draws floor line and foot tolerance range to canvas
	*/
	void DanceDisplay::drawFloor(Graphics& g)
	{
		// get position of floor and tolerance in CameraSpace coordinates
		D2D1_POINT_2F floorPosition = proc.getFloorPosition();
		D2D1_POINT_2F floorWithTolerance;
		floorWithTolerance.x = floorPosition.x;
		floorWithTolerance.y = floorPosition.y - proc.getParam(KinectInputPluginProcessor::ParameterType::FOOT_TOL);

		// calculate y position (using canvas coordinates) of floor and tolerance range 
		int yPos = transformToCanvasSpace(floorPosition, g.getClipBounds()).y;
		int yPosTol = transformToCanvasSpace(floorWithTolerance, g.getClipBounds()).y;

		// draw floor line and tolerance range
		g.setColour(Colours::aquamarine);
		g.drawHorizontalLine(yPos, g.getClipBounds().getBottomLeft().getX(), g.getClipBounds().getBottomRight().getX());
		g.setOpacity(0.3);
		g.fillRect(juce::Rectangle<int>(g.getClipBounds().getBottomLeft().getX(), yPosTol,
										g.getClipBounds().getWidth(), g.getClipBounds().getHeight() - yPosTol));
		g.setOpacity(1.0);
	}

	/**
	* Function draws musicians to canvas, body joints will be displayed with circles,
	* bones are displayed as lines
	*/
	bool DanceDisplay::drawMusicians(Graphics& g)
	{
		bool result = false;

		// paint all musicians joints to canvas
		for (KinectMusician m : proc.getMusicians())
		{
			Body* b = m.getBody();
			if (b == nullptr)
			{
				continue;
			}
			else
			{
				result = true;
				g.setColour(Colours::lightgreen);

				// draw bone from head to neck
				D2D1_POINT_2F head = transformToCanvasSpace(b->getJointPosition(JointType::JointType_Head), g.getClipBounds());
				//logger.write("Head Pos = " + std::to_string(head.x) + "/" + std::to_string(head.y));
				D2D1_POINT_2F neck = transformToCanvasSpace(b->getJointPosition(JointType::JointType_SpineShoulder), g.getClipBounds());
				//logger.write("Neck Pos = " + std::to_string(neck.x) + "/" + std::to_string(neck.y));
				g.drawLine(juce::Line<float>(head.x, head.y, neck.x, neck.y));
				// draw bone from neck to hip
				D2D1_POINT_2F hip = transformToCanvasSpace(b->getJointPosition(JointType::JointType_SpineBase), g.getClipBounds());
				g.drawLine(juce::Line<float>(neck.x, neck.y, hip.x, hip.y));
				// draw bones for left leg
				D2D1_POINT_2F lKnee = transformToCanvasSpace(b->getJointPosition(JointType::JointType_KneeLeft), g.getClipBounds());
				D2D1_POINT_2F lFoot = transformToCanvasSpace(b->getJointPosition(JointType::JointType_FootLeft), g.getClipBounds());
				g.drawLine(juce::Line<float>(hip.x, hip.y, lKnee.x, lKnee.y));
				g.drawLine(juce::Line<float>(lKnee.x, lKnee.y, lFoot.x, lFoot.y));
				// draw bones for right leg
				D2D1_POINT_2F rKnee = transformToCanvasSpace(b->getJointPosition(JointType::JointType_KneeRight), g.getClipBounds());
				D2D1_POINT_2F rFoot = transformToCanvasSpace(b->getJointPosition(JointType::JointType_FootRight), g.getClipBounds());
				g.drawLine(juce::Line<float>(hip.x, hip.y, rKnee.x, rKnee.y));
				g.drawLine(juce::Line<float>(rKnee.x, rKnee.y, rFoot.x, rFoot.y));
				// draw bones for left arm
				D2D1_POINT_2F lElbow = transformToCanvasSpace(b->getJointPosition(JointType::JointType_ElbowLeft), g.getClipBounds());
				D2D1_POINT_2F lHand = transformToCanvasSpace(b->getJointPosition(JointType::JointType_HandLeft), g.getClipBounds());
				g.drawLine(juce::Line<float>(neck.x, neck.y, lElbow.x, lElbow.y));
				g.drawLine(juce::Line<float>(lElbow.x, lElbow.y, lHand.x, lHand.y));
				// draw bones for right arm
				D2D1_POINT_2F rElbow = transformToCanvasSpace(b->getJointPosition(JointType::JointType_ElbowRight), g.getClipBounds());
				D2D1_POINT_2F rHand = transformToCanvasSpace(b->getJointPosition(JointType::JointType_HandRight), g.getClipBounds());
				g.drawLine(juce::Line<float>(neck.x, neck.y, rElbow.x, rElbow.y));
				g.drawLine(juce::Line<float>(rElbow.x, rElbow.y, rHand.x, rHand.y));

				// draw head and other joints
				g.fillEllipse(head.x - (HEAD_SIZE / 2), head.y - (HEAD_SIZE / 2), HEAD_SIZE, HEAD_SIZE);		//<head
				g.fillEllipse(neck.x - (JOINT_SIZE / 2), neck.y - (JOINT_SIZE / 2), JOINT_SIZE, JOINT_SIZE);	//<neck
				g.fillEllipse(hip.x - (JOINT_SIZE / 2), hip.y - (JOINT_SIZE / 2), JOINT_SIZE, JOINT_SIZE);		//<hip
				g.fillEllipse(lKnee.x - (JOINT_SIZE / 2), lKnee.y - (JOINT_SIZE / 2), JOINT_SIZE, JOINT_SIZE);	//<left knee
				g.fillEllipse(rKnee.x - (JOINT_SIZE / 2), rKnee.y - (JOINT_SIZE / 2), JOINT_SIZE, JOINT_SIZE);	//<right knee
				g.fillEllipse(lFoot.x - (JOINT_SIZE / 2), lFoot.y - (JOINT_SIZE / 2), JOINT_SIZE, JOINT_SIZE);	//<left foot
				g.fillEllipse(rFoot.x - (JOINT_SIZE / 2), rFoot.y - (JOINT_SIZE / 2), JOINT_SIZE, JOINT_SIZE);	//<right foot
				g.fillEllipse(lHand.x - (JOINT_SIZE / 2), lHand.y - (JOINT_SIZE / 2), JOINT_SIZE, JOINT_SIZE);	//<left hand
				g.fillEllipse(rHand.x - (JOINT_SIZE / 2), rHand.y - (JOINT_SIZE / 2), JOINT_SIZE, JOINT_SIZE);	//<right hand
				g.fillEllipse(lElbow.x - (JOINT_SIZE / 2), lElbow.y - (JOINT_SIZE / 2), JOINT_SIZE, JOINT_SIZE);//<left elbow
				g.fillEllipse(rElbow.x - (JOINT_SIZE / 2), rElbow.y - (JOINT_SIZE / 2), JOINT_SIZE, JOINT_SIZE);//<right elbow
			}
		}
		return result;
	}

	D2D1_POINT_2F DanceDisplay::transformToCanvasSpace(D2D1_POINT_2F pointCamSpace, juce::Rectangle<int> canvas)
	{
		//logger.write("Pos (CamSpace) = " + std::to_string(pointCamSpace.x) + "/" + std::to_string(pointCamSpace.y));
		D2D1_POINT_2F pointCanvSpace;
		float canvSpaceMinX = canvas.getBottomLeft().getX();
		float canvSpaceMinY = canvas.getBottomLeft().getY();
		float canvSpaceMaxX = canvas.getTopRight().getX();
		float canvSpaceMaxY = canvas.getTopRight().getY();
		//logger.write("canvSpaceMinX=" + std::to_string(canvSpaceMinX) + " canvSpaceMaxX=" + std::to_string(canvSpaceMaxX) +
		//	"canvSpaceMinY=" + std::to_string(canvSpaceMinY) + " canvSpaceMaxY=" + std::to_string(canvSpaceMaxY));
		float camSpaceMinX = 0;
		float camSpaceMinY = 689;
		float camSpaceMaxX = 768;
		float camSpaceMaxY = 0;
		float scaleX = (canvSpaceMaxX - canvSpaceMinX) / (camSpaceMaxX - camSpaceMinX);
		float scaleY = (canvSpaceMaxY - canvSpaceMinY) / (camSpaceMaxY - camSpaceMinY);
		//logger.write("scaleX=" + std::to_string(scaleX) + " scaleY=" + std::to_string(scaleY));
		pointCanvSpace.x = canvSpaceMinX + (pointCamSpace.x - camSpaceMinX) * scaleX;
		pointCanvSpace.y = canvSpaceMinY + (pointCamSpace.y - camSpaceMinY) * scaleY;
		return pointCanvSpace;
	}
}