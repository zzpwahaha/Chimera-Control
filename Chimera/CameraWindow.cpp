#include "stdafx.h"
#include "CameraWindow.h"
#include "commonFunctions.h"
#include "CameraSettingsControl.h"
#include "PlottingInfo.h"
#include "ATMCD32D.H"

CameraWindow::CameraWindow() :
	CDialog(),
	CameraSettings(&Andor),
	dataHandler(DATA_SAVE_LOCATION)
{
	// because of these lines the camera window does not need to "get friends".

};

void CameraWindow::getFriends(MainWindow* mainWin, ScriptingWindow* scriptWin, DeviceWindow* masterWin)
{
	mainWindowFriend = mainWin;
	scriptingWindowFriend = scriptWin;
	masterWindowFriend = masterWin;
}

IMPLEMENT_DYNAMIC(CameraWindow, CDialog)


BEGIN_MESSAGE_MAP(CameraWindow, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	// menu stuff
	ON_COMMAND_RANGE(MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &CameraWindow::passCommonCommand)

	ON_COMMAND_RANGE(PICTURE_SETTINGS_ID_START, PICTURE_SETTINGS_ID_END, &CameraWindow::handlePictureSettings)
	// these ids all go to the same function.
	ON_CONTROL_RANGE(EN_CHANGE, IDC_PICTURE_1_MIN_EDIT, IDC_PICTURE_1_MIN_EDIT, &CameraWindow::handlePictureEditChange)
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_1_MAX_EDIT, IDC_PICTURE_1_MAX_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_2_MIN_EDIT, IDC_PICTURE_2_MIN_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_2_MAX_EDIT, IDC_PICTURE_2_MAX_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_3_MIN_EDIT, IDC_PICTURE_3_MIN_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_3_MAX_EDIT, IDC_PICTURE_3_MAX_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_4_MIN_EDIT, IDC_PICTURE_4_MIN_EDIT, &CameraWindow::handlePictureEditChange )
	ON_CONTROL_RANGE( EN_CHANGE, IDC_PICTURE_4_MAX_EDIT, IDC_PICTURE_4_MAX_EDIT, &CameraWindow::handlePictureEditChange )
	// 
	ON_COMMAND(IDC_SET_IMAGE_PARAMETERS_BUTTON, &CameraWindow::readImageParameters)
	ON_COMMAND(IDC_SET_EM_GAIN_BUTTON, &CameraWindow::setEmGain)
	ON_COMMAND(IDC_ALERTS_BOX, &CameraWindow::passAlertPress)
	ON_COMMAND(IDC_SET_TEMPERATURE_BUTTON, &CameraWindow::passSetTemperaturePress)
	ON_COMMAND(IDC_SET_REPETITONS_PER_VARIATION_BUTTON, &CameraWindow::passRepsPerVarPress)
	ON_COMMAND(IDC_SET_VARIATION_NUMBER, &CameraWindow::passVariationNumberPress)
	ON_COMMAND(IDOK, &CameraWindow::catchEnter)
	ON_COMMAND(IDC_SET_ANALYSIS_LOCATIONS, &CameraWindow::passSetAnalysisLocations)

	ON_CBN_SELENDOK(IDC_TRIGGER_COMBO, &CameraWindow::passTrigger)
	ON_CBN_SELENDOK( IDC_CAMERA_MODE_COMBO, &CameraWindow::passCameraMode )

	ON_REGISTERED_MESSAGE( eCameraFinishMessageID, &CameraWindow::onCameraFinish )
	ON_REGISTERED_MESSAGE( eCameraProgressMessageID, &CameraWindow::onCameraProgress )

	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()

	ON_NOTIFY(LVN_COLUMNCLICK, IDC_PLOTTING_LISTVIEW, &CameraWindow::listViewLClick)
	ON_NOTIFY(NM_DBLCLK, IDC_PLOTTING_LISTVIEW, &CameraWindow::handleDblClick)

	ON_COMMAND(IDC_ALERTS_BOX, &CameraWindow::passAlertPress)
END_MESSAGE_MAP()


void CameraWindow::passSetAnalysisLocations()
{
	analysisHandler.onButtonPushed();
}


void CameraWindow::catchEnter()
{
	errBox("Hello there!");
}


