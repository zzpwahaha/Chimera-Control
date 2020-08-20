#include "stdafx.h"
#include "QtAndorWindow.h"
#include "Agilent/AgilentSettings.h"
#include <qdesktopwidget.h>
#include <PrimaryWindows/QtScriptWindow.h>
#include <PrimaryWindows/QtAndorWindow.h>
#include <PrimaryWindows/QtAuxiliaryWindow.h>
#include <PrimaryWindows/QtMainWindow.h>
#include <PrimaryWindows/QtBaslerWindow.h>
#include <PrimaryWindows/QtDeformableMirrorWindow.h>
#include <RealTimeDataAnalysis/AnalysisThreadWorker.h>
#include <RealTimeDataAnalysis/AtomCruncherWorker.h>
#include <ExperimentThread/ExpThreadWorker.h>
#include <QThread.h>
#include <qdebug.h>


QtAndorWindow::QtAndorWindow (QWidget* parent) : IChimeraQtWindow (parent),
	andorSettingsCtrl (),
	dataHandler (DATA_SAVE_LOCATION, this),
	andor (ANDOR_SAFEMODE),
	pics (false, "ANDOR_PICTURE_MANAGER", false, Qt::SmoothTransformation),
	imagingPiezo (this, IMG_PIEZO_INFO),
	analysisHandler (this)
{
	statBox = new ColorBox ();
	setWindowTitle ("Andor Window");
}

QtAndorWindow::~QtAndorWindow (){}

int QtAndorWindow::getDataCalNum () {
	return dataHandler.getCalibrationFileIndex ();
}

void QtAndorWindow::initializeWidgets (){
	andor.initializeClass (this, &imageTimes);
	POINT position = { 0,25 };
	statBox->initialize (position, this, 480, mainWin->getDevices (), 2);
	alerts.alertMainThread (0);
	alerts.initialize (position, this);
	analysisHandler.initialize (position, this);
	andorSettingsCtrl.initialize (position, this);
	imagingPiezo.initialize (position, this, 480, { "Horizontal Pzt.", "Disconnected", "Vertical Pzt." });
	position = { 480, 25 };
	stats.initialize (position, this);
	for (auto pltInc : range (6)){
		std::vector<pPlotDataVec> nodata (0);
		mainAnalysisPlots.push_back (new PlotCtrl (1, plotStyle::ErrorPlot, { 0,0,0,0 }, "INACTIVE", false, false));
		mainAnalysisPlots.back ()->init (position, 315, 130, this);
	}
	position = { 797, 25 };
	timer.initialize (position, this);
	position = { 797, 65 };
	pics.initialize (position, _myBrushes["Dark Green"], 530 * 2, 460 * 2 + 5, this);
	// end of literal initialization calls
	pics.setSinglePicture (andorSettingsCtrl.getSettings ().andor.imageSettings);
	andor.setSettings (andorSettingsCtrl.getSettings ().andor);


	QTimer* timer = new QTimer (this);
	connect (timer, &QTimer::timeout, [this]() {
		auto temp = andor.getTemperature ();
		andorSettingsCtrl.changeTemperatureDisplay (temp); 
		});
	timer->start (2000);
}

void QtAndorWindow::handlePrepareForAcq (void* lparam){
	reportStatus ("Preparing Andor Window for Acquisition...\n");
	AndorRunSettings* settings = (AndorRunSettings*)lparam;
	armCameraWindow (settings);
	completeCruncherStart ();
	completePlotterStart ();
}


void QtAndorWindow::handlePlotPop (unsigned id){
	for (auto& plt : mainAnalysisPlots)	{
	}
}

LRESULT QtAndorWindow::onBaslerFinish (WPARAM wParam, LPARAM lParam){
	if (!cameraIsRunning ()){
		dataHandler.normalCloseFile ();
	}
	return 0;
}


bool QtAndorWindow::wasJustCalibrated (){
	return justCalibrated;
}


bool QtAndorWindow::wantsAutoCal (){
	return andorSettingsCtrl.getAutoCal ();
}

void QtAndorWindow::writeVolts (unsigned currentVoltNumber, std::vector<float64> data){
	try	{
		dataHandler.writeVolts (currentVoltNumber, data);
	}
	catch (ChimeraError& err){
		reportErr (qstr (err.trace ()));
	}
}


void QtAndorWindow::handleImageDimsEdit (){
	try {
		pics.setParameters (andorSettingsCtrl.getSettings ().andor.imageSettings);
		QPainter painter (this);
		pics.redrawPictures (selectedPixel, analysisHandler.getAnalysisLocs (), analysisHandler.getGrids (), true,
			mostRecentPicNum, painter);
	}
	catch (ChimeraError& err){
		reportErr (qstr (err.trace ()));
	}
}


void QtAndorWindow::handleEmGainChange (){
	try {
		auto runSettings = andor.getAndorRunSettings ();
		andorSettingsCtrl.setEmGain (runSettings.emGainModeIsOn, runSettings.emGainLevel);
		auto settings = andorSettingsCtrl.getSettings ();
		runSettings.emGainModeIsOn = settings.andor.emGainModeIsOn;
		runSettings.emGainLevel = settings.andor.emGainLevel;
		andor.setSettings (runSettings);
		// and immediately change the EM gain mode.
		try	{
			andor.setGainMode ();
		}
		catch (ChimeraError& err){
			// this can happen e.g. if the camera is aquiring.
			reportErr (qstr (err.trace ()));
		}
	}
	catch (ChimeraError err){
		reportErr (qstr (err.trace ()));
	}
}


std::string QtAndorWindow::getSystemStatusString (){
	std::string statusStr;
	statusStr = "\nAndor Camera:\n";
	if (!ANDOR_SAFEMODE){
		statusStr += "\tCode System is Active!\n";
		statusStr += "\t" + andor.getSystemInfo ();
	}
	else{
		statusStr += "\tCode System is disabled! Enable in \"constants.h\"\n";
	}
	return statusStr;
}

