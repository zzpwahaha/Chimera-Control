#include "stdafx.h"
#include "Andor.h"
#include <process.h>
#include <algorithm>
#include <numeric>
#include "atmcd32d.h"
#include "CameraWindow.h"

void AndorCamera::updatePictureNumber( int newNumber )
{
	currentPictureNumber = newNumber;
}

void AndorCamera::pauseThread()
{
	// andor should not be taking images anymore at this point.
	threadInput.spuriousWakeupHandler = false;
}

void AndorCamera::onFinish()
{
	// right now this is very simple.
	cameraIsRunning = false;
}

void AndorCamera::getAcquisitionProgress( long& seriesNumber )
{
	if ( !ANDOR_SAFEMODE )
	{
		long dummyAccumulationNumber;
		andorErrorChecker( GetAcquisitionProgress( &dummyAccumulationNumber, &seriesNumber ) );
	}
	
}


void AndorCamera::getAcquisitionProgress( long& accumulationNumber, long& seriesNumber )
{
	if ( !ANDOR_SAFEMODE )
	{
		andorErrorChecker( GetAcquisitionProgress( &accumulationNumber, &seriesNumber ) );
	}
}


unsigned __stdcall AndorCamera::cameraThread( void* voidPtr )
{
	cameraThreadInput* input = (cameraThreadInput*) voidPtr;
	std::unique_lock<std::mutex> lock( input->runMutex );
	int safeModeCount = 0;
	long pictureNumber;

	while ( !input->Andor->cameraThreadExitIndicator )
	{
		/* 
		 * wait until unlocked. this happens when data is started.
		 * the first argument is the lock.  The when the lock is locked, this function just sits and doesn't use cpu, 
		 * unlike a while(gGlobalCheck){} loop that waits for gGlobalCheck to be set. The second argument here is a 
		 * lambda, more or less a quick inline function that doesn't in this case have a name. This handles something
		 * called spurious wakeups, which are weird and appear to relate to some optimization things from the quick
		 * search I did. Regardless, I don't fully understand why spurious wakeups occur, but this protects against
		 * them.
		 */
		// Also, anytime this gets locked, the count should be reset.
		input->signaler.wait( lock, [input, &safeModeCount ]() { return input->spuriousWakeupHandler; } );
		if ( !ANDOR_SAFEMODE )
		{
			try
			{
				// alternative to directly using events.
				input->Andor->waitForAcquisition();
				input->Andor->getStatus();
			}
			catch ( myException& exception )
			{
				if ( exception.whatBare() == "DRV_IDLE" )
				{
					// signal the end to the main thread.
					input->comm->sendCameraFin();
				}
				else
				{
					try
					{
						input->Andor->getAcquisitionProgress( pictureNumber );
					}
					catch ( myException& exception )
					{
						input->comm->sendError( exception.what());
					}
					input->comm->sendCameraProgress( pictureNumber );
				}
			}
		}
		else
		{
			// simulate an actual wait.
			Sleep( input->Andor->runSettings.kinetiCycleTime * 1000 );
			if ( input->Andor->cameraIsRunning && safeModeCount < input->Andor->runSettings.totalPicsInExperiment)
			{
				if ( input->Andor->runSettings.cameraMode == "Kinetic Series Mode" 
					 || input->Andor->runSettings.cameraMode == "Accumulate Mode" )
				{
					safeModeCount++;
					input->comm->sendCameraProgress( safeModeCount );
				}
				else
				{
					input->comm->sendCameraProgress( 1 );
				}
			}
			else
			{
				input->Andor->cameraIsRunning = false;
				safeModeCount = 0;
				input->comm->sendCameraFin();
				input->spuriousWakeupHandler = false;
			}
		}
	}
	return 0;
}


/// ANDOR SDK WRAPPERS
// the following functions are wrapped to throw errors if error are returned by the raw functions, as well as to only 
// excecute the raw functions if the camera is not in safemode.
void AndorCamera::waitForAcquisition()
{
	if ( !ANDOR_SAFEMODE )
	{
		andorErrorChecker( WaitForAcquisition() );
	}
}

void AndorCamera::getTemperature(int& temp)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetTemperature(&temp));
	}
}

//
void AndorCamera::getAdjustedRingExposureTimes(int size, float* timesArray)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetAdjustedRingExposureTimes(size, timesArray));
	}
}

void AndorCamera::setNumberKinetics(int number)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetNumberKinetics(number));
	}

}

// Andor Wrappers
void AndorCamera::getTemperatureRange(int& min, int& max)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetTemperatureRange(&min, &max));
	}
}

void AndorCamera::temperatureControlOn()
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(CoolerON());
	}
}


void AndorCamera::temperatureControlOff()
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(CoolerOFF());
	}
}


void AndorCamera::setTemperature(int temp)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetTemperature(temp));
	}
}


void AndorCamera::setADChannel(int channel)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetADChannel(channel));
	}
}


void AndorCamera::setHSSpeed(int type, int index)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetHSSpeed(type, index));
	}
}

// note that the function used here could be used to get actual information about the number of images, I just only use
// it to check whether there are any new images or not. Not sure if this is the smartest way to do this.
void AndorCamera::checkForNewImages()
{
	long first, last;
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetNumberNewImages(&first, &last));
	}
	// don't do anything with the info.
}


void AndorCamera::getOldestImage(long& dataArray, int size)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetOldestImage(&dataArray, size));
	}
}


void AndorCamera::setTriggerMode(int mode)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetTriggerMode(mode));
	}
}


void AndorCamera::setAcquisitionMode(int mode)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetAcquisitionMode(mode));
	}
}