void CameraWindow::passAlwaysShowGrid()
{
	if (alwaysShowGrid)
	{
		alwaysShowGrid = false;
		menu.CheckMenuItem(ID_PICTURES_ALWAYSSHOWGRID, MF_UNCHECKED);
	}
	else
	{
		alwaysShowGrid = true;
		menu.CheckMenuItem(ID_PICTURES_ALWAYSSHOWGRID, MF_CHECKED);
	}
	CDC* dc = GetDC();
	pics.setAlwaysShowGrid(alwaysShowGrid, dc);	
	ReleaseDC(dc);
	pics.setSpecialGreaterThanMax(specialGreaterThanMax);
}


void CameraWindow::passRepsPerVarPress()
{
	try
	{
		//CameraSettings.handleSetRepsPerVar();
	}
	catch (Error& err)
	{
		mainWindowFriend->getComm()->sendError(err.what());
	}
}


void CameraWindow::passVariationNumberPress()
{
	try
	{
		//CameraSettings.handleSetVarNum();
	}
	catch (Error& err)
	{
		mainWindowFriend->getComm()->sendError(err.what());
	}
}

void CameraWindow::passCameraMode()
{
	CameraSettings.handleModeChange(this);
}


void CameraWindow::abortCameraRun()
{
	int status;
	Andor.getStatus(status);
	
	if (ANDOR_SAFEMODE)
	{
		// simulate as if you needed to abort.
		status = DRV_ACQUIRING;
	}

	if (status == DRV_ACQUIRING)
	{
		Andor.abortAcquisition();
		timer.setTimerDisplay( "Aborted" );
		Andor.setIsRunningState( false );
		//ePlotThreadExitIndicator = false;
		//eThreadExitIndicator = false;
		// Wait until plotting thread is complete.
		//WaitForSingleObject( ePlottingThreadHandle, INFINITE );
		// camera is no longer running.
		try
		{
			dataHandler.closeFits();
		}
		catch (Error& err)
		{
			mainWindowFriend->getComm()->sendError(err.what());
		}
		
		if (Andor.getSettings().cameraMode != "Continuous Single Scans Mode")
		{
			
			int answer = MessageBoxA("Acquisition Aborted. Delete Data (fits_#) and (key_#) files for this run?", "Acquisition Aborted.", 
									 MB_YESNO );
			if (answer == IDYES)
			{
				try
				{
					dataHandler.deleteFitsAndKey(mainWindowFriend->getComm());
				}
				catch (Error& err)
				{
					mainWindowFriend->getComm()->sendError(err.what());
				}
			}
		}
	}
	else if (status == DRV_IDLE)
	{
		Andor.setIsRunningState(false);
	}
}

bool CameraWindow::cameraIsRunning()
{
	return Andor.isRunning();
}


void CameraWindow::handlePictureEditChange( UINT id )
{
	try
	{
		pics.handleEditChange(id);
	}
	catch (Error& err)
	{
		// these errors seem more deserving of an error box.
		errBox(err.what());
	}
}