void QtAndorWindow::windowSaveConfig (ConfigStream& saveFile){
	andorSettingsCtrl.handleSaveConfig (saveFile);
	pics.handleSaveConfig (saveFile);
	analysisHandler.handleSaveConfig (saveFile);
	imagingPiezo.handleSaveConfig (saveFile);
}

void QtAndorWindow::windowOpenConfig (ConfigStream& configFile){
	AndorRunSettings camSettings;
	try	{
		ProfileSystem::stdGetFromConfig (configFile, andor, camSettings);
		andorSettingsCtrl.setRunSettings (camSettings);
		andorSettingsCtrl.updateImageDimSettings (camSettings.imageSettings);
		andorSettingsCtrl.updateRunSettingsFromPicSettings ();
	}
	catch (ChimeraError& err){
		reportErr (qstr("Failed to get Andor Camera Run settings from file! " + err.trace ()));
	}
	try	{
		auto picSettings = ProfileSystem::stdConfigGetter (configFile, "PICTURE_SETTINGS",
			AndorCameraSettingsControl::getPictureSettingsFromConfig);
		andorSettingsCtrl.updatePicSettings (picSettings);
	}
	catch (ChimeraError& err)	{
		reportErr (qstr ("Failed to get Andor Camera Picture settings from file! " + err.trace ()));
	}
	try	{
		ProfileSystem::standardOpenConfig (configFile, pics.configDelim, &pics, Version ("4.0"));
	}
	catch (ChimeraError&)	{
		reportErr ("Failed to load picture settings from config!");
	}
	try	{
		ProfileSystem::standardOpenConfig (configFile, "DATA_ANALYSIS", &analysisHandler, Version ("4.0"));
	}
	catch (ChimeraError&){
		reportErr ("Failed to load Data Analysis settings from config!");
	}
	try	{
		pics.resetPictureStorage ();
		std::array<int, 4> nums = andorSettingsCtrl.getSettings ().palleteNumbers;
		pics.setPalletes (nums);
	}
	catch (ChimeraError& e){
		reportErr (qstr ("Andor Camera Window failed to read parameters from the configuration file.\n\n" + e.trace ()));
	}
	ProfileSystem::standardOpenConfig (configFile, imagingPiezo.getConfigDelim (), &imagingPiezo, Version ("5.3"));
}

void QtAndorWindow::passAlwaysShowGrid (){
	if (alwaysShowGrid)	{
		alwaysShowGrid = false;
	}
	else{
		alwaysShowGrid = true;
	}
	pics.setSpecialGreaterThanMax (specialGreaterThanMax);
}

void QtAndorWindow::abortCameraRun (){
	int status = andor.queryStatus ();
	if (ANDOR_SAFEMODE)	{
		// simulate as if you needed to abort.
		status = DRV_ACQUIRING;
	}
	if (status == DRV_ACQUIRING){
		andor.abortAcquisition ();
		timer.setTimerDisplay ("Aborted");
		andor.setIsRunningState (false);
		// close the plotting thread.
		plotThreadAborting = true;
		plotThreadActive = false;
		atomCrunchThreadActive = false;
		// Wait until plotting thread is complete.
		while (true){
			auto res = WaitForSingleObject (plotThreadHandle, 2e3);
			if (res == WAIT_TIMEOUT){
				auto answer = QMessageBox::question (this, qstr ("Close Real-Time Plotting?"), 
					"The real time plotting thread is taking a while to close. Continue waiting?");
				if (answer == QMessageBox::No) {
					// This might indicate something about the code is gonna crash...
					break;
				}
			}
			else{
				break;
			}
		}
		plotThreadAborting = false;
		// camera is no longer running.
		try	{
			dataHandler.normalCloseFile ();
		}
		catch (ChimeraError& err)	{
			reportErr (qstr (err.trace ()));
		}

		if (andor.getAndorRunSettings ().acquisitionMode != AndorRunModes::mode::Video){
			auto answer = QMessageBox::question(this, qstr("Delete Data?"), qstr("Acquisition Aborted. Delete Data "
				"file (data_" + str (dataHandler.getDataFileNumber ()) + ".h5) for this run?"));
			if (answer == QMessageBox::Yes){
				try	{
					dataHandler.deleteFile ();
				}
				catch (ChimeraError& err) {
					reportErr (qstr (err.trace ()));
				}
			}
		}
	}
	else if (status == DRV_IDLE) {
		andor.setIsRunningState (false);
	}
}

bool QtAndorWindow::cameraIsRunning (){
	return andor.isRunning ();
}

