// created by Mark O. Brown

#include "stdafx.h"
#include "ExperimentThread/Communicator.h"
#include "PrimaryWindows/QtAndorWindow.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include "PrimaryWindows/QtMainWindow.h"
#include "PrimaryWindows/QtScriptWindow.h"
#include "PrimaryWindows/QtBaslerWindow.h"
#include "PrimaryWindows/QtDeformableMirrorWindow.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "ExperimentMonitoringAndStatus/ColorBox.h"
#include <ExperimentThread/ExpThreadWorker.h>
#include "LowLevel/externals.h"


// pass all the windows so that the object can (in principle) send messages to any window.
void Communicator::initialize(IChimeraWindowWidget* win ){
	mainWin = win->mainWin;
	scriptWin = win->scriptWin;
	andorWin = win->andorWin;
	auxWin = win->auxWin;
	basWin = win->basWin;
	dmWin = win->dmWin;
}

void Communicator::sendBaslerFin ( ){
	//andorWin->PostMessage ( CustomMessages::BaslerFinMessageID, 0, 0 );
}


void Communicator::sendNoMotAlert ( ){
	//mainWin->PostMessage (CustomMessages::NoMotAlertMessageID, 0, 0 );
}



void Communicator::sendNoAtomsAlert ( ){
	//mainWin->PostMessage (CustomMessages::NoAtomsAlertMessageID, 0, 0 );
}


void Communicator::sendFinish ( ExperimentType type ){
	//mainWin->PostMessage (CustomMessages::GeneralFinMsgID, int ( type ), 0 );
}


// the two camera messages go straight to the camera window.
void Communicator::sendCameraFin(){
	//andorWin->PostMessage(CustomMessages::AndorFinishMessageID, 0, 0 );
}

void Communicator::sendExperimentProcedureFinish ( ){
	//mainWin->PostMessageA (CustomMessages::experimentProcedureFinish, 0, 0 );
}

void Communicator::sendCameraCalFin( ){
	//andorWin->PostMessage(CustomMessages::AndorCalFinMessageID, 0, 0 );
}


void Communicator::sendAutoServo( ){
	//auxWin->PostMessage(CustomMessages::AutoServoMessage, 0, 0 );
}


void Communicator::sendCameraProgress(long progress){
	//andorWin->PostMessage(CustomMessages::AndorProgressMessageID, 0, (LPARAM)progress );
}

void Communicator::sendCameraCalProgress( long progress ){
	//andorWin->PostMessage(CustomMessages::AndorCalProgMessageID, 0, (LPARAM)progress );
}

void Communicator::sendLogVoltsMessage ( UINT variation ){
	//auxWin->PostMessage (CustomMessages::LogVoltsMessageID, variation );
}


void Communicator::sendRepProgress(ULONG rep){
	//mainWin->PostMessage(CustomMessages::RepProgressMessageID, 0, LPARAM(rep));
}

void Communicator::sendTimer( std::string timerMsg ){
	if ( timerMsg != "" ){
		//andorWin->setTimerText( timerMsg );
	}
}

/*
 * Don't use this function directly, it was designed to be used with the macro in the header file.
 */
void Communicator::sendErrorEx( std::string statusMsg, const char *file, int line){
	mainWin->onErrorMessage (cstr(statusMsg));
}

/*
* Don't use this function directly, it was designed to be used with the macro in the header file.
*/
void Communicator::sendFatalErrorEx( std::string statusMsg, const char *file, int line){
	//mainWin->onFatalErrorMessage (statusMsg);
}


void Communicator::sendColorBox( std::string sysDelim, std::string color, ExpThreadWorker* threadManager ){
	emit threadManager->updateBoxColor (color.c_str(), sysDelim.c_str ());
}

void Communicator::sendStatus(std::string statusMsg){
	//mainWin->onStatusTextMessage (statusMsg);
	//mainWin->setShortStatus (statusMsg);
}

void Communicator::sendDebug(std::string statusMsg){
	if (statusMsg != ""){
		//postMyString(mainWin, CustomMessages::DebugUpdateMessageID, statusMsg);
	}
}


void Communicator::expUpdate (std::string updateTxt){
	if (!expQuiet) {
		sendStatus (updateTxt);
	}
}