LRESULT CameraWindow::onCameraProgress( WPARAM wParam, LPARAM lParam)
{
	unsigned long long pictureNumber = lParam;
	if (lParam == 0)
	{
		return NULL;
	}
	Andor.updatePictureNumber( pictureNumber );
	std::vector<std::vector<long>> picData;
	try
	{
		 picData = Andor.acquireImageData();
	}
	catch (Error& err)
	{
		mainWindowFriend->getComm()->sendError(err.what());
		return NULL;
	}

	if (pictureNumber == 1)
	{
		try
		{
			dataHandler.loadAndMoveKeyFile();
		}
		catch (Error& err)
		{
			mainWindowFriend->getComm()->sendError(err.what());
		}
	}

	CDC* drawer = GetDC();
	AndorRunSettings currentSettings = Andor.getSettings();
	try
	{
		
		if (realTimePic)
		{
			std::pair<int, int> minMax;
			// draw the most recent pic.
			minMax = stats.update(picData.back(), pictureNumber % currentSettings.picsPerRepetition, selectedPixel,
								  currentSettings.imageSettings.width, currentSettings.imageSettings.height,
								  pictureNumber / currentSettings.picsPerRepetition,
								  currentSettings.totalPicsInExperiment / currentSettings.picsPerRepetition);
			pics.drawPicture(drawer, pictureNumber % currentSettings.picsPerRepetition, picData.back(), minMax);
			timer.update(pictureNumber / currentSettings.picsPerRepetition, currentSettings.repetitionsPerVariation,
						 currentSettings.totalVariations, currentSettings.picsPerRepetition);
		}
		else if (pictureNumber % currentSettings.picsPerRepetition == 0)
		{

			int counter = 0;
			for (auto data : picData)
			{
				std::pair<int, int> minMax;
				minMax = stats.update(data, counter, selectedPixel, currentSettings.imageSettings.width,
									  currentSettings.imageSettings.height, pictureNumber / currentSettings.picsPerRepetition,
									  currentSettings.totalPicsInExperiment / currentSettings.picsPerRepetition);

				pics.drawPicture(drawer, counter, data, minMax);
				pics.drawDongles(drawer, selectedPixel);
				counter++;
			}
			timer.update(pictureNumber / currentSettings.picsPerRepetition, currentSettings.repetitionsPerVariation,
						 currentSettings.totalVariations, currentSettings.picsPerRepetition);
		}

	}
	catch (Error& err)
	{
		mainWindowFriend->getComm()->sendError(err.what());
	}

	ReleaseDC(drawer);
		
	/*
	// Wait until eImageVecQueue is available using the mutex.
	DWORD mutexMsg = WaitForSingleObject(plottingMutex, INFINITE);
	switch (mutexMsg)
	{
		case WAIT_OBJECT_0:
		{
			// Add data to the plotting queue, only if actually plotting something.
			/// TODO
			if (eCurrentPlotNames.size() != 0)
			{
				eImageVecQueue.push_back(eImagesOfExperiment[experimentPictureNumber]);
			}
			break;
		}
		case WAIT_ABANDONED:
		{
			// handle error...
			thrower("ERROR: waiting for the plotting mutex failed (Wait Abandoned)!\r\n");
			break;
		}
		case WAIT_TIMEOUT:
		{
			// handle error...
			thrower("ERROR: waiting for the plotting mutex failed (timout???)!\r\n");
			break;
		}
		case WAIT_FAILED:
		{
			// handle error...
			int a = GetLastError();
			thrower("ERROR: waiting for the plotting mutex failed (Wait Failed: " + std::to_string(a) + ")!\r\n");
			break;
		}
		default:
		{
			// handle error...
			thrower("ERROR: unknown response from WaitForSingleObject!\r\n");
			break;
		}
	}
	ReleaseMutex(plottingMutex);
	*/

	// write the data to the file.
	if (currentSettings.cameraMode != "Continuous Single Scans Mode")
	{
		try
		{
			dataHandler.writeFits(pictureNumber, picData[(pictureNumber-1) % currentSettings.picsPerRepetition]);
		}
		catch (Error& err)
		{
			mainWindowFriend->getComm()->sendError(err.what());
		}
	}

	/// TODO...?
	/*
	if (eCooler)
	{
	// start temp timer again when acq is complete
	SetTimer(eCameraWindowHandle, ID_TEMPERATURE_TIMER, 1000, NULL);
	}
	*/
	return 0;
}

void CameraWindow::handleSpecialLessThanMinSelection()
{
	if (specialLessThanMin)
	{
		specialLessThanMin = false;
		menu.CheckMenuItem(ID_PICTURES_LESS_THAN_MIN_SPECIAL, MF_UNCHECKED);
	}
	else
	{
		specialLessThanMin = true;
		menu.CheckMenuItem(ID_PICTURES_LESS_THAN_MIN_SPECIAL, MF_CHECKED);
	}
	pics.setSpecialLessThanMin(specialLessThanMin);
}

void CameraWindow::handleSpecialGreaterThanMaxSelection()
{
	if (specialGreaterThanMax)
	{
		specialGreaterThanMax = false;
		menu.CheckMenuItem(ID_PICTURES_GREATER_THAN_MAX_SPECIAL, MF_UNCHECKED);
	}
	else
	{
		specialGreaterThanMax = true;
		menu.CheckMenuItem(ID_PICTURES_GREATER_THAN_MAX_SPECIAL, MF_CHECKED);
	}
	pics.setSpecialGreaterThanMax(specialGreaterThanMax);
}

void CameraWindow::handleAutoscaleSelection()
{
	if (autoScalePictureData)
	{
		autoScalePictureData = false;
		menu.CheckMenuItem(ID_PICTURES_AUTOSCALEPICTURES, MF_UNCHECKED);
	}
	else
	{
		autoScalePictureData = true;
		menu.CheckMenuItem(ID_PICTURES_AUTOSCALEPICTURES, MF_CHECKED);
	}
	pics.setAutoScalePicturesOption(autoScalePictureData);
}