void QtAndorWindow::onCameraProgress (int picNumReported){
	currentPictureNum++;
	//qDebug () << currentPictureNum << picNumReported;
	unsigned picNum = currentPictureNum;
	if (picNum % 2 == 1){
		mainThreadStartTimes.push_back (std::chrono::high_resolution_clock::now ());
	}
	AndorRunSettings curSettings = andor.getAndorRunSettings ();
	if (picNumReported == -1){
		// last picture.
		picNum = curSettings.totalPicsInExperiment();
	}
	if (picNumReported != currentPictureNum && picNumReported != -1){
		if (curSettings.acquisitionMode != AndorRunModes::mode::Video){
			//reportErr ( "WARNING: picture number reported by andor isn't matching the"
			//								  "camera window record?!?!?!?!?" );
		}
	}
	// need to call this before acquireImageData().
	andor.updatePictureNumber (picNum);
	std::vector<Matrix<long>> rawPicData;
	try	{
		rawPicData = andor.acquireImageData ();
	}
	catch (ChimeraError& err){
		reportErr (qstr (err.trace ()));
		mainWin->pauseExperiment ();
		return;
	}
	std::vector<Matrix<long>> calPicData (rawPicData.size ());
	if (andorSettingsCtrl.getUseCal () && avgBackground.size () == rawPicData.front ().size ()){
		for (auto picInc : range (rawPicData.size ())){
			calPicData[picInc] = Matrix<long> (rawPicData[picInc].getRows (), rawPicData[picInc].getCols (), 0);
			for (auto pixInc : range (rawPicData[picInc].size ()))
			{
				calPicData[picInc].data[pixInc] = (rawPicData[picInc].data[pixInc] - avgBackground.data[pixInc]);
			}
		}
	}
	else { calPicData = rawPicData; }

	if (picNum % 2 == 1){
		imageGrabTimes.push_back (std::chrono::high_resolution_clock::now ());
	}
	emit newImage ({ picNum, calPicData[(picNum - 1) % curSettings.picsPerRepetition] }); 

	auto picsToDraw = andorSettingsCtrl.getImagesToDraw (calPicData);
	try
	{
		if (realTimePic){
			std::pair<int, int> minMax;
			// draw the most recent pic.
			minMax = stats.update (picsToDraw.back (), picNum % curSettings.picsPerRepetition, selectedPixel,
				picNum / curSettings.picsPerRepetition,
				curSettings.totalPicsInExperiment () / curSettings.picsPerRepetition);
			pics.drawBitmap (picsToDraw.back (), minMax, picNum % curSettings.picsPerRepetition,
				analysisHandler.getAnalysisLocs (), analysisHandler.getGrids (), picNum, 
				analysisHandler.getDrawGridOption ());

			timer.update (picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
				curSettings.totalVariations, curSettings.picsPerRepetition);
		}
		else if (picNum % curSettings.picsPerRepetition == 0){
			int counter = 0;
			for (auto data : picsToDraw){
				std::pair<int, int> minMax;
				minMax = stats.update (data, counter, selectedPixel, picNum / curSettings.picsPerRepetition,
					curSettings.totalPicsInExperiment () / curSettings.picsPerRepetition);
				if (minMax.second > 50000){
					numExcessCounts++;
					if (numExcessCounts > 2){
						// POTENTIALLY DANGEROUS TO CAMERA.
						// AUTO PAUSE THE EXPERIMENT. 
						// This can happen if a laser, particularly the axial raman laser, is left on during an image.
						// cosmic rays may occasionally trip it as well.
						reportErr ("No MOT and andor win wants auto pause!");
						commonFunctions::handleCommonMessage (ID_ACCELERATOR_F2, this);
						errBox ("EXCCESSIVE CAMERA COUNTS DETECTED!!!");
					}
				}
				else{
					numExcessCounts = 0;
				}
				pics.drawBitmap (data, minMax, counter, analysisHandler.getAnalysisLocs (),
								 analysisHandler.getGrids (), picNum+counter, analysisHandler.getDrawGridOption ());
				counter++;
			}
			timer.update (picNum / curSettings.picsPerRepetition, curSettings.repetitionsPerVariation,
				curSettings.totalVariations, curSettings.picsPerRepetition);
		}
	}
	catch (ChimeraError& err){
		reportErr (qstr (err.trace ()));
		mainWin->pauseExperiment ();
	}
	// write the data to the file.
	if (curSettings.acquisitionMode != AndorRunModes::mode::Video){
		try	{
			// important! write the original raw data, not the pic-to-draw, which can be a difference pic, or the calibrated
			// pictures, which can have the background subtracted.
			dataHandler.writeAndorPic (rawPicData[(picNum - 1) % curSettings.picsPerRepetition],
									   curSettings.imageSettings);
		}
		catch (ChimeraError& err){
			reportErr (err.qtrace ());
			try {
				mainWin->pauseExperiment ();
			}
			catch (ChimeraError & err2) {
				reportErr (err2.qtrace ());
			}
		}
	}
	mostRecentPicNum = picNum;
}

void QtAndorWindow::handleSetAnalysisPress (){
	analysisHandler.setGridCornerLocation (pics.getSelLocation ());
	analysisHandler.saveGridParams ();
}

void QtAndorWindow::wakeRearranger (){
	std::unique_lock<std::mutex> lock (rearrangerLock);
	rearrangerConditionVariable.notify_all ();
}


void QtAndorWindow::handleSpecialLessThanMinSelection (){
	if (specialLessThanMin)	{
		specialLessThanMin = false;
		//mainWin->checkAllMenus (ID_PICTURES_LESS_THAN_MIN_SPECIAL, MF_UNCHECKED);
	}
	else{
		specialLessThanMin = true;
		//mainWin->checkAllMenus (ID_PICTURES_LESS_THAN_MIN_SPECIAL, MF_CHECKED);
	}
	pics.setSpecialLessThanMin (specialLessThanMin);
}


void QtAndorWindow::handleSpecialGreaterThanMaxSelection ()
{
	if (specialGreaterThanMax){
		specialGreaterThanMax = false;
		//mainWin->checkAllMenus (ID_PICTURES_GREATER_THAN_MAX_SPECIAL, MF_UNCHECKED);
	}
	else{
		specialGreaterThanMax = true;
		//mainWin->checkAllMenus (ID_PICTURES_GREATER_THAN_MAX_SPECIAL, MF_CHECKED);
	}
	pics.setSpecialGreaterThanMax (specialGreaterThanMax);
}


void QtAndorWindow::handleAutoscaleSelection ()
{
	if (autoScalePictureData){
		autoScalePictureData = false;
		//mainWin->checkAllMenus (ID_PICTURES_AUTOSCALEPICTURES, MF_UNCHECKED);
	}
	else{
		autoScalePictureData = true;
		//mainWin->checkAllMenus (ID_PICTURES_AUTOSCALEPICTURES, MF_CHECKED);
	}
	pics.setAutoScalePicturesOption (autoScalePictureData);
}


