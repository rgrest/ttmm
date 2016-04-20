#pragma once
#include "KinectInputPluginProcessor.h"

namespace ttmm
{
	class DanceDisplay : public juce::Component
	{
	public:
		DanceDisplay(KinectInputPluginProcessor &);
		void paint(Graphics&) override;
		
	private:
		KinectInputPluginProcessor& proc;
		void drawFloor(Graphics& g);
		bool drawMusicians(Graphics& g);
		D2D1_POINT_2F transformToCanvasSpace(D2D1_POINT_2F pointCamSpace, juce::Rectangle<int> canvas);

		const float JOINT_SIZE = 10.0;
		const float HEAD_SIZE = 20.0;
	};
}