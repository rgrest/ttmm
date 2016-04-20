#pragma once
#include "DynamicComposition.h"

namespace ttmm {
	class ListDisplay : public juce::ListBoxModel
	{
	public:
		ListDisplay();
		~ListDisplay();
		int getNumRows() override;
		void paintListBoxItem(int rowNr, Graphics& g, int width, int height, bool rowIsSelected) override;
		int addRow(String msg);

	private:
		std::vector<String>* listData;
	};
}

