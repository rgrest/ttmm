#pragma once
#include "ListDisplay.h"

namespace ttmm
{
	class MusicPluginEditor : public juce::AudioProcessorEditor
	{
	public:
		MusicPluginEditor(DynamicComposition &);
		~MusicPluginEditor() = default;

		void paint(Graphics&) override;
		void addContent(String content);

	private:
		const int WIN_WIDTH = 500;
		const int WIN_HEIGHT = 500;
		const int PADDING = 10;

		TextEditor* txtSong;
		TextEditor* txtTempo;
		Label* lblSong;
		Label* lblTempo;
		TextButton* btnSong;
		TextButton* btnPlus;
		TextButton* btnMinus;
		ListBox* boxNotes;

		DynamicComposition& processor;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicPluginEditor)
	};
}