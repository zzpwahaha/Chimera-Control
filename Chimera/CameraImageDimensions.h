// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "Version.h"
#include "imageParameters.h"

struct cameraPositions;
class AndorWindow;
class MainWindow;


class ImageDimsControl
{
	public:
		ImageDimsControl();
		void initialize( cameraPositions& pos, CWnd* parentWindow, bool isTriggerModeSensitive, int& id );
		imageParameters readImageParameters();
		void setImageParametersFromInput( imageParameters param, AndorWindow* camWin );
		bool checkReady();
		void handleSave( std::ofstream& saveFile );
		void handleNew( std::ofstream& newfile );
		void handleOpen( std::ifstream& openFile, Version ver );
		imageParameters getImageParameters();
		void rearrange( AndorRunModes cameraMode, AndorTriggerMode triggerMode, int width, int height, fontMap fonts );
		HBRUSH colorEdits( HWND window, UINT message, WPARAM wParam, LPARAM lParam, MainWindow* mainWin );
		void drawBackgrounds( AndorWindow* camWin );
	private:
		Control<CStatic> leftText;
		Control<CStatic>  rightText;
		Control<CStatic>  horBinningText;
		Control<CStatic>  bottomLabel;
		Control<CStatic>  topLabel;
		Control<CStatic>  vertBinningText;
		Control<CEdit>  leftEdit;
		Control<CEdit> rightEdit;
		Control<CEdit> horBinningEdit;
		Control<CEdit> bottomEdit;
		Control<CEdit> topEdit;
		Control<CEdit> vertBinningEdit;
		bool isReady;
		imageParameters currentImageParameters;
};
;