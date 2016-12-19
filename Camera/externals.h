#pragma once
#include "Windows.h"
#include "ATMCD32D.H"
#include <vector>
#include <boost/tuple/tuple.hpp>
#include "gnuplot-iostream.h"
#include <array>
#include <deque>
#include "Control.h"
#include "PlottingInfo.h"
#include "fitsio.h"
#include "ConfigurationFileSystem.h"
#include "ExperimentTimer.h"
#include "DataFileSystem.h"
#include "SMSTextingControl.h"
#include "DataAnalysisHandler.h"
#include "CameraImageParameters.h"
#include "AlertSystem.h"
#include "EmbeddedPythonHandler.h"
#include "PictureStats.h"
#include "PictureOptions.h"

/// THINGS THAT THE USER SETS !@$#!@#$!@#$!@$!@#$!@#$!@$#!@#$!@#$@!#$!$!@#$!@#$@!$#@!#$!@#$!@$#!#$!@#$!@#$!@#$!@#$!@$#!@$#!@$#!@$#@!#$!@$#!@$#!@#$!@$#!@$#!@$#!#
// extern bool eFitsOkay;
extern bool eSettingAnalysisLocations;
extern bool eRealTimePictures;
extern bool eAutoanalyzeData;
extern DataFileSystem eExperimentData;
extern SMSTextingControl eTextingHandler;
extern ConfigurationFileSystem eCameraFileSystem;
extern ExperimentTimer eCameraWindowExperimentTimer;
extern DataAnalysisHandler eAutoAnalysisHandler;
extern CameraImage eImageControl;
extern AlertSystem eAlerts;
extern EmbeddedPythonHandler Python;
extern PictureStats ePicStats;
extern PictureOptions ePictureOptionsControl;

extern std::array<int, 4> eCurrentMaximumPictureCount;
extern std::array<int, 4> eCurrentMinimumPictureCount;
//extern std::array<int, 4> eCurrentPicturePallete;
extern int eMinimumSliderCount;
extern int eMaximumSliderCount;
extern std::array<std::vector<std::vector<RECT> >, 4> ePixelRectangles;
extern std::pair<int, int> eCurrentlySelectedPixel;
extern int eSelectedPixelCount;

extern bool eAutoscalePictures;
extern bool eEMGainMode;
extern int eEMGainLevel;

extern unsigned int ePlottingIsSlowMessage, ePlottingCaughtUpMessage;
// Temperature stuff
extern int eCameraTemperatureSetting;
// More parameters.
extern double eKineticCycleTime;
// 
extern int eCurrentAccumulationModeTotalAccumulationNumber;
extern int eCurrentTotalVariationNumber;
extern int ePicturesPerVariation;
extern int eTotalNumberOfPicturesInSeries;
extern int eNumberOfRunsToAverage;
// // // // // // // // // //
extern std::vector<std::string> eCurrentPlotNames;

extern int ePicturesPerRepetition;
extern bool eExperimentIsRunning;

extern std::string eCurrentlySelectedCameraMode;
extern std::string eCurrentlyRunningCameraMode;
extern std::string eCurrentTriggerMode;

extern double eAccumulationTime;
extern double eAccumulationModeAccumulationNumber;

extern int ePreviousPicturesPerSubSeries;
extern int ePlottingFrequency;

extern PlottingInfo eCurrentPlottingInfo;

extern int eCurrentAnalysisSetSelectionNumber;
extern int eCurrentPixelSelectionNumber;
extern int eCurrentDataSetSelectionNumber;
extern int eCurrentPositivePictureNumber;
extern int eCurrentPositivePixelNumber;
extern int eCurrentPostSelectionPictureNumber;
extern int eCurrentPostSelectionPixelNumber;
extern int eCurrentPostSelectionConditionNumber;
extern int eRepetitionsPerVariation;

/// !@#$!@#$@!#$!@#$!@ GLOBAL VARIABLES THAT THE CODE (ONLY THE CODE) USES !@#$!@#$!@#$!@#$!@#$!@#$!@$!@#$!@#$!@#$!@$#!@#$!@#$!@#$!@#$!@#$!@#$!@#$@!#$@!#$!@#$!@
/// Data Analysis
// This vector contains all of the data points for all pixels in the current picture.
extern std::vector<std::vector<long> > eImagesOfExperiment;
// This vector contains all of the most recent imageVecs ^ that haven't yet been plotted.
extern std::vector<std::vector<long> > eImageVecQueue;
// I'm using the gnuplot program to plot, and a nice iostream header to feed commands into that program. see the iostream-gnuplot header.
extern Gnuplot ePlotter;

/// Other
// points data required to draw polyline
extern POINT* ePointsArray;
// Contains coordinates of drawing areas and is used during painting.
extern std::array<RECT, 4> eImageBackgroundAreas;
extern std::array<RECT, 4> eImageDrawAreas;
//extern RECT rect;
// an object which will contain information about our camera.
extern AndorCapabilities eCameraCapabilities;
// Indicate if cooler is on
extern BOOL eCooler;
//
extern BOOL eErrorFlag;
// Cameramodel
extern char eModel[32];   
// Settings for the camera. Currently hard-coded, but I'd like to change that at some point.
extern int eAcquisitionMode;   
extern int eReadMode;
// dims of CCD chip
extern int eXPixels, eYPixels;
// Vertical Speed Index
extern int eVerticalSpeedNumber;
// Horizontal Speed Index
extern int eHorizontalSpeedNumber;
// AD Index
extern int eADNumber;
// File Saving Info
extern std::string eFinalSaveFolder;
extern std::string eFinalSaveName;
extern int eCurrentAccumulationNumber;
extern volatile int eCurrentThreadAccumulationNumber;
// This gets set when redrawing happens to signal that I don't need to redraw my own plots.
extern bool eRedrawFlag;
extern bool eDataExists;

