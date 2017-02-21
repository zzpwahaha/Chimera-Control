
#pragma once

#include "Control.h"
#include <Windows.h>
#include <unordered_map>

class Repetitions
{
	public:
		void initialize(POINT& pos, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, int& id);
		void setRepetitions(unsigned int number);
		unsigned int getRepetitionNumber();
		INT_PTR handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, std::unordered_map<std::string, HBRUSH> brushes);
		void handleButtonPush();
	private:
		unsigned int repetitionNumber;
		Control<CEdit> repetitionEdit;
		Control<CEdit> repetitionDisp;
		Control<CButton> setRepetitionButton;
};
