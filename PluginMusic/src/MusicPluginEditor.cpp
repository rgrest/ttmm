#include "MusicPluginEditor.h"

namespace ttmm {
	MusicPluginEditor::MusicPluginEditor(DynamicComposition &p)
		: AudioProcessorEditor(&p), processor(p)
	{
		logger.write("Plugin Window opened");
		setSize(WIN_WIDTH, WIN_HEIGHT);

		txtSong = new TextEditor("txtSong", 0);
		txtTempo = new TextEditor("txtTempo", 0);
		lblSong = new Label("lblSong", "Song Name:");
		lblTempo = new Label("lblTempo", "Song Tempo:");
		btnSong = new TextButton("Ändern..", "Klicken um neues MIDI File auszuwählen");
		btnPlus = new TextButton("+", "Klicken um Tempo zu erhöhen");
		btnMinus = new TextButton("-", "Klicken um Tempo zu verringern");
		boxNotes = new ListBox();

		lblSong->setSize(100, 20);
		lblSong->setTopLeftPosition(20, 20);
		txtSong->setSize(200, 20);
		txtSong->setTopLeftPosition(150, 20);
		txtSong->setText(processor.getSongName());
		txtSong->setReadOnly(true);
		btnSong->setSize(70, 20);
		btnSong->setTopLeftPosition(360, 20);
		
		lblTempo->setSize(100, 20);
		lblTempo->setTopLeftPosition(20, 50);
		txtTempo->setSize(200, 20);
		txtTempo->setTopLeftPosition(150, 50);
		txtTempo->setText(processor.getSongTempo());
		txtTempo->setReadOnly(true);
		btnMinus->setSize(30, 20);
		btnMinus->setTopLeftPosition(360, 50);
		btnPlus->setSize(30, 20);
		btnPlus->setTopLeftPosition(400, 50);

		boxNotes->setSize(WIN_WIDTH - 40, WIN_HEIGHT - 100);
		boxNotes->setTopLeftPosition(20, 80);
		boxNotes->setModel(new ListDisplay());

		addAndMakeVisible(txtSong);
		addAndMakeVisible(lblSong);
		addAndMakeVisible(btnSong);
		addAndMakeVisible(txtTempo);
		addAndMakeVisible(lblTempo);
		addAndMakeVisible(btnPlus);
		addAndMakeVisible(btnMinus);
		addAndMakeVisible(boxNotes);
	}

	void MusicPluginEditor::paint(Graphics& g)
	{
		//// fill the whole window white
		g.fillAll(Colours::white);
	}

	void MusicPluginEditor::addContent(String content)
	{
		if (this == nullptr)
		{
			//ttmm::logfileMusic->write("editor fenster existiert nicht");
			return;
		}
		else
		{
			//ttmm::logfileMusic->write("editor fenster ist offen, schreibe eintrag in liste");
			int nr = ((ListDisplay*)boxNotes->getModel())->addRow(content);
			boxNotes->scrollToEnsureRowIsOnscreen(nr);
			boxNotes->repaint();
		}
	}
}
