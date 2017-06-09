#pragma once

#include "Control.h"
#include "commonTypes.h"

struct mainOptions
{
	bool dontActuallyGenerate;
	bool connectToMaster;
	bool getVariables;
	bool programIntensity;
};

class MainOptionsControl
{
	public:
		void initialize(int& idStart, POINT& loc, CWnd* parent, fontMap fonts, std::vector<CToolTipCtrl*>& tooltips);
		bool handleEvent(UINT id, MainWindow* comm);
		mainOptions getOptions();
		void setOptions(mainOptions options);
		void rearrange(int width, int height, fontMap fonts);
	private:
		Control<CStatic> header;
		Control<CButton> connectToMaster;
		Control<CButton> getVariables;
		Control<CButton> controlIntensity;
		mainOptions currentOptions;
};