extern int eCurrentTempColor;

extern int eCount1;
extern int eCount2;
extern int eCount3;

/// Windows API Stuffs
extern HINSTANCE eHInst;

// Main Handles
extern HWND eCameraWindowHandle;
// Code-Edited Edit Handles (disp or text)
extern HwndControl eTriggerTextDisplayHandle, eImgLeftSideTextHandle,
			eImageBottomSideTextHandle, eImgRightSideTextHandle, eImageTopSideTextHandle, eVerticalBinningTextHandle, eHorizontalBinningTextHandle, 
			eKineticCycleTimeTextHandle, eTotalPictureNumberTextHandle, ePixel1TextDisplay, ePixel2TextDisplay, 
			eImgLeftSideDispHandle, eImgRightSideDispHandle, eHorizontalBinningDispHandle, eImageBottomSideDispHandle, eImageTopSideDispHandle,
			eVerticalBinningDispHandle,	eKineticCycleTimeDispHandle, eRepetitionsPerVariationDisp, 
			eCurrentAccumulationNumDispHandle, eMinCountDispHandle,	eMaxCountDispHandle, eVariationNumberDisp;
// error status
extern HwndControl eErrorEditHandle, eErrorClear, eErrorText;
// camera status
extern HwndControl eStatusEditHandle, eClearStatusButtonHandle, eStatusText;
// User-Edited Edit Handles
extern HwndControl eImgLeftSideEditHandle, eImageTopEditHandle, eImgRightSideEditHandle, eImageBottomEditHandle,
	eVerticalBinningEditHandle, eHorizontalBinningEditHandle, eKineticCycleTimeEditHandle, eRepetitionsPerVariationEdit, ePixel1XEditHandle,
	ePixel2XEditHandle, ePixel1YEditHandle, ePixel2YEditHandle, eVariationNumberEdit;
// Checked Box Handles
extern HwndControl eIncDataFileOptionBoxHandle;
// Button Handles
extern HwndControl eSetImageParametersButtonHandle, eSetAnalysisPixelsButtonHandle,
			eSetKineticSeriesCycleTimeButtonHandle, eSetNumberRepetitionsPerVariationButton, ePlotAverageCountsBoxHandle, 
			eSeVariationNumberButton;
// Temperature Control
extern HwndControl eSetTemperatureButtonHandle, eTempTextDisplayHandle, eCurrentTempDisplayHandle, eTempDispHandle, eTempEditHandle, eTempOffButton;

// ComboBox Handles
extern HwndControl eTriggerComboHandle, eCameraModeComboHandle;

// temp
extern HwndControl eSetAccumulationTimeButton, eAccumulationTimeEdit, eAccumulationTimeDisp, eSetAccumulationNumberButton, eAccumulationNumberEdit, 
			   eSetAccumulationNumberDisp, eSetRunningAverageNumberToAverageButton, eRunningAverageEdit, eRunningAverageDisp, eRunningAverageBox,
			   eSetPlottingFrequencyButton, ePlottingFrequencyEdit, ePlottingFrequencyDisp, eAllPlotsCombo, eAllPlotsText, eCurrentPlotsCombo, 
			   eCurrentPlotsText, eSeriesProgressBar, eSubSeriesProgressBar;
	
// Contrast Control
extern HwndControl eMinimumPictureSlider1, eMaximumPictureSlider1, eMinSliderNumberEdit1, 
			   eMaxSliderNumberEdit1, eMinSliderText1, eMaxSliderText1, eMinimumPictureSlider2, eMaximumPictureSlider2, eMinSliderNumberEdit2, 
			   eMaxSliderNumberEdit2, eMinSliderText2, eMaxSliderText2, eMinimumPictureSlider3, eMaximumPictureSlider3, eMinSliderNumberEdit3, 
			   eMaxSliderNumberEdit3, eMinSliderText3, eMaxSliderText3, eMinimumPictureSlider4, eMaximumPictureSlider4, eMinSliderNumberEdit4, 
			   eMaxSliderNumberEdit4, eMinSliderText4, eMaxSliderText4;
// EM Gain Mode
extern HwndControl eSetEMGain, eEMGainText, eEMGainEdit, eEMGainDisplay, eEMGainForceChangeButton;

// Other win32 Objects
extern HDC eDrawPallete;
extern std::array<HPALETTE, 3> eAppPalette;

extern HBRUSH eGreyGreenBrush;
extern HBRUSH eDarkGreenBrush;
extern HBRUSH eDarkRedBrush;
extern HBRUSH eDarkBlueBrush;
extern HBRUSH eGreyRedBrush;
extern HBRUSH eWhiteBrush;
extern HBRUSH eVeryGreyGreenBrush;
// This is the mutex used to make sure the data queue doesn't get read and wrote to at the same time. 
extern HANDLE ePlottingMutex;
extern HANDLE eImagesMutex;

/// Dialog controls
extern HWND eInitializeDialogBoxHandle;
extern HWND eRichEditMessageBoxRichEditHandle;
extern HWND eRichEditOkCancelMessageBoxRichEditHandle;

/// Threading Stuffs
// Used to tell the camera thread when to exit. volatile is important because this is also handled inside a thread.
extern volatile bool eThreadExitIndicator;
extern volatile bool ePlotThreadExitIndicator;

extern HANDLE eCameraThreadHandle;
extern HANDLE ePlottingThreadHandle;
// The codes that the thread passes back to the main program. 
extern unsigned int eAccMessageID, eFinMessageID, eErrMessageID;

extern std::string eDataType;