void AndorCamera::setReadMode(int mode)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetReadMode(mode));
	}
}


void AndorCamera::setRingExposureTimes(int sizeOfTimesArray, float* arrayOfTimes)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetRingExposureTimes(sizeOfTimesArray, arrayOfTimes));
	}
}


void AndorCamera::setImage(int hBin, int vBin, int lBorder, int rBorder, int tBorder, int bBorder)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetImage(hBin, vBin, lBorder, rBorder, tBorder, bBorder));
	}
}


void AndorCamera::setKineticCycleTime(float cycleTime)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetKineticCycleTime(cycleTime));
	}
}


void AndorCamera::setFrameTransferMode(int mode)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetFrameTransferMode(mode));
	}
}


void AndorCamera::getAcquisitionTimes(float& exposure, float& accumulation, float& kinetic)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetAcquisitionTimings(&exposure, &accumulation, &kinetic));
	}
}

/*
*/
void AndorCamera::getStatus()
{
	int status;
	getStatus( status );
	if (ANDOR_SAFEMODE)
	{
		status = DRV_IDLE;
	}
	if (status != DRV_IDLE)
	{
		thrower( "ERROR: You tried to start the camera, but the camera was not idle! Camera was in state corresponding to " 
				 + std::to_string( status ) + "\r\n" );
	}
}

void AndorCamera::setIsRunningState( bool state )
{
	cameraIsRunning = state;
}

void AndorCamera::getStatus(int& status)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetStatus(&status));
	}
}


void AndorCamera::startAcquisition()
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(StartAcquisition());
	}
}


void AndorCamera::abortAcquisition()
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(AbortAcquisition());
	}
}


void AndorCamera::setAccumulationCycleTime(float time)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetAccumulationCycleTime(time));
	}
}


void AndorCamera::setAccumulationNumber(int number)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetNumberAccumulations(number));
	}
}


void AndorCamera::getNumberOfPreAmpGains(int& number)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetNumberPreAmpGains(&number));
	}
}


void AndorCamera::setPreAmpGain(int index)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetPreAmpGain(index));
	}
}


void AndorCamera::getPreAmpGain(int index, float& gain)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetPreAmpGain(index, &gain));
	}
}


void AndorCamera::setOutputAmplifier(int type)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetOutputAmplifier(type));
	}
}


void AndorCamera::setEmGainSettingsAdvanced(int state)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetEMAdvanced(state));
	}
}


void AndorCamera::setEmCcdGain(int gain)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetEMCCDGain(gain));
	}
}

///

bool AndorCamera::isRunning()
{
	return cameraIsRunning;
}

AndorCamera::AndorCamera()
{
	runSettings.emGainModeIsOn = false;
}

void AndorCamera::confirmAcquisitionTimings(float& kinetic, float& accumulation, std::vector<float>& exposures)
{
	// not sure if this function is necessary...
}

/*
 * Get whatever settings the camera is currently using in it's operation, assuming it's operating.
 */
AndorRunSettings AndorCamera::getSettings()
{
	return runSettings;
}

void AndorCamera::setSettings(AndorRunSettings settingsToSet)
{
	runSettings = settingsToSet;
}

void AndorCamera::setAcquisitionMode()
{
	setAcquisitionMode(runSettings.acquisitionMode);
}

/* 
	* Large function which initializes a given camera image run.
	*/
void AndorCamera::setSystem(CameraWindow* camWin)
{
	/// Set a bunch of parameters.
	// Set to 1 MHz readout rate in both cases
	setADChannel(1);
	if (runSettings.emGainModeIsOn)
	{
		setHSSpeed(0, 0);
	}
	else
	{
		setHSSpeed(1, 0);
	}
	setAcquisitionMode();
	setReadMode();
	setExposures();
	setImageParametersToCamera();
	// Set Mode-Specific Parameters
	if (runSettings.acquisitionMode == 5)
	{
		setFrameTransferMode();
	}
	else if (runSettings.acquisitionMode == 3)
	{
		setKineticCycleTime();
		setScanNumber();
		// set this to 1.
		setNumberAccumulations(true);
	}	
	else if (runSettings.acquisitionMode == 2)
	{
		setAccumulationCycleTime();
		setNumberAccumulations(false);
	}
	confirmAcquisitionTimings(runSettings.kinetiCycleTime, runSettings.accumulationTime, 
									runSettings.exposureTimes);
	setGainMode();
	setCameraTriggerMode();
	// Set trigger mode.
	// check plotting parameters
	/// TODO!
	// CAREFUL! I can only modify these guys here because I'm sure that I'm also not writing to them in the plotting thread since the plotting thread hasn't
	// started yet. If moving stuff around, be careful.
	// Initialize the thread accumulation number.
	// this->??? = 1;
	// //////////////////////////////
	getStatus();
	/// setup fits files
	std::string errMsg;
	if (runSettings.cameraMode != "Continuous Single Scans Mode")
	{
		/// TODO: also, change to HDF5
		/*
		if (eExperimentData.initializeDataFiles(errMsg))
		{
			appendText(errMsg, IDC_ERROR_EDIT);
			return -1;
		}
		*/
	}
	/// Do some plotting stuffs
	//eAlerts.setAlertThreshold();
	//ePicStats.reset();

	// the lock is released when the lock object function goes out of scope, which happens immediately after
	// the start acquisition call
	//std::lock_guard<std::mutex> lock( threadInput.runMutex );
	
	cameraIsRunning = true;
	// remove the spurious wakeup check.
	threadInput.spuriousWakeupHandler = true;
	// notify the thread that the experiment has started..
	threadInput.signaler.notify_all();
	startAcquisition();
}

