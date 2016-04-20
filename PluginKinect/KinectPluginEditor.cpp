#include "KinectPluginEditor.h"

namespace ttmm {


	/**
	* Constructor for KinectPluginEditor
	*/
	KinectPluginEditor::KinectPluginEditor(KinectInputPluginProcessor& p)
		: AudioProcessorEditor(&p), processor(p)
	{
		logger.write("Plugin Window opened");
		opened = true;
		// set window size
		setSize(WIN_WIDTH, WIN_HEIGHT);

		// set canvas size and position
		int canvasSize = WIN_HEIGHT - (PADDING * 2);
		//canvas.setSize(canvasSize, canvasSize);
		//canvas.setPosition(PADDING, PADDING); 
		display = new DanceDisplay(p);
		display->setSize(canvasSize, canvasSize);
		display->setTopLeftPosition(PADDING, PADDING);

		// create gui objects
		txtFootTol = new TextEditor("txtFootTol",0);
		txtHandTol = new TextEditor("txtHandTol", 0);
		txtMatchTol = new TextEditor("txtMatchTol", 0);
		lblFootTol = new Label("lblFootTol", "Foot Tolerance");
		lblHandTol = new Label("lblHandTol", "Hand Tolerance");
		lblMatchTol = new Label("lblMatchTol", "Matching Tolerance");

		// set gui objects' parameters (size, position, listener,..)
		int width = ((WIN_WIDTH - (canvasSize + (PADDING * 2))) - (PADDING * 2)) / 2;
		int height = 20;
		txtFootTol->setSize(width, height);
		txtHandTol->setSize(width, height);
		txtMatchTol->setSize(width, height);
		lblFootTol->setSize(width, height);
		lblHandTol->setSize(width, height);
		lblMatchTol->setSize(width, height);
		int xposLbl = (WIN_WIDTH - (canvasSize + (PADDING * 2))) + PADDING;
		int xposTxt = xposLbl + width;
		txtFootTol->setTopLeftPosition(xposTxt, PADDING + 20);
		txtHandTol->setTopLeftPosition(xposTxt, PADDING + 50);
		txtMatchTol->setTopLeftPosition(xposTxt, PADDING + 80);
		lblFootTol->setTopLeftPosition(xposLbl, PADDING + 20);
		lblHandTol->setTopLeftPosition(xposLbl, PADDING + 50);
		lblMatchTol->setTopLeftPosition(xposLbl, PADDING + 80);
		txtFootTol->addListener(this);

		// fill with debug infos:
		txtFootTol->setText(juce::String(processor.getParam(KinectInputPluginProcessor::ParameterType::FOOT_TOL)), false);
		txtHandTol->setText(juce::String(processor.getParam(KinectInputPluginProcessor::ParameterType::HAND_TOL)), false);
		txtMatchTol->setText(juce::String(processor.getParam(KinectInputPluginProcessor::ParameterType::MATCH_TOL)), false);

		// add gui components to editor
		addAndMakeVisible(txtFootTol);
		addAndMakeVisible(txtHandTol);
		addAndMakeVisible(txtMatchTol);
		addAndMakeVisible(lblFootTol);
		addAndMakeVisible(lblHandTol);
		addAndMakeVisible(lblMatchTol);
		addAndMakeVisible(display);

		// start thread for asynchronous repainting
		repaintThread = std::thread(&KinectPluginEditor::repaint, this);
		if (!repaintThread.joinable())
		{
			ttmm::logger.write("Failed to start the Repaint Thread");
		}
	}

	KinectPluginEditor::~KinectPluginEditor() 
	{
		ttmm::logger.write("Closing Plugin Window");
		opened = false;
		repaintThread.join();

		if (repaintThread.joinable())
		{
			ttmm::logger.write(
				"Repaint thread still running but it shouldn't. That's weird.");
		}
	}

	void KinectPluginEditor::textEditorTextChanged(TextEditor& e)
	{
		if (e.isEmpty()) 
		{
			// yet not defined
			return;
		}
		int tol = e.getText().getIntValue();
		if (e.getName().toStdString() == "txtFootTol")
		{
			processor.updateParam(KinectInputPluginProcessor::ParameterType::FOOT_TOL, tol);
		}
		if (e.getName().toStdString() == "txtHandTol")
		{
			processor.updateParam(KinectInputPluginProcessor::ParameterType::HAND_TOL, tol);
		}
		if (e.getName().toStdString() == "txtMatchTol")
		{
			processor.updateParam(KinectInputPluginProcessor::ParameterType::MATCH_TOL, tol);
		}
	}

	void KinectPluginEditor::paint(Graphics& g)
	{
		//// fill the whole window white
		g.fillAll(Colours::white);
	}
	
}