LRESULT QtAndorWindow::onCameraCalFinish (WPARAM wParam, LPARAM lParam){
	// notify the andor object that it is done.
	andor.onFinish ();
	andor.pauseThread ();
	andor.setCalibrating (false);
	justCalibrated = true;
	andorSettingsCtrl.cameraIsOn (false);
	// normalize.
	for (auto& p : avgBackground){
		p /= 100.0;
	}
	// if auto cal is selected, always assume that the user was trying to start with F5.
	if (andorSettingsCtrl.getAutoCal ()){
		//PostMessageA (WM_COMMAND, MAKEWPARAM (ID_ACCELERATOR_F5, 0));
	}
	return 0;
}

dataPoint QtAndorWindow::getMainAnalysisResult (){
	return mostRecentAnalysisResult;
}

std::mutex& QtAndorWindow::getActivePlotMutexRef (){
	return activePlotMutex;
}

void QtAndorWindow::cleanUpAfterExp (){
	plotThreadActive = false;
	atomCrunchThreadActive = false;
	dataHandler.normalCloseFile ();
}

LRESULT QtAndorWindow::onCameraFinish (WPARAM wParam, LPARAM lParam){
	// notify the andor object that it is done.
	andor.onFinish ();
	if (alerts.soundIsToBePlayed ()){
		alerts.playSound ();
	}
	reportStatus( "Andor has finished taking pictures and is no longer running.\r\n" );
	andorSettingsCtrl.cameraIsOn (false);
	// rearranger thread handles these right now.
	mainThreadStartTimes.clear ();

	crunchFinTimes.clear ();
	crunchSeesTimes.clear ();
	scriptWin->stopRearranger ();
	wakeRearranger ();
	{
		std::lock_guard<std::mutex> lock (activePlotMutex);
		if (activeDlgPlots.size () != 0){
			mostRecentAnalysisResult = activeDlgPlots.back ()->getMainAnalysisResult ();
		}
	}
	return 0;
}


int QtAndorWindow::getMostRecentFid (){
	return dataHandler.getDataFileNumber ();

}

int QtAndorWindow::getPicsPerRep (){
	return andorSettingsCtrl.getSettings ().andor.picsPerRepetition;
}

std::string QtAndorWindow::getMostRecentDateString (){
	return dataHandler.getMostRecentDateString ();
}


bool QtAndorWindow::wantsThresholdAnalysis (){
	return analysisHandler.wantsThresholdAnalysis ();
}

atomGrid QtAndorWindow::getMainAtomGrid (){
	return analysisHandler.getAtomGrid (0);
}


void QtAndorWindow::armCameraWindow (AndorRunSettings* settings){
	//readImageParameters ();
	pics.setNumberPicturesActive (settings->picsPerRepetition);
	if (settings->picsPerRepetition == 1){
		pics.setSinglePicture (settings->imageSettings);
	}
	else{
		pics.setMultiplePictures (settings->imageSettings, settings->picsPerRepetition);
	}
	pics.resetPictureStorage ();
	pics.setParameters (settings->imageSettings);
	redrawPictures (false);
	andorSettingsCtrl.setRepsPerVariation (settings->repetitionsPerVariation);
	andorSettingsCtrl.setVariationNumber (settings->totalVariations);
	pics.setSoftwareAccumulationOptions (andorSettingsCtrl.getSoftwareAccumulationOptions ());
	try {
		andor.preparationChecks ();
	}
	catch (ChimeraError & err) {
		reportErr (err.qtrace ());
	}
	// turn some buttons off.
	andorSettingsCtrl.cameraIsOn (true);
	stats.reset ();
	analysisHandler.updateDataSetNumberEdit (dataHandler.getNextFileNumber () - 1);
}

bool QtAndorWindow::getCameraStatus (){
	return andor.isRunning ();
}

void QtAndorWindow::stopSound (){
	alerts.stopSound ();
}

void QtAndorWindow::passSetTemperaturePress (){
	try{
		if (andor.isRunning ()){
			thrower ("ERROR: the camera (thinks that it?) is running. You can't change temperature settings during camera "
				"operation.");
		}
		andorSettingsCtrl.handleSetTemperaturePress ();
		auto settings = andorSettingsCtrl.getSettings ();
		andor.setSettings (settings.andor);
		andor.setTemperature ();
	}
	catch (ChimeraError& err){
		reportErr (qstr (err.trace ()));
	}
	mainWin->updateConfigurationSavedStatus (false);
}

void QtAndorWindow::assertDataFileClosed () {
	dataHandler.assertClosed ();
}

void QtAndorWindow::handlePictureSettings (){
	selectedPixel = { 0,0 };
	andorSettingsCtrl.handlePictureSettings ();
	if (andorSettingsCtrl.getSettings ().andor.picsPerRepetition == 1){
		pics.setSinglePicture (andorSettingsCtrl.getSettings ().andor.imageSettings);
	}
	else{
		pics.setMultiplePictures (andorSettingsCtrl.getSettings ().andor.imageSettings,
								  andorSettingsCtrl.getSettings ().andor.picsPerRepetition);
	}
	pics.resetPictureStorage ();
	std::array<int, 4> nums = andorSettingsCtrl.getSettings ().palleteNumbers;
	pics.setPalletes (nums);
}

/*
Check that the camera is idle, or not aquiring pictures. Also checks that the data analysis handler isn't active.
*/
void QtAndorWindow::checkCameraIdle (){
	if (andor.isRunning ()){
		thrower ("Camera is already running! Please Abort to restart.\r\n");
	}
	if (analysisHandler.getLocationSettingStatus ()){
		thrower ("Please finish selecting analysis points before starting the camera!\r\n");
	}
	// make sure it's idle.
	try{
		andor.queryStatus ();
		if (ANDOR_SAFEMODE){
			thrower ("DRV_IDLE");
		}
	}
	catch (ChimeraError& exception){
		if (exception.whatBare () != "DRV_IDLE"){
			throwNested (" while querying andor status to check if idle.");
		}
	}
}

void QtAndorWindow::handleMasterConfigSave (std::stringstream& configStream){
	andorSettingsCtrl.handelSaveMasterConfig (configStream);
}