// This function queries the camera for how many pictures are available, retrieves all of them, then paints them to the main window. It returns the success of
// this operation.
std::vector<std::vector<long>> AndorCamera::acquireImageData()
{
	try
	{
		checkForNewImages();
	}
	catch (myException& exception)
	{
		if (exception.whatBare() == "DRV_NO_NEW_DATA")
		{
			// just return this anyways.
			return imagesOfExperiment;
		}
		else
		{
			// pass it up.
			throw;
		}
	}
	/// ///
	// for only one image... (each image processed from the call from a separate windows message)
	int size;
	// If there is no data the acquisition must have been aborted
	// free all allocated memory
	int experimentPictureNumber;
	if (runSettings.showPicsInRealTime)
	{
		experimentPictureNumber = 0;
	}
	else
	{
		experimentPictureNumber = (((currentPictureNumber - 1) % runSettings.totalPicsInVariation) % runSettings.picsPerRepetition);
	}
	if (experimentPictureNumber == 0)
	{
		WaitForSingleObject(imagesMutex, INFINITE);
		imagesOfExperiment.clear();
		if (runSettings.showPicsInRealTime)
		{
			imagesOfExperiment.resize(1);
		}
		else
		{
			imagesOfExperiment.resize(runSettings.picsPerRepetition);
		}
		ReleaseMutex(imagesMutex);
	}

	size = runSettings.imageSettings.width * runSettings.imageSettings.height;
	std::vector<long> tempImage;
	tempImage.resize(size);
	WaitForSingleObject(imagesMutex, INFINITE);
	imagesOfExperiment[experimentPictureNumber].resize(size);
	ReleaseMutex(imagesMutex);
 	if (!ANDOR_SAFEMODE)
	{
		getOldestImage(tempImage[0], tempImage.size());
		// immediately rotate
		WaitForSingleObject(imagesMutex, INFINITE);
		for (int imageVecInc = 0; imageVecInc < imagesOfExperiment[experimentPictureNumber].size(); imageVecInc++)
		{
			imagesOfExperiment[experimentPictureNumber][imageVecInc] = tempImage[((imageVecInc 
				% runSettings.imageSettings.width) + 1) * runSettings.imageSettings.height 
				- imageVecInc / runSettings.imageSettings.width - 1];
		}
		ReleaseMutex(imagesMutex);
	}
	else
	{
		for (int imageVecInc = 0; imageVecInc < imagesOfExperiment[experimentPictureNumber].size(); imageVecInc++)
		{
			// tempImage[imageVecInc] = imageVecInc;// (imageVecInc == 5);
			
			//tempImage[0] = 1000;
			if (experimentPictureNumber == 0 && imageVecInc == 0)
			{
				tempImage[imageVecInc] = 400;
			}
			else if (experimentPictureNumber != 0 && imageVecInc == 0)
			{
				if (rand() % 2)
				{
					tempImage[imageVecInc] = 400;
				}
				else
				{
					tempImage[imageVecInc] = rand() % 30 + 95;
				}
			}
			else
			{
				tempImage[imageVecInc] = rand() % 30 + 95;
			}

		}
		WaitForSingleObject(imagesMutex, INFINITE);
		for (int imageVecInc = 0; imageVecInc < imagesOfExperiment[experimentPictureNumber].size(); imageVecInc++)
		{
			imagesOfExperiment[experimentPictureNumber][imageVecInc] = tempImage[((imageVecInc % runSettings.imageSettings.width)
				+ 1) * runSettings.imageSettings.height - imageVecInc / runSettings.imageSettings.width - 1];
		}
		ReleaseMutex(imagesMutex);

	}
	// ???
	// eDataExists = true;
	// Display data and query max data value to be displayed in status box
	BOOL bRetValue = TRUE;
	long maxValue = 1;
	long minValue = 65536;
	/*
	if (imagesOfExperiment[experimentPictureNumber].size() != 0)
	{
		// Find max value and scale data to fill rect
		for (int pixelInc = 0; pixelInc < runSettings.imageSettings.width * runSettings.imageSettings.height; pixelInc++)
		{
			if (imagesOfExperiment[experimentPictureNumber][pixelInc] > maxValue)
			{
				maxValue = imagesOfExperiment[experimentPictureNumber][pixelInc];
			}
			if (imagesOfExperiment[experimentPictureNumber][pixelInc] < minValue)
			{
				minValue = imagesOfExperiment[experimentPictureNumber][pixelInc];
			}
		}
		if (maxValue == minValue)
		{
			return this->imagesOfExperiment;
		}
		// update the picture
		if (experimentPictureNumber == this->runSettings.picsPerRepetition - 1 
			|| this->runSettings.showPicsInRealTime)
		{
			//this->drawDataWindow();
		}
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

		// write the data to the file.
		std::string errMsg;
		int experimentPictureNumber;
		if (this->runSettings.showPicsInRealTime)
		{
			experimentPictureNumber = 0;
		}
		else
		{
			experimentPictureNumber = (((this->currentPictureNumber - 1) 
				% this->runSettings.totalPicsInVariation) % runSettings.picsPerRepetition);
		}
		if (this->runSettings.cameraMode != "Continuous Single Scans Mode")
		{
			/// TODO
			/*
			if (eExperimentData.writeFits(errMsg, experimentPictureNumber, this->currentMainThreadRepetitionNumber, eImagesOfExperiment))
			{
				thrower(errMsg);
			}

		}
	}
	else
	{
		thrower("ERROR: Data range is zero\r\n");
		return this->imagesOfExperiment;
	}
	*/
	/// TODO
	/*
	if (eCooler)
	{
		// start temp timer again when acq is complete
		SetTimer(eCameraWindowHandle, ID_TEMPERATURE_TIMER, 1000, NULL);
	}
	*/
	// % 4 at the end because there are only 4 pictures available on the screen.
	int imageLocation = (((currentPictureNumber - 1) % runSettings.totalPicsInVariation) % runSettings.repetitionsPerVariation) % 4;
	return imagesOfExperiment;
}


