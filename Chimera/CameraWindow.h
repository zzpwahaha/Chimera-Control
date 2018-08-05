#pragma once

#include "CameraSettingsControl.h"
#include "ColorBox.h"
#include "PictureStats.h"
#include "PictureManager.h"
#include "AlertSystem.h"
#include "DataAnalysisHandler.h"
#include "ExperimentTimer.h"
#include "DataLogger.h"
#include "commonFunctions.h"
#include "atomCruncherInput.h"
#include "cameraPositions.h"
#include "commonTypes.h"
#include "Queues.h"
#include <bitset>


class MainWindow;
class ScriptingWindow;
class AuxiliaryWindow;
class BaslerWindow;


class AndorWindow : public CDialog
{
	using CDialog::CDialog;
	
	DECLARE_DYNAMIC( AndorWindow )

	public:
		/// overrides
 		AndorWindow();
 		HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
		BOOL OnInitDialog() override;
		void OnMouseMove( UINT thing, CPoint point );
		BOOL PreTranslateMessage( MSG* pMsg );
		void OnCancel() override;
		void OnSize( UINT nType, int cx, int cy );
		void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar );
		void OnTimer( UINT_PTR id );
		void OnLButtonUp( UINT stuff, CPoint loc );
		void OnRButtonUp( UINT stuff, CPoint loc );
		/// directly called by the message map or 1 simple step removed.
		void wakeRearranger( );
		LRESULT onCameraFinish( WPARAM wParam, LPARAM lParam );
		LRESULT onCameraCalFinish( WPARAM wParam, LPARAM lParam );
		LRESULT onCameraProgress( WPARAM wParam, LPARAM lParam );
		LRESULT onCameraCalProgress( WPARAM wParam, LPARAM lParam );		
		void handleDblClick( NMHDR* info, LRESULT* lResult );
		void listViewRClick( NMHDR* info, LRESULT* lResult );
		void handleSpecialGreaterThanMaxSelection();
		void handleSpecialLessThanMinSelection();
		void readImageParameters();
		void passCommonCommand( UINT id );
		void passTrigger();
		void passCameraMode();
		void passSetTemperaturePress();
		void passAlwaysShowGrid();
		void passManualSetAnalysisLocations();
		void passSetGridCorner( );
		void catchEnter();
		void setDataType( std::string dataType );
		/// auxiliary functions.
		void calibrate ( );
		dataPoint getMainAnalysisResult ( );
		void checkCameraIdle( );
		void handleEmGainChange();
		void fillMasterThreadInput( MasterThreadInput* input );
		DataLogger* getLogger();
		std::string getSystemStatusString();
		void loadFriends(MainWindow* mainWin, ScriptingWindow* scriptWin, AuxiliaryWindow* auxWin, 
						  BaslerWindow* basWin);
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleMasterConfigSave(std::stringstream& configStream);
		void handleMasterConfigOpen(std::stringstream& configStream, Version version);
		void handlePictureEditChange(UINT id);
		void handleOpeningConfig(std::ifstream& configFile, Version ver );
		void redrawPictures( bool andGrid );
		void changeBoxColor( systemInfo<char> colors );
		cToolTips getToolTips();
		bool getCameraStatus();
		void setTimerText( std::string timerText );
		void prepareAndor( ExperimentInput& input );
		void startCamera();
		std::string getStartMessage();
		void setEmGain();
		void handlePictureSettings( UINT id );
		bool cameraIsRunning();
		void abortCameraRun();
		void handleAutoscaleSelection();
		void assertOff();
		void passPictureSettings( UINT id );
		void prepareAtomCruncher( ExperimentInput& input );
		void preparePlotter( ExperimentInput& input );
		static UINT __stdcall atomCruncherProcedure(void* input);
		void writeVolts( UINT currentVoltNumber, std::vector<float64> data );
		friend void commonFunctions::handleCommonMessage( int msgID, CWnd* parent, MainWindow* mainWin, 
														  ScriptingWindow* scriptWin, AndorWindow* camWin, 
														  AuxiliaryWindow* masterWin, BaslerWindow* basWin );
		void passAtomGridCombo( );
		void passDelGrid( );
		void startAtomCruncher(ExperimentInput& input);
		void startPlotterThread( ExperimentInput& input );
		bool wantsAutoPause( );
		std::atomic<bool>* getSkipNextAtomic();
		void stopPlotter( );
		void stopSound( );
		void handleImageDimsEdit(UINT id );
		void loadCameraCalSettings( ExperimentInput& input );
		bool wasJustCalibrated( );
		bool wantsAutoCal( );
	private:
		bool justCalibrated=false;
		DECLARE_MESSAGE_MAP();

		AndorCamera Andor;
		AndorCameraSettingsControl CameraSettings;
		PictureManager pics;

		ColorBox box;
		PictureStats stats;
		AlertSystem alerts;
		ExperimentTimer timer;		
		// these two could probably be combined in a sensible way.
		DataAnalysisControl analysisHandler;
		DataLogger dataHandler;

		MainWindow* mainWin;
		ScriptingWindow* scriptWin;
		AuxiliaryWindow* auxWin;
		BaslerWindow* basWin;

		cToolTips tooltips;
		coordinate selectedPixel = { 0,0 };
		CMenu menu;
		// some picture menu options
		bool autoScalePictureData;
		bool alwaysShowGrid;
		bool specialLessThanMin;
		bool specialGreaterThanMax;
		bool realTimePic;
		// plotting stuff;
		HANDLE plotThreadHandle;
		imageQueue imQueue;
		std::mutex imageLock;
		std::condition_variable rearrangerConditionVariable;
		// the following two queues and locks aren't directly used by the camera window, but the camera window
		// distributes them to the threads that do use them.

		multiGridAtomQueue plotterAtomQueue;
		multiGridImageQueue plotterPictureQueue;
		atomQueue rearrangerAtomQueue;

		// 
		std::mutex plotLock;
		std::mutex rearrangerLock;
		HANDLE atomCruncherThreadHandle;
		std::atomic<bool> atomCrunchThreadActive;
		// 
		std::atomic<bool> plotThreadActive;
		std::atomic<bool> plotThreadAborting = false;
		std::atomic<bool> skipNext=false;
		std::vector<double> plotterKey;
		chronoTimes imageTimes, imageGrabTimes, mainThreadStartTimes, crunchSeesTimes, crunchFinTimes;		
		std::vector<PlotDialog*> activePlots;
		UINT mostRecentPicNum = 0;
		UINT currentPictureNum = 0;
		std::vector<long> avgBackground;
};