void QtAndorWindow::handleMasterConfigOpen (ConfigStream& configStream){
	mainWin->updateConfigurationSavedStatus (false);
	selectedPixel = { 0,0 };
	andorSettingsCtrl.handleOpenMasterConfig (configStream, this);
	pics.setParameters (andorSettingsCtrl.getSettings ().andor.imageSettings);
	redrawPictures (true);
}


DataLogger& QtAndorWindow::getLogger (){
	return dataHandler;
}


void QtAndorWindow::loadCameraCalSettings (AllExperimentInput& input){
	redrawPictures (false);
	try{
		checkCameraIdle ();
	}
	catch (ChimeraError& err){
		reportErr (qstr (err.trace ()));
	}
	// I used to mandate use of a button to change image parameters. Now I don't have the button and just always 
	// update at this point.
	readImageParameters ();
	pics.setNumberPicturesActive (1);
	// biggest check here, camera settings includes a lot of things.
	andorSettingsCtrl.checkIfReady ();
	// reset the image which is about to be calibrated.
	avgBackground = Matrix<long> (0, 0);
	/// start the camera.
	andor.setCalibrating (true);
}

AndorCameraCore& QtAndorWindow::getCamera (){
	return andor;
}


void QtAndorWindow::prepareAtomCruncher (AllExperimentInput& input){
	input.cruncherInput = new atomCruncherInput;
	input.cruncherInput->plotterActive = plotThreadActive;
	input.cruncherInput->imageDims = andorSettingsCtrl.getSettings ().andor.imageSettings;
	atomCrunchThreadActive = true;
	input.cruncherInput->plotterNeedsImages = input.masterInput->plotterInput->needsCounts;
	input.cruncherInput->cruncherThreadActive = &atomCrunchThreadActive;
	skipNext = false;
	input.cruncherInput->skipNext = &skipNext;
	//input.cruncherInput->imQueue = &imQueue;
	// options
	if (input.masterInput){
		auto& niawg = input.masterInput->devices.getSingleDevice< NiawgCore > ();
		input.cruncherInput->rearrangerActive = niawg.expRerngOptions.active;
	}
	else{
		input.cruncherInput->rearrangerActive = false;
	}
	input.cruncherInput->grids = analysisHandler.getGrids ();
	input.cruncherInput->thresholds = andorSettingsCtrl.getSettings ().thresholds;
	input.cruncherInput->picsPerRep = andorSettingsCtrl.getSettings ().andor.picsPerRepetition;
	input.cruncherInput->catchPicTime = &crunchSeesTimes;
	input.cruncherInput->finTime = &crunchFinTimes;
	input.cruncherInput->atomThresholdForSkip = mainWin->getMainOptions ().atomSkipThreshold;
	input.cruncherInput->rearrangerConditionWatcher = &rearrangerConditionVariable;
}


void QtAndorWindow::startAtomCruncher (AllExperimentInput& input){
	atomCruncherWorker = new CruncherThreadWorker (input.cruncherInput);
	QThread* thread = new QThread;
	atomCruncherWorker->moveToThread (thread);
	connect (thread, &QThread::started, atomCruncherWorker, &CruncherThreadWorker::init);
	connect (thread, &QThread::finished, thread, &CruncherThreadWorker::deleteLater);
	connect (this, &QtAndorWindow::newImage, atomCruncherWorker, &CruncherThreadWorker::handleImage);
	thread->start ();
}


bool QtAndorWindow::wantsAutoPause (){
	return alerts.wantsAutoPause ();
}

std::atomic<bool>& QtAndorWindow::getPlotThreadActiveRef (){
	return plotThreadActive;
}

std::atomic<HANDLE>& QtAndorWindow::getPlotThreadHandleRef (){
	return plotThreadHandle;
}

void QtAndorWindow::completeCruncherStart () {
	auto* cruncherInput = new atomCruncherInput;
	cruncherInput->plotterActive = plotThreadActive;
	cruncherInput->imageDims = andorSettingsCtrl.getSettings ().andor.imageSettings;
	atomCrunchThreadActive = true;
	cruncherInput->plotterNeedsImages = true;// input.masterInput->plotterInput->needsCounts;
	cruncherInput->cruncherThreadActive = &atomCrunchThreadActive;
	skipNext = false;
	cruncherInput->skipNext = &skipNext;
	//input.cruncherInput->imQueue = &imQueue;
	// options
	//if (input.masterInput) {
	//	auto& niawg = input.masterInput->devices.getSingleDevice< NiawgCore > ();
	//	input.cruncherInput->rearrangerActive = niawg.expRerngOptions.active;
	//}
	//else {
	//}
	cruncherInput->rearrangerActive = false;
	cruncherInput->grids = analysisHandler.getGrids ();
	cruncherInput->thresholds = andorSettingsCtrl.getSettings ().thresholds;
	cruncherInput->picsPerRep = andorSettingsCtrl.getSettings ().andor.picsPerRepetition;
	cruncherInput->catchPicTime = &crunchSeesTimes;
	cruncherInput->finTime = &crunchFinTimes;
	cruncherInput->atomThresholdForSkip = mainWin->getMainOptions ().atomSkipThreshold;
	cruncherInput->rearrangerConditionWatcher = &rearrangerConditionVariable;

	atomCruncherWorker = new CruncherThreadWorker (cruncherInput);
	QThread* thread = new QThread;
	atomCruncherWorker->moveToThread (thread);


	connect (mainWin->getExpThread(), &QThread::finished, atomCruncherWorker, &QObject::deleteLater);

	connect (thread, &QThread::started, atomCruncherWorker, &CruncherThreadWorker::init);
	connect (thread, &QThread::finished, thread, &CruncherThreadWorker::deleteLater);
	connect (this, &QtAndorWindow::newImage, atomCruncherWorker, &CruncherThreadWorker::handleImage);
	thread->start ();
}