void AndorCamera::drawDataWindow(void)
{
	if (imagesOfExperiment.size() != 0)
	{
		for (int experimentImagesInc = 0; experimentImagesInc < imagesOfExperiment.size(); experimentImagesInc++)
		{
			long maxValue = 1;
			long minValue = 65536;
			double avgValue;
			// for all pixels... find the max and min of the picture.
			for (int pixelInc = 0; pixelInc < imagesOfExperiment[experimentImagesInc].size(); pixelInc++)
			{
				try
				{
					if (imagesOfExperiment[experimentImagesInc][pixelInc] > maxValue)
					{
						maxValue = imagesOfExperiment[experimentImagesInc][pixelInc];
					}
					if (imagesOfExperiment[experimentImagesInc][pixelInc] < minValue)
					{
						minValue = imagesOfExperiment[experimentImagesInc][pixelInc];
					}
				}
				catch (std::out_of_range&)
				{
					thrower( "ERROR: caught std::out_of_range in drawDataWindow! experimentImagesInc = " + std::to_string( experimentImagesInc )
							 + ", pixelInc = " + std::to_string( pixelInc ) + ", eImagesOfExperiment.size() = " + std::to_string( imagesOfExperiment.size() )
							 + ", eImagesOfExperiment[experimentImagesInc].size() = " + std::to_string( imagesOfExperiment[experimentImagesInc].size() )
							 + ". Attempting to continue..." );
				}
			}
			avgValue = std::accumulate( imagesOfExperiment[experimentImagesInc].begin(),
										imagesOfExperiment[experimentImagesInc].end(), 0.0 )
				/ imagesOfExperiment[experimentImagesInc].size();
			HDC hDC = 0;
			float yscale;
			long modrange = 1;
			double dTemp = 1;
			int dataWidth, dataHeight;
			int palletteInc, j, iTemp;
			HANDLE hloc;
			PBITMAPINFO pbmi;
			WORD argbq[PICTURE_PALETTE_SIZE];
			BYTE *DataArray;
			// % 4 at the end because there are only 4 pictures available on the screen.
				
			int imageLocation;
			if (runSettings.showPicsInRealTime)
			{
				imageLocation = (((currentPictureNumber - 1) % runSettings.totalPicsInVariation) 
					% runSettings.picsPerRepetition) % 4;
			}
			else
			{
				imageLocation = experimentImagesInc % 4;
			}
			// Rotated
			/*
			int selectedPixelCount = imagesOfExperiment[experimentImagesInc][eCurrentlySelectedPixel.first 
																				+ eCurrentlySelectedPixel.second * tempParam.width];

			ePicStats.update(selectedPixelCount, maxValue, minValue, avgValue, imageLocation);
			*/
			/*
			hDC = GetDC(eCameraWindowHandle);
			std::array<int, 4> colors = ePictureOptionsControl.getPictureColors();
			SelectPalette(hDC, eAppPalette[colors[imageLocation]], TRUE);
			RealizePalette(hDC);
			pixelsAreaWidth = eImageDrawAreas[imageLocation].right - eImageDrawAreas[imageLocation].left + 1;
			pixelsAreaHeight = eImageDrawAreas[imageLocation].bottom - eImageDrawAreas[imageLocation].top + 1;
			*/
			/*
			if (eAutoscalePictures)
			{
				modrange = maxValue - minValue;
			}
			else
			{
				modrange = eCurrentMaximumPictureCount[imageLocation] - eCurrentMinimumPictureCount[imageLocation];
			}
			*/
			dataWidth = runSettings.imageSettings.width;
			dataHeight = runSettings.imageSettings.height;
			// imageBoxWidth must be a multiple of 4, otherwise StretchDIBits has problems apparently T.T
			yscale = (256.0f) / (float)modrange;

			for (palletteInc = 0; palletteInc < PICTURE_PALETTE_SIZE; palletteInc++)
			{
				argbq[palletteInc] = (WORD)palletteInc;
			}

			hloc = LocalAlloc(LMEM_ZEROINIT | LMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + (sizeof(WORD)*PICTURE_PALETTE_SIZE));

			pbmi = (PBITMAPINFO)LocalLock(hloc);
			pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			pbmi->bmiHeader.biPlanes = 1;
			pbmi->bmiHeader.biBitCount = 8;
			pbmi->bmiHeader.biCompression = BI_RGB;
			pbmi->bmiHeader.biClrUsed = PICTURE_PALETTE_SIZE;

			pbmi->bmiHeader.biHeight = dataHeight;
			memcpy(pbmi->bmiColors, argbq, sizeof(WORD) * PICTURE_PALETTE_SIZE);

			DataArray = (BYTE*)malloc(dataWidth * dataHeight * sizeof(BYTE));
			memset(DataArray, PICTURE_PALETTE_SIZE - 1, dataWidth * dataHeight);
			for (palletteInc = 0; palletteInc < runSettings.imageSettings.height; palletteInc++)
			{
				for (j = 0; j < runSettings.imageSettings.width; j++)
				{
					/*
					if (eAutoscalePictures)
					{
						dTemp = ceil(yscale * (eImagesOfExperiment[experimentImagesInc][j + palletteInc * tempParam.width] - minValue));
					}
					else
					{
						dTemp = ceil(yscale * (eImagesOfExperiment[experimentImagesInc][j + palletteInc * tempParam.width] - eCurrentMinimumPictureCount[imageLocation]));
					}
					*/
					if (dTemp < 0)
					{
						// raise value to zero which is the floor of values this parameter can take.
						iTemp = 0;
					}
					else if (dTemp > PICTURE_PALETTE_SIZE - 1)
					{
						// round to maximum value.
						iTemp = PICTURE_PALETTE_SIZE - 1;
					}
					else
					{
						// no rounding or flooring to min or max needed.
						iTemp = (int)dTemp;
					}
					// store the value.
					DataArray[j + palletteInc * dataWidth] = (BYTE)iTemp;
				}
			}
			SetStretchBltMode(hDC, COLORONCOLOR);
			// eCurrentAccumulationNumber starts at 1.
			BYTE *finalDataArray = NULL;
			switch (runSettings.imageSettings.width % 4)
			{
				case 0:
				{
					pbmi->bmiHeader.biWidth = dataWidth;
					/*
					StretchDIBits(hDC, eImageDrawAreas[imageLocation].left, eImageDrawAreas[imageLocation].top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth,
						dataHeight, DataArray, (BITMAPINFO FAR*)pbmi, DIB_PAL_COLORS,
						SRCCOPY);
					*/
					break;
				}
				case 2:
				{
					// make array that is twice as long.
					finalDataArray = (BYTE*)malloc(dataWidth * dataHeight * 2);
					memset(finalDataArray, 255, dataWidth * dataHeight * 2);

					for (int dataInc = 0; dataInc < dataWidth * dataHeight; dataInc++)
					{
						finalDataArray[2 * dataInc] = DataArray[dataInc];
						finalDataArray[2 * dataInc + 1] = DataArray[dataInc];
					}
					pbmi->bmiHeader.biWidth = dataWidth * 2;
					/*
					StretchDIBits(hDC, eImageDrawAreas[imageLocation].left, eImageDrawAreas[imageLocation].top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth * 2, dataHeight,
						finalDataArray, (BITMAPINFO FAR*)pbmi, DIB_PAL_COLORS, SRCCOPY);
						*/
					free(finalDataArray);
					break;
				}
				default:
				{
					// make array that is 4X as long.
					finalDataArray = (BYTE*)malloc(dataWidth * dataHeight * 4);
					memset(finalDataArray, 255, dataWidth * dataHeight * 4);
					for (int dataInc = 0; dataInc < dataWidth * dataHeight; dataInc++)
					{
						int data = DataArray[dataInc];
						finalDataArray[4 * dataInc] = data;
						finalDataArray[4 * dataInc + 1] = data;
						finalDataArray[4 * dataInc + 2] = data;
						finalDataArray[4 * dataInc + 3] = data;
					}
					pbmi->bmiHeader.biWidth = dataWidth * 4;
					/*
					StretchDIBits(hDC, eImageDrawAreas[imageLocation].left, eImageDrawAreas[imageLocation].top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth * 4, dataHeight,
						finalDataArray, (BITMAPINFO FAR*)pbmi, DIB_PAL_COLORS, SRCCOPY);
						*/
					free(finalDataArray);
					break;
				}
			}
			free(DataArray);
			/// other drawings
			/*
			RECT relevantRect = ePixelRectangles[imageLocation][eCurrentlySelectedPixel.first][tempParam.height - 1 - eCurrentlySelectedPixel.second];
			*/
			// make crosses
			/*
			std::vector<std::pair<int, int>> atomLocations = eAutoAnalysisHandler.getAtomLocations();
			for (int analysisPointInc = 0; analysisPointInc < atomLocations.size(); analysisPointInc++)
			{
				RECT crossRect = ePixelRectangles[imageLocation][atomLocations[analysisPointInc].first][tempParam.height - 1 - atomLocations[analysisPointInc].second];
				HDC hdc;
				HPEN crossPen;
				hdc = GetDC(eCameraWindowHandle);
				if (colors[imageLocation] == 0 || colors[imageLocation] == 2)
				{
					crossPen = CreatePen(0, 1, RGB(255, 0, 0));
				}
				else
				{
					crossPen = CreatePen(0, 1, RGB(0, 255, 0));
				}
				SelectObject(hdc, crossPen);
				MoveToEx(hdc, crossRect.left, crossRect.top, 0);
				LineTo(hdc, crossRect.right, crossRect.top);
				LineTo(hdc, crossRect.right, crossRect.bottom);
				LineTo(hdc, crossRect.left, crossRect.bottom);
				LineTo(hdc, crossRect.left, crossRect.top);
				SetBkMode(hdc, TRANSPARENT);
				SetTextColor(hdc, RGB(200, 200, 200));
				int atomNumber = analysisPointInc + 1;
				DrawTextEx(hdc, const_cast<char *>(std::to_string(atomNumber).c_str()), std::to_string(atomNumber).size(),
					&crossRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL);
				ReleaseDC(eCameraWindowHandle, hdc);
				DeleteObject(crossPen);
			}
			// color circle
			RECT halfRect;
			halfRect.left = relevantRect.left + 7.0 * (relevantRect.right - relevantRect.left) / 16.0;
			halfRect.right = relevantRect.left + 9.0 * (relevantRect.right - relevantRect.left) / 16.0;
			halfRect.top = relevantRect.top + 7.0 * (relevantRect.bottom - relevantRect.top) / 16.0;
			halfRect.bottom = relevantRect.top + 9.0 * (relevantRect.bottom - relevantRect.top) / 16.0;
			HGDIOBJ originalBrush = SelectObject(hDC, GetStockObject(HOLLOW_BRUSH));
			HGDIOBJ originalPen = SelectObject(hDC, GetStockObject(DC_PEN));
			if (colors[imageLocation] == 0 || colors[imageLocation] == 2)
			{
				SetDCPenColor(hDC, RGB(255, 0, 0));
				Ellipse(hDC, relevantRect.left, relevantRect.top, relevantRect.right, relevantRect.bottom);
				SelectObject(hDC, GetStockObject(DC_BRUSH));
				SetDCBrushColor(hDC, RGB(255, 0, 0));
			}
			else
			{
				SetDCPenColor(hDC, RGB(0, 255, 0));
				Ellipse(hDC, relevantRect.left, relevantRect.top, relevantRect.right, relevantRect.bottom);
				SelectObject(hDC, GetStockObject(DC_BRUSH));
				SetDCBrushColor(hDC, RGB(0, 255, 0));
			}
			Ellipse(hDC, halfRect.left, halfRect.top, halfRect.right, halfRect.bottom);
			SelectObject(hDC, originalBrush);
			SelectObject(hDC, originalPen);
			ReleaseDC(eCameraWindowHandle, hDC);
			LocalUnlock(hloc);
			LocalFree(hloc);
			*/
		}
	}
}
// The following are a set of simple functions that call the indicated andor SDK function if not in safe mode and check the error message.
void AndorCamera::setCameraTriggerMode()
{
	std::string errMsg;
	int trigType;
	if (runSettings.triggerMode == "Internal")
	{
		trigType = 0;
	}
	else if (runSettings.triggerMode == "External")
	{
		trigType = 1;
	}
	else if (runSettings.triggerMode == "Start On Trigger")
	{
		trigType = 6;
	}
	setTriggerMode(trigType);
}


