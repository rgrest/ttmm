#include "ListDisplay.h"

namespace ttmm
{
	ListDisplay::ListDisplay()
	{
		listData = new std::vector<String>();
	}

	ListDisplay::~ListDisplay()
	{
		listData->clear();
		listData->~vector();
	}

	int ListDisplay::getNumRows()
	{
		return listData->size();
	}

	int ListDisplay::addRow(String msg)
	{
		listData->push_back(msg);
		return listData->size();
	}

	void ListDisplay::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
	{
		if (rowIsSelected) {
			g.fillAll(Colours::lightblue);
		}
		g.setColour(Colours::black);
		g.setFont(height * 0.7f);
		g.drawText("Row Number " + String(rowNumber + 1), 5, 0, width, height, Justification::centredLeft, true);
	}
}