LRESULT CameraWindow::onCameraFinish( WPARAM wParam, LPARAM lParam )
{
	// notify the andor object that it is done.
	Andor.onFinish();
	Andor.pauseThread();
	if (alerts.soundIsToBePlayed())
	{
		alerts.playSound();
	}
	dataHandler.forceFitsClosed();
	mainWindowFriend->getComm()->sendColorBox( { /*niawg*/'-', /*camera*/'B', /*intensity*/'-' } );
	mainWindowFriend->getComm()->sendStatus( "Camera has finished taking pictures and is no longer running.\r\n" );
	CameraSettings.cameraIsOn( false );
	mainWindowFriend->handleFinish();
	return 0;
}


void CameraWindow::startCamera()
{
	// turn some buttons off.
	CameraSettings.cameraIsOn( true );
	CDC* dc = GetDC();
	pics.refreshBackgrounds( dc );
	ReleaseDC(dc);
	stats.reset();
	Andor.armCamera( this );
	/// setup fits files
	if (Andor.getSettings().cameraMode != "Video Mode")
	{
		/// TODO: also, change to HDF5
		try
		{
			dataHandler.initializeDataFiles(&analysisHandler, Andor.getSettings().imageSettings,
											Andor.getSettings().totalPicsInExperiment);
		}
		catch (Error& err)
		{
			mainWindowFriend->getComm()->sendError(err.what());
		}
	}
}

bool CameraWindow::getCameraStatus()
{
	return Andor.isRunning();
}

void CameraWindow::handleDblClick(NMHDR* info, LRESULT* lResult)
{
	analysisHandler.handleDoubleClick();
}

void CameraWindow::listViewLClick( NMHDR* info, LRESULT* lResult )
{
	
	analysisHandler.handleRClick();

}

void CameraWindow::OnLButtonUp(UINT stuff, CPoint loc)
{
	alerts.stopSound();
}

// pics looks up the location itself.
void CameraWindow::OnRButtonUp( UINT stuff, CPoint clickLocation )
{
	alerts.stopSound();
	CDC* dc = GetDC();
	try
	{
		std::pair<int, int> box = pics.handleRClick(clickLocation);
		if (box.first != -1)
		{
			selectedPixel = box;
			pics.redrawPictures(dc, selectedPixel);
		}
		ReleaseDC(dc);
	}
	catch (Error& err)
	{
		ReleaseDC(dc);
		mainWindowFriend->getComm()->sendError(err.what());
	}
}

/*
 *
 */
void CameraWindow::passSetTemperaturePress()
{
	try
	{
		CameraSettings.handleSetTemperaturePress();
	}
	catch (Error& err)
	{
		mainWindowFriend->getComm()->sendError(err.what());
	}
}

/*
 *
 */
void CameraWindow::OnTimer(UINT_PTR id)
{
	CameraSettings.handleTimer();
}
/*
 *
 */
void CameraWindow::passAlertPress()
{
	alerts.handleCheckBoxPress();
}

/*
 *
 */
void CameraWindow::passTrigger()
{
	CameraSettings.handleTriggerControl(this);
}


void CameraWindow::handlePictureSettings(UINT id)
{
	selectedPixel = { 0,0 };
	CameraSettings.handlePictureSettings(id, &Andor);
	if (CameraSettings.getSettings().picsPerRepetition == 1)
	{
		pics.setSinglePicture( this, selectedPixel, CameraSettings.readImageParameters( this ) );
	}
	else
	{
		pics.setMultiplePictures( this, selectedPixel, CameraSettings.readImageParameters( this ), 
								  CameraSettings.getSettings().picsPerRepetition );
	}
	std::array<int, 4> nums = CameraSettings.getPaletteNumbers();
	pics.setPalletes(nums);

	CRect rect;
	GetWindowRect(&rect);
	OnSize(0, rect.right - rect.left, rect.bottom - rect.top);
}