void AndorCamera::setTemperature()
{
	// Get the current temperature
	if (runSettings.temperatureSetting < -60 || runSettings.temperatureSetting > 25)
	{
		int answer = MessageBox( 0, "Warning: The selected temperature is outside the normal temperature range of the camera (-60 through "
								 "25 C). Proceed anyways?", 0, MB_OKCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
	}
	// Proceedure to initiate cooling
	changeTemperatureSetting( false );
}

void AndorCamera::setReadMode()
{
	setReadMode(runSettings.readMode);
}

void AndorCamera::setExposures()
{
	if (runSettings.exposureTimes.size() > 0 && runSettings.exposureTimes.size() <= 16)
	{
		setRingExposureTimes(runSettings.exposureTimes.size(), runSettings.exposureTimes.data());
	}
	else
	{
		thrower("ERROR: Invalid size for vector of exposure times, value of " + std::to_string(runSettings.exposureTimes.size()) + ".");
	}
}

void AndorCamera::setImageParametersToCamera()
{
	setImage(runSettings.imageSettings.verticalBinning, runSettings.imageSettings.horizontalBinning, 
			 runSettings.imageSettings.topBorder, runSettings.imageSettings.bottomBorder, 
			 runSettings.imageSettings.leftBorder, runSettings.imageSettings.rightBorder);
}

void AndorCamera::setKineticCycleTime()
{
	setKineticCycleTime(runSettings.kinetiCycleTime);
}

void AndorCamera::setScanNumber()
{
	if (runSettings.totalPicsInExperiment == 0 && runSettings.totalPicsInVariation != 0)
	{
		// all is good. The eCurrentTotalVariationNumber has not been set yet.
	}
	else if (runSettings.totalPicsInVariation == 0)
	{
		thrower("ERROR: Scan Number Was Zero.\r\n");
	}
	else
	{
		setNumberKinetics(runSettings.totalPicsInExperiment);
	}
}


void AndorCamera::setFrameTransferMode()
{
	setFrameTransferMode(runSettings.frameTransferMode);
}


/*
	* exposures should be initialized to be the correct size. Nothing else matters for the inputs, they get 
	* over-written.
	* throws exception if fails
	*/
void AndorCamera::checkAcquisitionTimings(float& kinetic, float& accumulation, std::vector<float>& exposures)
{
	float tempExposure, tempAccumTime, tempKineticTime;
	float * timesArray = NULL;
	std::string errMsg;
	if (ANDOR_SAFEMODE)
	{
		// if in safemode initialize this stuff to the values to be outputted.
		if (exposures.size() > 0)
		{
			tempExposure = exposures[0];
		}
		else
		{
			tempExposure = 0;
		}
		tempAccumTime = accumulation;
		tempKineticTime = kinetic;
	}
	else
	{
		tempExposure = 0;
		tempAccumTime = 0;
		tempKineticTime = 0;
	}
	// It is necessary to get the actual times as the system will calculate the
	// nearest possible time. eg if you set exposure time to be 0, the system
	// will use the closest value (around 0.01s)
	timesArray = new float[exposures.size()];
	if (ANDOR_SAFEMODE)
	{
		getAcquisitionTimes(tempExposure, tempAccumTime, tempKineticTime);
		getAdjustedRingExposureTimes(exposures.size(), timesArray);
	}
	else 
	{
		for (int exposureInc = 0; exposureInc < exposures.size(); exposureInc++)
		{
			timesArray[exposureInc] = exposures[exposureInc];
		}
	}
	// success. Set times
	if (exposures.size() > 0)
	{
		for (int exposureInc = 0; exposureInc < exposures.size(); exposureInc++)
		{
			exposures[exposureInc] = timesArray[exposureInc];
		}
		delete[] timesArray;
	}
	accumulation = tempAccumTime;
	kinetic = tempKineticTime;
}
	
	

/*
 (
 */
void AndorCamera::setAccumulationCycleTime()
{
	setAccumulationCycleTime(runSettings.accumulationTime);
}


void AndorCamera::setNumberAccumulations(bool isKinetic)
{
	std::string errMsg;
	if (isKinetic)
	{
		// right now, kinetic series mode always has one accumulation. could add this feature later if desired.
		//setNumberAccumulations(true);
	}
	else
	{
		// ???
		//setNumberAccumulations(false);
	}
}

void AndorCamera::setGainMode()
{
	if (runSettings.emGainModeIsOn == false)
	{
		// Set Gain
		int numGain;
		getNumberOfPreAmpGains(numGain);
		setPreAmpGain(2);
		float myGain;
		getPreAmpGain(2, myGain);
		// 1 is for conventional gain mode.
		setOutputAmplifier(1);
	}
	else
	{
		// 0 is for em gain mode.
		setOutputAmplifier(0);
		setPreAmpGain(2);
		if (runSettings.emGainLevel > 300)
		{
			setEmGainSettingsAdvanced(1);
		}
		else
		{
			setEmGainSettingsAdvanced(0);
		}
		setEmCcdGain(runSettings.emGainLevel);
	}
}

///

void AndorCamera::changeTemperatureSetting(bool turnTemperatureControlOff)
{
	char aBuffer[256];
	int minimumAllowedTemp, maximumAllowedTemp;
	// the default, in case the program is in safemode.
	minimumAllowedTemp = -60;
	maximumAllowedTemp = 25;
	// clear buffer
	wsprintf(aBuffer, "");
	// check if temp is in valid range
	getTemperatureRange(minimumAllowedTemp, maximumAllowedTemp);
	if (runSettings.temperatureSetting < minimumAllowedTemp || runSettings.temperatureSetting > maximumAllowedTemp)
	{
		thrower("ERROR: Temperature is out of range\r\n");
	}
	else
	{
		// if it is in range, switch on cooler and set temp
		if (turnTemperatureControlOff == false)
		{
			temperatureControlOn();
		}
		else
		{
			temperatureControlOff();
		}
	}

	// ???
	/*
	eCooler = TRUE;
	SetTimer(eCameraWindowHandle, ID_TEMPERATURE_TIMER, 1000, NULL);
	*/
	if (turnTemperatureControlOff == false)
	{
		setTemperature(runSettings.temperatureSetting);
	}
	else
	{
		thrower("Temperature Control has been turned off.\r\n");
	}
}

/*
 *
 */
void AndorCamera::andorErrorChecker(int errorCode)
{
	std::string errorMessage = "uninitialized";
	switch (errorCode)
	{
		case 20001:
		{
			errorMessage = "DRV_ERROR_CODES";
			break;
		}
		case 20002:
		{
			errorMessage = "DRV_SUCCESS";
			break;
		}
		case 20003:
		{
			errorMessage = "DRV_VXDNOTINSTALLED";
			break;
		}
		case 20004:
		{
			errorMessage = "DRV_ERROR_SCAN";
			break;
		}
		case 20005:
		{
			errorMessage = "DRV_ERROR_CHECK_SUM";
			break;
		}
		case 20006:
		{
			errorMessage = "DRV_ERROR_FILELOAD";
			break;
		}
		case 20007:
		{
			errorMessage = "DRV_UNKNOWN_FUNCTION";
			break;
		}
		case 20008:
		{
			errorMessage = "DRV_ERROR_VXD_INIT";
			break;
		}
		case 20009:
		{
			errorMessage = "DRV_ERROR_ADDRESS";
			break;
		}
		case 20010:
		{
			errorMessage = "DRV_ERROR_PAGELOCK";
			break;
		}
		case 20011:
		{
			errorMessage = "DRV_ERROR_PAGE_UNLOCK";
			break;
		}
		case 20012:
		{
			errorMessage = "DRV_ERROR_BOARDTEST";
			break;
		}
		case 20013:
		{
			errorMessage = "DRV_ERROR_ACK";
			break;
		}
		case 20014:
		{
			errorMessage = "DRV_ERROR_UP_FIFO";
			break;
		}
		case 20015:
		{
			errorMessage = "DRV_ERROR_PATTERN";
			break;
		}
		case 20017:
		{
			errorMessage = "DRV_ACQUISITION_ERRORS";
			break;
		}
		case 20018:
		{
			errorMessage = "DRV_ACQ_BUFFER";
			break;
		}
		case 20019:
		{
			errorMessage = "DRV_ACQ_DOWNFIFO_FULL";
			break;
		}
		case 20020:
		{
			errorMessage = "DRV_PROC_UNKNOWN_INSTRUCTION";
			break;
		}
		case 20021:
		{
			errorMessage = "DRV_ILLEGAL_OP_CODE";
			break;
		}
		case 20022:
		{
			errorMessage = "DRV_KINETIC_TIME_NOT_MET";
			break;
		}
		case 20023:
		{
			errorMessage = "DRV_KINETIC_TIME_NOT_MET";
			break;
		}
		case 20024:
		{
			errorMessage = "DRV_NO_NEW_DATA";
			break;
		}
		case 20026:
		{
			errorMessage = "DRV_SPOOLERROR";
			break;
		}
		case 20033:
		{
			errorMessage = "DRV_TEMPERATURE_CODES";
			break;
		}
		case 20034:
		{
			errorMessage = "DRV_TEMPERATURE_OFF";
			break;
		}
		case 20035:
		{
			errorMessage = "DRV_TEMPERATURE_NOT_STABILIZED";
			break;
		}
		case 20036:
		{
			errorMessage = "DRV_TEMPERATURE_STABILIZED";
			break;
		}
		case 20037:
		{
			errorMessage = "DRV_TEMPERATURE_NOT_REACHED";
			break;
		}
		case 20038:
		{
			errorMessage = "DRV_TEMPERATURE_OUT_RANGE";
			break;
		}
		case 20039:
		{
			errorMessage = "DRV_TEMPERATURE_NOT_SUPPORTED";
			break;
		}
		case 20040:
		{
			errorMessage = "DRV_TEMPERATURE_DRIFT";
			break;
		}
		case 20049:
		{
			errorMessage = "DRV_GENERAL_ERRORS";
			break;
		}
		case 20050:
		{
			errorMessage = "DRV_INVALID_AUX";
			break;
		}
		case 20051:
		{
			errorMessage = "DRV_COF_NOTLOADED";
			break;
		}
		case 20052:
		{
			errorMessage = "DRV_FPGAPROG";
			break;
		}
		case 20053:
		{
			errorMessage = "DRV_FLEXERROR";
			break;
		}
		case 20054:
		{
			errorMessage = "DRV_GPIBERROR";
			break;
		}
		case 20064:
		{
			errorMessage = "DRV_DATATYPE";
			break;
		}
		case 20065:
		{
			errorMessage = "DRV_DRIVER_ERRORS";
			break;
		}
		case 20066:
		{
			errorMessage = "DRV_P1INVALID";
			break;
		}
		case 20067:
		{
			errorMessage = "DRV_P2INVALID";
			break;
		}
		case 20068:
		{
			errorMessage = "DRV_P3INVALID";
			break;
		}
		case 20069:
		{
			errorMessage = "DRV_P4INVALID";
			break;
		}
		case 20070:
		{
			errorMessage = "DRV_INIERROR";
			break;
		}
		case 20071:
		{
			errorMessage = "DRV_COFERROR";
			break;
		}
		case 20072:
		{
			errorMessage = "DRV_ACQUIRING";
			break;
		}
		case 20073:
		{
			errorMessage = "DRV_IDLE";
			break;
		}
		case 20074:
		{
			errorMessage = "DRV_TEMPCYCLE";
			break;
		}
		case 20075:
		{
			errorMessage = "DRV_NOT_INITIALIZED";
			break;
		}
		case 20076:
		{
			errorMessage = "DRV_P5INVALID";
			break;
		}
		case 20077:
		{
			errorMessage = "DRV_P6INVALID";
			break;
		}
		case 20078:
		{
			errorMessage = "DRV_INVALID_MODE";
			break;
		}
		case 20079:
		{
			errorMessage = "DRV_INVALID_FILTER";
			break;
		}
		case 20080:
		{
			errorMessage = "DRV_I2CERRORS";
			break;
		}
		case 20081:
		{
			errorMessage = "DRV_DRV_ICDEVNOTFOUND";
			break;
		}
		case 20082:
		{
			errorMessage = "DRV_I2CTIMEOUT";
			break;
		}
		case 20083:
		{
			errorMessage = "DRV_P7INVALID";
			break;
		}
		case 20089:
		{
			errorMessage = "DRV_USBERROR";
			break;
		}
		case 20090:
		{
			errorMessage = "DRV_IOCERROR";
			break;
		}
		case 20091:
		{
			errorMessage = "DRV_NOT_SUPPORTED";
			break;
		}
		case 20093:
		{
			errorMessage = "DRV_USB_INTERRUPT_ENDPOINT_ERROR";
			break;
		}
		case 20094:
		{
			errorMessage = "DRV_RANDOM_TRACK_ERROR";
			break;
		}
		case 20095:
		{
			errorMessage = "DRV_INVALID_tRIGGER_MODE";
			break;
		}
		case 20096:
		{
			errorMessage = "DRV_LOAD_FIRMWARE_ERROR";
			break;
		}
		case 20097:
		{
			errorMessage = "DRV_DIVIDE_BY_ZERO_ERROR";
			break;
		}
		case 20098:
		{
			errorMessage = "DRV_INVALID_RINGEXPOSURES";
			break;
		}
		case 20099:
		{
			errorMessage = "DRV_BINNING_ERROR";
			break;
		}
		case 20100:
		{
			errorMessage = "DRV_INVALID_AMPLIFIER";
			break;
		}
		case 20115:
		{
			errorMessage = "DRV_ERROR_MAP";
			break;
		}
		case 20116:
		{
			errorMessage = "DRV_ERROR_UNMAP";
			break;
		}
		case 20117:
		{
			errorMessage = "DRV_ERROR_MDL";
			break;
		}
		case 20118:
		{
			errorMessage = "DRV_ERROR_UNMDL";
			break;
		}
		case 20119:
		{
			errorMessage = "DRV_ERROR_BUFSIZE";
			break;
		}
		case 20121:
		{
			errorMessage = "DRV_ERROR_NOHANDLE";
			break;
		}
		case 20130:
		{
			errorMessage = "DRV_GATING_NOT_AVAILABLE";
			break;
		}
		case 20131:
		{
			errorMessage = "DRV_FPGA_VOLTAGE_ERROR";
			break;
		}
		case 20990:
		{
			errorMessage = "DRV_ERROR_NOCAMERA";
			break;
		}
		case 20991:
		{
			errorMessage = "DRV_NOT_SUPPORTED";
			break;
		}
		case 20992:
		{
			errorMessage = "DRV_NOT_AVAILABLE";
			break;
		}
		default:
		{
			errorMessage = "UNKNOWN ERROR MESSAGE RETURNED FROM CAMERA FUNCTION!";
			break;
		}
	}
	/// So no throw is considered success.
	if (errorMessage != "DRV_SUCCESS")
	{
		thrower( errorMessage );
	}
}