void QtAndorWindow::completePlotterStart () {
	/// start the plotting thread.
	plotThreadActive = true;
	plotThreadAborting = false;
	auto* pltInput = new realTimePlotterInput (analysisHandler.getPlotTime ());
	//auto& pltInput = input.masterInput->plotterInput;
	pltInput->plotParentWindow = this;
	pltInput->cameraSettings = andorSettingsCtrl.getSettings ();
	pltInput->aborting = &plotThreadAborting;
	pltInput->active = &plotThreadActive;
	pltInput->imageShape = andorSettingsCtrl.getSettings ().andor.imageSettings;
	pltInput->picsPerVariation = mainWin->getRepNumber () * andorSettingsCtrl.getSettings ().andor.picsPerRepetition;
	pltInput->variations = auxWin->getTotalVariationNumber ();
	pltInput->picsPerRep = andorSettingsCtrl.getSettings ().andor.picsPerRepetition;
	pltInput->alertThreshold = alerts.getAlertThreshold ();
	pltInput->wantAtomAlerts = alerts.wantsAtomAlerts ();
	pltInput->numberOfRunsToAverage = 5;
	pltInput->plottingFrequency = analysisHandler.getPlotFreq ();
	analysisHandler.fillPlotThreadInput (pltInput);
	// remove old plots that aren't trying to sustain.
	activeDlgPlots.erase (std::remove_if (activeDlgPlots.begin (), activeDlgPlots.end (), PlotDialog::removeQuery),
		activeDlgPlots.end ());
	/*std::vector<double> dummyKey;
	dummyKey.resize (input.masterInput->numVariations);
	pltInput->key = dummyKey;
	unsigned count = 0;
	for (auto& e : pltInput->key) {
		e = count++;
	}*/
	unsigned mainPlotInc = 0;
	for (auto plotParams : pltInput->plotInfo) {
		// Create vector of data to be shared between plotter and data analysis handler. 
		std::vector<pPlotDataVec> data;
		// assume 1 data set...
		unsigned numDataSets = 1;
		// +1 for average line
		unsigned numLines = numDataSets * (pltInput->grids[plotParams.whichGrid].height
			* pltInput->grids[plotParams.whichGrid].width + 1);
		data.resize (numLines);
		for (auto& line : data) {
			line = pPlotDataVec (new plotDataVec (1, { 0, -1, 0 }));
			line->resize (1);
			// initialize x axis for all data sets.
			unsigned count = 0;
			//for (auto& keyItem : pltInput->key) {
			//	line->at (count++).x = keyItem;
			//}
		}
		bool usedDlg = false;
		plotStyle style = plotParams.isHist ? plotStyle::HistPlot : plotStyle::ErrorPlot;
		while (true) {
			if (mainPlotInc >= 6) {
				// TODO: put extra plots in dialogs.
				usedDlg = true;
				break;
			}
			break;
		}
		if (!usedDlg && mainPlotInc < 6) {
			mainAnalysisPlots[mainPlotInc]->setStyle (style);
			mainAnalysisPlots[mainPlotInc]->setThresholds (andorSettingsCtrl.getSettings ().thresholds[0]);
			mainAnalysisPlots[mainPlotInc]->setTitle (plotParams.name);
			mainPlotInc++;
		}
	}

	bool gridHasBeenSet = false;
	//auto& pltInput = input.masterInput->plotterInput;
	for (auto gridInfo : pltInput->grids) {
		if (!(gridInfo.topLeftCorner == coordinate (0, 0))) {
			gridHasBeenSet = true;
			break;
		}
	}
	if ((!gridHasBeenSet && pltInput->analysisLocations.size () == 0) || pltInput->plotInfo.size () == 0) {
		plotThreadActive = false;
	}
	else {
		// start the plotting thread
		plotThreadActive = true;
		analysisThreadWorker = new AnalysisThreadWorker (pltInput);
		QThread* thread = new QThread;
		analysisThreadWorker->moveToThread (thread);
		connect (thread, &QThread::started, analysisThreadWorker, &AnalysisThreadWorker::init);
		connect (thread, &QThread::finished, thread, &QThread::deleteLater);
		connect (thread, &QThread::finished, analysisThreadWorker, &AnalysisThreadWorker::deleteLater);
		
		connect (mainWin->getExpThreadWorker(), &ExpThreadWorker::plot_Xvals_determined,
				 analysisThreadWorker, &AnalysisThreadWorker::setXpts);
		connect (mainWin->getExpThread(), &QThread::finished, analysisThreadWorker, &QObject::deleteLater);

		connect (analysisThreadWorker, &AnalysisThreadWorker::newPlotData, this,
			[this](std::vector<std::vector<dataPoint>> data, int plotNum) {mainAnalysisPlots[plotNum]->setData (data); });
		if (atomCruncherWorker) {
			connect (atomCruncherWorker, &CruncherThreadWorker::atomArray,
				analysisThreadWorker, &AnalysisThreadWorker::handleNewPic);
			connect (atomCruncherWorker, &CruncherThreadWorker::pixArray,
				analysisThreadWorker, &AnalysisThreadWorker::handleNewPix);
		}
		thread->start ();
	}

}