BOOL CameraWindow::PreTranslateMessage(MSG* pMsg)
{
	for (int toolTipInc = 0; toolTipInc < tooltips.size(); toolTipInc++)
	{
		tooltips[toolTipInc]->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CameraWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* scrollbar)
{
	pics.handleScroll(nSBCode, nPos, scrollbar);
}


void CameraWindow::OnSize( UINT nType, int cx, int cy )
{
	AndorRunSettings settings = CameraSettings.getSettings();
	stats.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts() );
	CameraSettings.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts() );
	box.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts() );
	pics.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts() );
	alerts.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts() );
	analysisHandler.rearrange( settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts() );
	pics.setParameters( CameraSettings.readImageParameters( this ) );
	RedrawWindow();
	CDC* dc = GetDC();
	try
	{
		pics.redrawPictures(dc, selectedPixel );
	}
	catch (Error& err)
	{
		mainWindowFriend->getComm()->sendError(err.what());
	}
	ReleaseDC(dc);
	timer.rearrange(settings.cameraMode, settings.triggerMode, cx, cy, mainWindowFriend->getFonts());



}


void CameraWindow::setEmGain()
{
	try 
	{
		CameraSettings.setEmGain( &Andor );
	}
	catch (Error& exception)
	{
		errBox( exception.what() );
	}
}


void CameraWindow::handleMasterConfigSave(std::stringstream& configStream)
{
	imageParameters settings = CameraSettings.getSettings().imageSettings;
	configStream << settings.leftBorder << " " << settings.rightBorder << " " << settings.horizontalBinning << " ";
	configStream << settings.topBorder << " " << settings.bottomBorder << " " << settings.verticalBinning << "\n";
}


void CameraWindow::handleMasterConfigOpen(std::stringstream& configStream, double version)
{
	imageParameters settings = CameraSettings.getSettings().imageSettings;
	selectedPixel = { 0,0 };
	std::string tempStr;
	try
	{
		configStream >> tempStr;
		settings.leftBorder = std::stol(tempStr);
		configStream >> tempStr;
		settings.rightBorder = std::stol(tempStr);
		configStream >> tempStr;
		settings.horizontalBinning = std::stol(tempStr);
		configStream >> tempStr;
		settings.topBorder = std::stol(tempStr);
		configStream >> tempStr;
		settings.bottomBorder = std::stol(tempStr);
		configStream >> tempStr;
		settings.verticalBinning = std::stol(tempStr);
		settings.width = (settings.rightBorder - settings.leftBorder + 1) / settings.horizontalBinning;
		settings.height = (settings.bottomBorder - settings.topBorder + 1) / settings.verticalBinning;

		CameraSettings.setImageParameters(settings, this);
		pics.setParameters(settings);
		redrawPictures(true);
	}
	catch (std::invalid_argument& err)
	{
		thrower("ERROR: Bad value seen in master configueration file while attempting to load camera settings!");
	}
	/* 

	try
	{
		redrawPictures(false);
		imageParameters parameters = CameraSettings.readImageParameters( this );
		pics.setParameters( parameters );
	}
	catch (Error& exception)
	{
		//errBox( "Error!" );
		Communicator* comm = mainWindowFriend->getComm();			
	} );
		comm->sendError(exception.whatStr() + "\r\n");
	}
	CDC* dc = GetDC();
	pics.drawGrids(dc);
	ReleaseDC(dc);
	*/
}
//comm->sendColorBox({ /*niawg*/'-', /*camera*/'R', /*intensity*/'-'


void CameraWindow::prepareCamera()
{
	if ( Andor.isRunning() )
	{
		thrower( "System is already running! Please Abort to restart.\r\n" );
	}
	if ( analysisHandler.getLocationSettingStatus() )
	{
		thrower( "Please finish selecting analysis points!" );
	}
	// biggest check here, camera settings includes a lot of things.
	CameraSettings.checkIfReady();
	try
	{
		Andor.getStatus();
		if ( ANDOR_SAFEMODE )
		{
			thrower( "DRV_IDLE" );
		}
	}
	catch ( Error& exception )
	{
		if ( exception.whatBare() != "DRV_IDLE" )
		{
			throw;
		}
	}
	CDC* dc = GetDC();
	pics.refreshBackgrounds(dc);
	ReleaseDC(dc);
	//
	CameraSettings.updatePassivelySetSettings();
	pics.setNumberPicturesActive( CameraSettings.getSettings().picsPerRepetition );

	/// start the plotting thread.
	/*
	// set default colors and linewidths on plots
	this->currentRepetitionNumber = 1;
	// Create the Mutex. This function just opens the mutex if it already exists.
	plottingMutex = CreateMutexEx( 0, NULL, FALSE, MUTEX_ALL_ACCESS );
	// prepare for start of thread.
	plotThreadExitIndicator = true;
	/// TODO!
	// start thread.

	unsigned int * plottingThreadID = NULL;
	std::vector<std::string>* argPlotNames;
	argPlotNames = new std::vector<std::string>;
	for (int plotNameInc = 0; plotNameInc < eCurrentPlotNames.size(); plotNameInc++)
	{
	argPlotNames->push_back(eCurrentPlotNames[plotNameInc]);
	}
	// clear this before starting the thread.
	eImageVecQueue.clear();
	// start the plotting thread.
	ePlottingThreadHandle = (HANDLE)_beginthreadex(0, 0, plotterProcedure, argPlotNames, 0,
	plottingThreadID);
	*/
	/// start the camera.
	Andor.setSettings( CameraSettings.getSettings() );
}

