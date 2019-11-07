#include "stdafx.h"
#include "Thrower.h"
#include "ProfileSystem.h"
#include "range.h"
#include "DmControl.h"
#include "string.h"
#include <boost/lexical_cast.hpp>


DmControl::DmControl(std::string serialNumber, bool safeMode) : defObject(serialNumber, safeMode){
	defObject.initialize();
	temp = std::vector<double>(137);
}

void DmControl::initialize(POINT loc, CWnd* parent, int count, std::string serialNumber, LONG width, UINT &control_id) {
	currentInfo.ActuatorCount = count;
	currentInfo.serialNumber = serialNumber;
	piston = std::vector<pistonButton>(count);
	POINT B[137];
	POINT A[13] = { {loc}, {loc.x - (2 * width), loc.y + width }, {loc.x - (3 * width), loc.y + 2 * width},
				{loc.x - (3 * width), loc.y + 3 * width}, {loc.x - (4 * width), loc.y + 4 * width},
				{loc.x - (4 * width), loc.y + 5 * width}, {loc.x - (4 * width), loc.y + 6 * width},
				{loc.x - (4 * width), loc.y + 7 * width} , {loc.x - (4 * width), loc.y + 8 * width},
				{loc.x - (3 * width), loc.y + 9 * width} , {loc.x - (3 * width), loc.y + 10 * width},
				{loc.x - (2 * width), loc.y + 11 * width}, {loc.x, loc.y + 12 * width } };
	int collumns[13] = { 5, 9, 11, 11, 13, 13, 13, 13, 13, 11, 11, 9, 5 };
	int w = 0;
	for (int y = 0; y < 13; y++) {
		for (int z = 0; z < collumns[y]; z++) {
			B[w] = { A[y].x, A[y].y };
			A[y].x += width;
			w++;
		}
	}
	if (count != 137) {
		thrower("GUI Only Works with acutator count of 137");
	}
	else {
		//POINT init = A[0];
		int i;
			for (i = 0; i < 137; i++) {
				piston[i].Voltage.sPos = { B[i].x, B[i].y, B[i].x + width, B[i].y + width };
				piston[i].Voltage.Create(NORM_EDIT_OPTIONS | WS_BORDER, piston[i].Voltage.sPos, parent, control_id++);
			}
	}
	programNow.sPos = { 40, 50, 90, 60 };
	programNow.Create("Program Now", NORM_PUSH_OPTIONS, programNow.sPos, parent, IDC_DM_PROGRAMNOW);
	std::vector<double> initial = std::vector<double>(137, 0.0);
	setMirror(initial.data());
	updateButtons();
}

void DmControl::handleOnPress(int i) {

	piston[i].Voltage.EnableWindow();
}

void DmControl::updateButtons() {
	defObject.readDMArray(temp);
	double value;
	for (int i = 0; i < 137; i++) {
	
		value = temp[i];
		double rounded = (int)(value * 1000.0) / 1000.0;
		std::string s1 = boost::lexical_cast<std::string>(rounded);
		piston[i].Voltage.SetWindowTextA(cstr(s1));
	}
}

void DmControl::ProgramNow() {
	std::vector<double> values;
	for (int i = 0; i < 137; i++) {
		CString s1;
		piston[i].Voltage.GetWindowTextA(s1);
		std::string s2 = str(s1, 4, false, true);
		try{
			double s3 = boost::lexical_cast<double>(s2);
			if (s3 < 0 || s3 > 1) {
				thrower("cannot enter values greater than 1 or less than 0");
				values.push_back(0.0);
			}
			else {
				values.push_back(s3);
			}
		}
		catch(Error&){
			
		}
		
	}
	defObject.loadArray(values.data());
	updateButtons();
}

void DmControl::setMirror(double *A) {
	defObject.loadArray(A);
	updateButtons(); //pass an array and program it

}

int DmControl::getActNum() {
	return defObject.getActCount();
}

HBRUSH DmControl::handleColorMessage(CWnd* window, CDC* cDC)
{
	DWORD controlID = window->GetDlgCtrlID();
	if (controlID == onOffLabel.GetDlgCtrlID())
	{
		cDC->SetBkColor(_myRGBs["Static-Bkgd"]);
		cDC->SetTextColor(_myRGBs["Text"]);
		return *_myBrushes["Static-Bkgd"];
	}
	else
	{
		return NULL;
	}
}