void QtAndorWindow::preparePlotter (AllExperimentInput& input){
	/// start the plotting thread.
	plotThreadActive = true;
	plotThreadAborting = false;
	input.masterInput->plotterInput = new realTimePlotterInput (analysisHandler.getPlotTime ());
	auto& pltInput = input.masterInput->plotterInput;
	pltInput->plotParentWindow = this;
	pltInput->cameraSettings = andorSettingsCtrl.getSettings ();
	pltInput->aborting = &plotThreadAborting;
	pltInput->active = &plotThreadActive;
	pltInput->imageShape = andorSettingsCtrl.getSettings ().andor.imageSettings;
	pltInput->picsPerVariation = mainWin->getRepNumber () * andorSettingsCtrl.getSettings ().andor.picsPerRepetition;
	pltInput->variations = auxWin->getTotalVariationNumber ();
	pltInput->picsPerRep = andorSettingsCtrl.getSettings ().andor.picsPerRepetition;
	pltInput->alertThreshold = alerts.getAlertThreshold ();
	pltInput->wantAtomAlerts = alerts.wantsAtomAlerts ();
	pltInput->numberOfRunsToAverage = 5;
	pltInput->plottingFrequency = analysisHandler.getPlotFreq ();
	analysisHandler.fillPlotThreadInput (pltInput);
	// remove old plots that aren't trying to sustain.
	activeDlgPlots.erase (std::remove_if (activeDlgPlots.begin (), activeDlgPlots.end (), PlotDialog::removeQuery),
						  activeDlgPlots.end ());
	std::vector<double> dummyKey;
	dummyKey.resize (input.masterInput->numVariations);
	//pltInput->key = dummyKey;
	//unsigned count = 0;
	//for (auto& e : pltInput->key){
	//	e = count++;
	//}
	unsigned mainPlotInc = 0;
	for (auto plotParams : pltInput->plotInfo){
		// Create vector of data to be shared between plotter and data analysis handler. 
		std::vector<pPlotDataVec> data;
		// assume 1 data set...
		unsigned numDataSets = 1;
		// +1 for average line
		unsigned numLines = numDataSets * (pltInput->grids[plotParams.whichGrid].height
									   * pltInput->grids[plotParams.whichGrid].width + 1);
		data.resize (numLines);
		for (auto& line : data){
			line = pPlotDataVec (new plotDataVec (dummyKey.size (), { 0, -1, 0 }));
			line->resize (dummyKey.size ());
			// initialize x axis for all data sets.
			unsigned count = 0;
			for (auto& keyItem : dummyKey)	{
				line->at (count++).x = keyItem;
			}
		}
		bool usedDlg = false;
		plotStyle style = plotParams.isHist ? plotStyle::HistPlot : plotStyle::ErrorPlot;
		while (true){
			if (mainPlotInc >= 6){
				// TODO: put extra plots in dialogs.
				usedDlg = true;
				break;
			}
			break;
		}
		if (!usedDlg && mainPlotInc < 6){
			mainAnalysisPlots[mainPlotInc]->setStyle (style);
			mainAnalysisPlots[mainPlotInc]->setThresholds (andorSettingsCtrl.getSettings ().thresholds[0]);
			mainAnalysisPlots[mainPlotInc]->setTitle (plotParams.name);
			mainPlotInc++;
		}
	}
}


bool QtAndorWindow::wantsNoMotAlert (){
	if (cameraIsRunning ()){
		return alerts.wantsMotAlerts ();
	}
	else{
		return false;
	}
}

unsigned QtAndorWindow::getNoMotThreshold (){
	return alerts.getAlertThreshold ();
}

void QtAndorWindow::startPlotterThread (AllExperimentInput& input){
	bool gridHasBeenSet = false;
	auto& pltInput = input.masterInput->plotterInput;
	for (auto gridInfo : pltInput->grids){
		if (!(gridInfo.topLeftCorner == coordinate (0, 0))){
			gridHasBeenSet = true;
			break;
		}
	}
	if ((!gridHasBeenSet && pltInput->analysisLocations.size () == 0) || pltInput->plotInfo.size () == 0){
		plotThreadActive = false;
	}
	else{
		// start the plotting thread
		plotThreadActive = true;
		analysisThreadWorker = new AnalysisThreadWorker (pltInput);
		QThread* thread = new QThread;
		analysisThreadWorker->moveToThread (thread);
		connect (thread, &QThread::started, analysisThreadWorker, &AnalysisThreadWorker::init);
		connect (thread, &QThread::finished, thread, &QThread::deleteLater);
		connect (thread, &QThread::finished, analysisThreadWorker, &AnalysisThreadWorker::deleteLater);
		connect (analysisThreadWorker, &AnalysisThreadWorker::newPlotData, this, 
			[this](std::vector<std::vector<dataPoint>> data, int plotNum) {mainAnalysisPlots[plotNum]->setData (data); });
		if (atomCruncherWorker) {
			connect ( atomCruncherWorker, &CruncherThreadWorker::atomArray, 
				      analysisThreadWorker, &AnalysisThreadWorker::handleNewPic );
			connect ( atomCruncherWorker, &CruncherThreadWorker::pixArray,
					  analysisThreadWorker, &AnalysisThreadWorker::handleNewPix );
		}
		thread->start ();
	}
}