std::string CameraWindow::getStartMessage()
{
	// get selected plots
	std::vector<std::string> plots = analysisHandler.getActivePlotList();
	imageParameters currentImageParameters = CameraSettings.readImageParameters( this );
	bool errCheck = false;
	for (int plotInc = 0; plotInc < plots.size(); plotInc++)
	{
		PlottingInfo tempInfoCheck;
		if (tempInfoCheck.loadPlottingInfoFromFile( PLOT_FILES_SAVE_LOCATION + plots[plotInc] + ".plot" ) == -1)
		{
			thrower( "Failed to load a plot file: " + plots[plotInc] );
		}
		if (tempInfoCheck.getPictureNumber() != this->CameraSettings.getSettings().picsPerRepetition)
		{
			thrower( "ERROR: one of the plots selected, " + plots[plotInc] + ", is not built for the currently "
					 "selected number of pictures per experiment. Please revise either the current setting or the plot"
					 " file." );
		}
		tempInfoCheck.setGroups( analysisHandler.getAtomLocations() );
		std::vector<std::pair<int, int>> plotLocations = tempInfoCheck.getAllPixelLocations();
	}
	std::string dialogMsg;
	dialogMsg = "Starting Parameters:\r\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n";
	dialogMsg += "Current Camera Temperature Setting: " + str(
		CameraSettings.getSettings().temperatureSetting ) + "\r\n";
	dialogMsg += "Exposure Times: ";
	for (int exposureInc = 0; exposureInc < this->CameraSettings.getSettings().exposureTimes.size(); exposureInc++)
	{
		dialogMsg += str( CameraSettings.getSettings().exposureTimes[exposureInc] * 1000 ) + ", ";
	}
	dialogMsg += "\r\n";
	dialogMsg += "Image Settings: " + str( currentImageParameters.leftBorder ) + " - " + str( currentImageParameters.rightBorder ) + ", "
		+ str( currentImageParameters.topBorder ) + " - " + str( currentImageParameters.bottomBorder ) + "\r\n";
	dialogMsg += "\r\n";
	dialogMsg += "Kintetic Cycle Time: " + str( CameraSettings.getSettings().kinetiCycleTime ) + "\r\n";
	dialogMsg += "Pictures per Repetition: " + str( CameraSettings.getSettings().picsPerRepetition ) + "\r\n";
	dialogMsg += "Repetitions per Variation: " + str( CameraSettings.getSettings().totalPicsInVariation ) + "\r\n";
	dialogMsg += "Variations per Experiment: " + str( CameraSettings.getSettings().totalVariations ) + "\r\n";
	dialogMsg += "Total Pictures per Experiment: " + str( CameraSettings.getSettings().totalPicsInExperiment ) + "\r\n";
	dialogMsg += "Real-Time Atom Detection Thresholds: ";

	for (int exposureInc = 0; exposureInc < CameraSettings.getThresholds().size(); exposureInc++)
	{
		dialogMsg += str( CameraSettings.getThresholds()[exposureInc] ) + ", ";
	}

	dialogMsg += "\r\n";
	dialogMsg += "Current Plotting Options: \r\n";

	for (int plotInc = 0; plotInc < plots.size(); plotInc++)
	{
		dialogMsg += "\t" + plots[plotInc] + "\r\n";
	}

	return dialogMsg;
}

void CameraWindow::setTimerText( std::string timerText )
{
	timer.setTimerDisplay( timerText );
}


void CameraWindow::OnCancel()
{
	try
	{
		passCommonCommand( ID_FILE_MY_EXIT );
	}
	catch (Error& exception)
	{
		errBox( exception.what() );
	}
}


