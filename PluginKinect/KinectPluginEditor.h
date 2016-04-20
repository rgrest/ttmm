#pragma once

#include "DanceDisplay.h"
#include <thread>

namespace ttmm 
{
	class KinectPluginEditor : public juce::AudioProcessorEditor , private juce::TextEditor::Listener
	{
		public:
			KinectPluginEditor(KinectInputPluginProcessor &);
			~KinectPluginEditor();

			void paint(Graphics&) override;
			void textEditorTextChanged(juce::TextEditor&) override;

		private:
			const int WIN_WIDTH = 1000;
			const int WIN_HEIGHT = 500;
			const int PADDING = 10;

			KinectInputPluginProcessor& processor;

			juce::TextEditor* txtFootTol;
			juce::TextEditor* txtHandTol;
			juce::TextEditor* txtMatchTol;
			std::thread repaintThread;

			Label* lblFootTol;
			Label* lblHandTol;
			Label* lblMatchTol;

			DanceDisplay* display;
			bool opened = false;

			void repaint() {
				while (opened)
				{
					if (display != nullptr)
					{
						display->repaint();
					}
				}
			}

			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KinectPluginEditor)
	};
}