std::string QtAndorWindow::getStartMessage (){
	// get selected plots
	auto andrSttngs = andorSettingsCtrl.getSettings ().andor;
	std::vector<std::string> plots = analysisHandler.getActivePlotList ();
	imageParameters currentImageParameters = andrSttngs.imageSettings;
	bool errCheck = false;
	for (unsigned plotInc = 0; plotInc < plots.size (); plotInc++){
		PlottingInfo tempInfoCheck (PLOT_FILES_SAVE_LOCATION + "\\" + plots[plotInc] + ".plot");
		if (tempInfoCheck.getPicNumber () != andrSttngs.picsPerRepetition){
			thrower (": one of the plots selected, " + plots[plotInc] + ", is not built for the currently "
				"selected number of pictures per experiment. Please revise either the current setting or the plot"
				" file.");
		}
		tempInfoCheck.setGroups (analysisHandler.getAnalysisLocs ());
		std::vector<std::pair<unsigned, unsigned>> plotLocations = tempInfoCheck.getAllPixelLocations ();
	}
	std::string dialogMsg;
	dialogMsg = "Camera Parameters:\r\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n";
	dialogMsg += "Current Camera Temperature Setting:\r\n\t" + str (
		andrSttngs.temperatureSetting) + "\r\n";
	dialogMsg += "Exposure Times: ";
	for (auto& time : andrSttngs.exposureTimes){
		dialogMsg += str (time * 1000) + ", ";
	}
	dialogMsg += "\r\n";
	dialogMsg += "Image Settings:\r\n\t" + str (currentImageParameters.left) + " - " + str (currentImageParameters.right) + ", "
		+ str (currentImageParameters.bottom) + " - " + str (currentImageParameters.top) + "\r\n";
	dialogMsg += "\r\n";
	dialogMsg += "Kintetic Cycle Time:\r\n\t" + str (andrSttngs.kineticCycleTime) + "\r\n";
	dialogMsg += "Pictures per Repetition:\r\n\t" + str (andrSttngs.picsPerRepetition) + "\r\n";
	dialogMsg += "Repetitions per Variation:\r\n\t" + str (andrSttngs.totalPicsInVariation ()) + "\r\n";
	dialogMsg += "Variations per Experiment:\r\n\t" + str (andrSttngs.totalVariations) + "\r\n";
	dialogMsg += "Total Pictures per Experiment:\r\n\t" + str (andrSttngs.totalPicsInExperiment ()) + "\r\n";

	dialogMsg += "Real-Time Atom Detection Thresholds:\r\n\t";
	unsigned count = 0;
	for (auto& picThresholds : andorSettingsCtrl.getSettings ().thresholds){
		dialogMsg += "Pic " + str (count) + " thresholds: ";
		for (auto thresh : picThresholds){
			dialogMsg += str (thresh) + ", ";
		}
		dialogMsg += "\r\n";
		count++;
	}

	dialogMsg += "\r\nReal-Time Plots:\r\n";
	for (unsigned plotInc = 0; plotInc < plots.size (); plotInc++){
		dialogMsg += "\t" + plots[plotInc] + "\r\n";
	}

	return dialogMsg;
}

void QtAndorWindow::fillMasterThreadInput (ExperimentThreadInput* input){
	currentPictureNum = 0;
	// starting a not-calibration, so reset this.
	justCalibrated = false;
	//input->atomQueueForRearrangement = &rearrangerAtomQueue;
	input->rearrangerLock = &rearrangerLock;
	input->andorsImageTimes = &imageTimes;
	input->grabTimes = &imageGrabTimes;
	input->analysisGrid = analysisHandler.getAtomGrid (0);
	input->conditionVariableForRerng = &rearrangerConditionVariable;
}

void QtAndorWindow::setTimerText (std::string timerText){
	timer.setTimerDisplay (timerText);
}

void QtAndorWindow::setDataType (std::string dataType){
	stats.updateType (dataType);
}

void QtAndorWindow::redrawPictures (bool andGrid){
	try	{
		if (andGrid){
			pics.drawGrids ();
		}
	}
	catch (ChimeraError& err){
		reportErr (err.qtrace ());
	}
	// currently don't attempt to redraw previous picture data.
}

std::atomic<bool>* QtAndorWindow::getSkipNextAtomic (){
	return &skipNext;
}

void QtAndorWindow::stopPlotter (){
	plotThreadAborting = true;
}

// this is typically a little redundant to call, but can use to make sure things are set to off.
void QtAndorWindow::assertOff (){
	andorSettingsCtrl.cameraIsOn (false);
	plotThreadActive = false;
	atomCrunchThreadActive = false;
}

void QtAndorWindow::readImageParameters (){
	selectedPixel = { 0,0 };
	try	{
		redrawPictures (false);
		imageParameters parameters = andorSettingsCtrl.getSettings ().andor.imageSettings;
		pics.setParameters (parameters);
	}
	catch (ChimeraError& exception){
		reportErr (exception.qtrace () + "\r\n");
	}
	pics.drawGrids ();
}

void QtAndorWindow::fillExpDeviceList (DeviceList& list){
	list.list.push_back (andor);
}

piezoChan<double> QtAndorWindow::getAlignmentVals () {
	auto& core = imagingPiezo.getCore ();
	return { core.getCurrentXVolt (), core.getCurrentYVolt (), core.getCurrentZVolt () };
}

void QtAndorWindow::handleNormalFinish (profileSettings finishedProfile) {
	wakeRearranger ();
	cleanUpAfterExp ();
	handleBumpAnalysis (finishedProfile);
}

void QtAndorWindow::handleBumpAnalysis (profileSettings finishedProfile) {
	std::ifstream configFileRaw (finishedProfile.configFilePath ());
	// check if opened correctly.
	if (!configFileRaw.is_open ()) {
		errBox ("Opening of Configuration File for bump analysis Failed!");
		return;
	}
	ConfigStream cStream (configFileRaw);
	cStream.setCase (false);
	configFileRaw.close ();
	ProfileSystem::getVersionFromFile (cStream);
	ProfileSystem::jumpToDelimiter (cStream, "DATA_ANALYSIS");
	auto settings = analysisHandler.getAnalysisSettingsFromFile (cStream);
	// get the options from the config file, not from the current config settings. this is important especially for 
	// handling this in the calibration. 
	if (settings.autoBumpOption) {
		auto grid = andorWin->getMainAtomGrid ();
		auto dateStr = andorWin->getMostRecentDateString ();
		auto fid = andorWin->getMostRecentFid ();
		auto ppr = andorWin->getPicsPerRep ();
		try {
			auto res = pythonHandler.runCarrierAnalysis (dateStr, fid, grid, this);
			auto name =	settings.bumpParam;
			// zero is the default.
			if (name != "" && res != 0) {
				auxWin->getGlobals ().adjustVariableValue (str (name, 13, false, true), res);
			}
			reportStatus ( qstr("Successfully completed auto bump analysis and set variable \"" + name + "\" to value " 
						   + str (res) + "\n"));
		}
		catch (ChimeraError & err) {
			reportErr ("Bump Analysis Failed! " + err.qtrace ());
		}
	}
}

void QtAndorWindow::handleTransformationModeChange () {
	auto mode = andorSettingsCtrl.getTransformationMode ();
	pics.setTransformationMode (mode);
}