std::vector<CToolTipCtrl*> CameraWindow::getToolTips()
{
	return tooltips;
}


BOOL CameraWindow::OnInitDialog()
{
	Andor.initializeClass(mainWindowFriend->getComm());
	cameraPositions positions;
	// all of the initialization functions increment and use the id, so by the end it will be 3000 + # of controls.
	int id = 3000;
	positions.sPos = { 0, 0 };
	box.initialize( positions.sPos, id, this, 480, mainWindowFriend->getFonts(), tooltips );
	positions.videoPos = positions.amPos = positions.seriesPos = positions.sPos;
	alerts.initialize( positions, this, false, id, mainWindowFriend->getFonts(), tooltips );
	analysisHandler.initialize( positions, id, this, mainWindowFriend->getFonts(), tooltips, false );
	CameraSettings.initialize( positions, id, this, mainWindowFriend->getFonts(), tooltips );
	POINT position = { 480, 0 };
	stats.initialize( position, this, id, mainWindowFriend->getFonts(), tooltips );
	positions.sPos = { 757, 0 };
	timer.initialize( positions, this, false, id, mainWindowFriend->getFonts(), tooltips );
	position = { 757, 40 };
	pics.initialize( position, this, id, mainWindowFriend->getFonts(), tooltips, mainWindowFriend->getBrushes()["Dark Green"] );
	pics.setSinglePicture( this, { 0,0 }, CameraSettings.readImageParameters( this ) );
	
	Andor.setSettings(CameraSettings.getSettings());

	// load the menu
	menu.LoadMenu( IDR_MAIN_MENU );
	SetMenu( &menu );

	// final steps
	ShowWindow( SW_MAXIMIZE );
	SetTimer( NULL, 1000, NULL );

	CRect rect;
	GetWindowRect( &rect );
	OnSize( 0, rect.right - rect.left, rect.bottom - rect.top );
	return TRUE;
}


void CameraWindow::redrawPictures( bool andGrid )
{
	CDC* dc = GetDC();
	try
	{
		pics.refreshBackgrounds(dc);
		if (andGrid)
		{
			pics.drawGrids(dc);
		}
		ReleaseDC(dc);
	}
	catch (Error& err)
	{
		ReleaseDC(dc);
		mainWindowFriend->getComm()->sendError(err.what());
	}
	// currently don't attempt to redraw previous picture data.
}


HBRUSH CameraWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	brushMap brushes = mainWindowFriend->getBrushes();
	rgbMap rgbs = mainWindowFriend->getRgbs();
	HBRUSH result;
	int num = pWnd->GetDlgCtrlID();

	result = *CameraSettings.handleColor(num, pDC, mainWindowFriend->getBrushes(), mainWindowFriend->getRgbs());
	if (result) { return result; }

	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{			
			CBrush* result = box.handleColoring(num, pDC, brushes, rgbs);
			if (result)
			{
				return *result;
			}
			else
			{
				pDC->SetTextColor( rgbs["White"] );
				pDC->SetBkColor( rgbs["Medium Grey"] );
				return *brushes["Medium Grey"];
			}
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Green"]);
			return *brushes["Dark Green"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		default:
		{
			return *brushes["Light Grey"];
		}
	}
}


void CameraWindow::passCommonCommand(UINT id)
{
	try
	{
		commonFunctions::handleCommonMessage( id, this, mainWindowFriend, scriptingWindowFriend, this, 
											 masterWindowFriend );
	}
	catch (Error& err)
	{
		// catch any extra errors that handleCommonMessage doesn't explicitly handle.
		errBox( err.what() );
	}
}


void CameraWindow::assertOff()
{
	CameraSettings.cameraIsOn(false);
}


void CameraWindow::readImageParameters()
{
	selectedPixel = { 0,0 };
	try
	{
		redrawPictures(false);
		imageParameters parameters = CameraSettings.readImageParameters( this );
		pics.setParameters( parameters );
	}
	catch (Error& exception)
	{
		//errBox( "Error!" );
		Communicator* comm = mainWindowFriend->getComm();
		comm->sendColorBox( { /*niawg*/'-', /*camera*/'R', /*intensity*/'-' } );
		comm->sendError( exception.whatStr() + "\r\n" );
	}
	CDC* dc = GetDC();
	pics.drawGrids(dc);
	ReleaseDC(dc);
}

void CameraWindow::changeBoxColor(systemInfo<char> colors)
{
	box.changeColor(colors);
}
