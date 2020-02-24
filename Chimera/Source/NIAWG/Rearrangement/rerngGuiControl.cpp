﻿// created by Mark O. Brown
#include "stdafx.h"
#include "rerngGuiControl.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include <boost/lexical_cast.hpp>

void rerngGuiControl::initialize ( int& id, POINT& loc, CWnd* parent, cToolTips& tooltips )
{
	header.sPos = { loc.x, loc.y, loc.x + 640, loc.y += 25 };
	header.Create ( "REARRANGEMENT OPTIONS", NORM_HEADER_OPTIONS, header.sPos, parent, id++ );
	header.fontType = fontTypes::HeadingFont;
	experimentIncludesRerng.sPos = { loc.x, loc.y, loc.x + 320, loc.y += 25 };
	experimentIncludesRerng.Create ( "Experiment has Rerng?", NORM_CHECK_OPTIONS,
									 experimentIncludesRerng.sPos, parent, IDC_RERNG_EXPERIMENT_BUTTON );
	flashingRateText.sPos = { loc.x, loc.y, loc.x + 200, loc.y + 25 };
	flashingRateText.Create ( "Flashing Rate (MHz)", NORM_STATIC_OPTIONS, flashingRateText.sPos, parent, id++ );
	flashingRateEdit.sPos = { loc.x + 200, loc.y, loc.x + 320, loc.y += 25 };
	flashingRateEdit.Create ( NORM_EDIT_OPTIONS, flashingRateEdit.sPos, parent, id++ );
	flashingRateEdit.SetWindowTextA ( "1" );

	moveSpeedText.sPos = { loc.x, loc.y, loc.x + 200, loc.y + 25 };
	moveSpeedText.Create ( "Move Speed (ms)", NORM_STATIC_OPTIONS, moveSpeedText.sPos, parent, id++ );
	moveSpeedEdit.sPos = { loc.x + 200, loc.y, loc.x + 320, loc.y += 25 };
	moveSpeedEdit.Create ( NORM_EDIT_OPTIONS | ES_AUTOHSCROLL, moveSpeedEdit.sPos, parent, id++ );
	moveSpeedEdit.SetWindowTextA ( "0.06" );

	loc.y -= 75;
	loc.x += 320;

	movingBiasText.sPos = { loc.x, loc.y, loc.x + 200, loc.y + 25 };
	movingBiasText.Create ( "Moving Tweezer Bias (/1)", NORM_STATIC_OPTIONS, movingBiasText.sPos, parent, id++ );
	movingBiasEdit.sPos = { loc.x + 200, loc.y, loc.x + 320, loc.y += 25 };
	movingBiasEdit.Create ( NORM_EDIT_OPTIONS, movingBiasEdit.sPos, parent, id++ );
	movingBiasEdit.SetWindowTextA ( "0.3" );

	deadTimeText.sPos = { loc.x, loc.y, loc.x + 200, loc.y + 25 };
	deadTimeText.Create ( "Dead Time (ns)", NORM_STATIC_OPTIONS, deadTimeText.sPos, parent, id++ );
	deadTimeEdit.sPos = { loc.x + 200, loc.y, loc.x + 320, loc.y += 25 };
	deadTimeEdit.Create ( NORM_EDIT_OPTIONS, deadTimeEdit.sPos, parent, id++ );
	deadTimeEdit.SetWindowTextA ( "0" );
	staticMovingRatioText.sPos = { loc.x, loc.y, loc.x + 200, loc.y + 25 };
	staticMovingRatioText.Create ( "Static/Moving Ratio", NORM_STATIC_OPTIONS, staticMovingRatioText.sPos, parent, id++ );
	staticMovingRatioEdit.sPos = { loc.x + 200, loc.y, loc.x + 320, loc.y += 25 };
	staticMovingRatioEdit.Create ( NORM_EDIT_OPTIONS, staticMovingRatioEdit.sPos, parent, id++ );
	staticMovingRatioEdit.SetWindowTextA ( "1" );
	loc.x -= 320;
	outputRearrangeEvents.sPos = { loc.x, loc.y, loc.x + 320, loc.y += 25 };
	outputRearrangeEvents.Create ( "Output Event Info", NORM_CHECK_OPTIONS, outputRearrangeEvents.sPos, parent, id++ );
	outputIndividualEvents.sPos = { loc.x, loc.y, loc.x + 320, loc.y += 25 };
	outputIndividualEvents.Create ( "Output Individual Event Info?", NORM_CHECK_OPTIONS, outputIndividualEvents.sPos,
									parent, id++ );
	loc.y -= 50;
	preprogramMoves.sPos = { loc.x + 320, loc.y, loc.x + 640, loc.y += 25 };
	preprogramMoves.Create ( "Preprogram Moves?", NORM_CHECK_OPTIONS, preprogramMoves.sPos, parent, id++ );
	useCalibration.sPos = { loc.x + 320, loc.y, loc.x + 640, loc.y += 25 };
	useCalibration.Create ( "Use Calibration?", NORM_CHECK_OPTIONS, useCalibration.sPos, parent, id++ );

	finalMoveTimeText.sPos = { loc.x, loc.y, loc.x + 320, loc.y + 25 };
	finalMoveTimeText.Create ( "Final-Move-Time (ms): ", NORM_STATIC_OPTIONS, finalMoveTimeText.sPos, parent, id++ );
	finalMoveTimeEdit.sPos = { loc.x + 320, loc.y, loc.x + 640, loc.y += 25 };
	finalMoveTimeEdit.Create ( NORM_EDIT_OPTIONS, finalMoveTimeEdit.sPos, parent, id++ );
	finalMoveTimeEdit.SetWindowTextA ( "1" );

	rerngModeCombo.sPos = { loc.x, loc.y, loc.x + 320, loc.y + 500 };
	rerngModeCombo.Create ( NORM_COMBO_OPTIONS, rerngModeCombo.sPos, parent, IDC_RERNG_MODE_COMBO );
	for ( auto m : rerngMode::allModes )
	{
		rerngModeCombo.AddString ( rerngMode::toStr ( m ).c_str() );
	}
	rerngModeCombo.SelectString ( 0, rerngMode::toStr ( rerngMode::mode::Lazy ).c_str ( ) );

 	fastMoveTime.sPos = { loc.x + 320, loc.y, loc.x + 560, loc.y + 25 };
 	fastMoveTime.Create ( "Fast-Move (us):", NORM_STATIC_OPTIONS, fastMoveTime.sPos, parent, id++ );
 	fastMoveTimeEdit.sPos = { loc.x + 560, loc.y, loc.x + 640, loc.y += 25 };
 	fastMoveTimeEdit.Create ( NORM_EDIT_OPTIONS, fastMoveTimeEdit.sPos, parent, id++ );
	fastMoveTimeEdit.SetWindowTextA ( "2" );

	updateActive ( );
}


rerngGuiOptionsForm rerngGuiControl::getParams( )
{
	rerngGuiOptionsForm tempParams;
	tempParams.active = experimentIncludesRerng.GetCheck( );
	tempParams.outputInfo = outputRearrangeEvents.GetCheck( );
	tempParams.outputIndv = outputIndividualEvents.GetCheck( );
	tempParams.preprogram = preprogramMoves.GetCheck( );
	tempParams.useCalibration = useCalibration.GetCheck( );
	CString tempTxt;
	try
	{
		flashingRateEdit.GetWindowTextA ( tempTxt );
		tempParams.flashingRate = str ( tempTxt );
		moveSpeedEdit.GetWindowTextA ( tempTxt );
		tempParams.moveSpeed = str ( tempTxt );
		movingBiasEdit.GetWindowTextA ( tempTxt );
		tempParams.moveBias = str ( tempTxt );
		deadTimeEdit.GetWindowTextA ( tempTxt );
		tempParams.deadTime = str ( tempTxt );
		staticMovingRatioEdit.GetWindowTextA ( tempTxt );
		tempParams.staticMovingRatio = str ( tempTxt );
		finalMoveTimeEdit.GetWindowTextA ( tempTxt );
		tempParams.finalMoveTime = str ( tempTxt );
		fastMoveTimeEdit.GetWindowTextA ( tempTxt );
		tempParams.fastMoveTime = str ( tempTxt );
		rerngModeCombo.GetLBText ( rerngModeCombo.GetCurSel ( ), tempTxt );
		tempParams.rMode = rerngMode::fromStr ( str ( tempTxt ) );
	}
	catch ( boost::bad_lexical_cast&)
	{
		throwNested ( "Failed to convert rearrangement parameters to correct format! check that the inputs are the "
				 "correct types please." );
	}
	return tempParams;
}


void rerngGuiControl::rearrange( int width, int height, fontMap fonts )
{
	preprogramMoves.rearrange( width, height, fonts );
	useCalibration.rearrange( width, height, fonts );
	header.rearrange(width, height, fonts);
	experimentIncludesRerng.rearrange( width, height, fonts );
	flashingRateText.rearrange( width, height, fonts );
	flashingRateEdit.rearrange( width, height, fonts );
	moveSpeedText.rearrange( width, height, fonts );
	moveSpeedEdit.rearrange( width, height, fonts );
	movingBiasText.rearrange( width, height, fonts );
	movingBiasEdit.rearrange( width, height, fonts );
	deadTimeText.rearrange( width, height, fonts );
	deadTimeEdit.rearrange( width, height, fonts );
	staticMovingRatioEdit.rearrange( width, height, fonts );
	staticMovingRatioText.rearrange( width, height, fonts );
	outputRearrangeEvents.rearrange( width, height, fonts );
	outputIndividualEvents.rearrange( width, height, fonts );
	finalMoveTimeText.rearrange( width, height, fonts );
	finalMoveTimeEdit.rearrange( width, height, fonts );
	fastMoveTime.rearrange( width, height, fonts );
	fastMoveTimeEdit.rearrange( width, height, fonts );
	rerngModeCombo.rearrange ( width, height, fonts );
}


void rerngGuiControl::updateActive ( )
{
	auto params = getParams ( );
	flashingRateEdit.EnableWindow ( 0 );
	moveSpeedEdit.EnableWindow ( 0 );
	movingBiasEdit.EnableWindow ( 0 );
	deadTimeEdit.EnableWindow ( 0 );
	staticMovingRatioEdit.EnableWindow ( 0 );
	preprogramMoves.EnableWindow ( 0 );
	useCalibration.EnableWindow ( 0 );
	outputRearrangeEvents.EnableWindow ( 0 );
	outputIndividualEvents.EnableWindow ( 0 );
	finalMoveTimeEdit.EnableWindow ( 0 );
	fastMoveTimeEdit.EnableWindow ( 0 );
	rerngModeCombo.EnableWindow ( 0 );
	if ( params.active )
	{
		outputRearrangeEvents.EnableWindow ( 1 );
		outputIndividualEvents.EnableWindow ( 1 );
		rerngModeCombo.EnableWindow ( 1 );
		switch ( params.rMode )
		{
			case rerngMode::mode::Lazy:
			{
				break;
			}
			case rerngMode::mode::Antoine:
			{
				break;
			}
			case rerngMode::mode::StandardFlashing:
			{
				flashingRateEdit.EnableWindow ( 1 );
				moveSpeedEdit.EnableWindow ( 1 );
				movingBiasEdit.EnableWindow ( 1 );
				deadTimeEdit.EnableWindow ( 1 );
				staticMovingRatioEdit.EnableWindow ( 1 );
				preprogramMoves.EnableWindow ( 1 );
				useCalibration.EnableWindow ( 1 );
				finalMoveTimeEdit.EnableWindow ( 1 );
				break;
			}
			case rerngMode::mode::Ultrafast:
			{
				movingBiasEdit.EnableWindow ( 1 );
				deadTimeEdit.EnableWindow ( 1 );
				preprogramMoves.EnableWindow ( 1 );
				useCalibration.EnableWindow ( 1 );
				fastMoveTimeEdit.EnableWindow ( 1 );
				break;
			}
		}
	}
}


void rerngGuiControl::handleOpenConfig( std::ifstream& openFile, Version ver )
{
	rerngGuiOptionsForm info;
	std::string tmpStr;
	openFile >> info.active;
	openFile >> tmpStr;
	info.flashingRate = tmpStr;
	openFile >> tmpStr;
	info.moveBias = tmpStr;
	openFile >> tmpStr;
	info.moveSpeed = tmpStr;
	if ( ver < Version ( "2.3" ) )
	{
		std::string garbage;
		openFile >> garbage;
		openFile >> garbage;
	}
	if ( ver > Version ( "2.2" ) )
	{
		openFile >> tmpStr;
		info.deadTime = tmpStr;
		openFile >> tmpStr;
		info.staticMovingRatio = tmpStr;
	}
	else
	{
		info.deadTime = str ( "0" );
		info.staticMovingRatio = str ( "1" );
	}
	if ( ver > Version ( "2.5" ) )
	{
		openFile >> info.outputInfo;
	}
	else
	{
		info.outputInfo = false;
	}
	if ( ver > Version ( "2.10" ) )
	{
		openFile >> info.outputIndv;
	}
	else
	{
		info.outputIndv = false;
	}
	if ( ver > Version ( "2.11" ) )
	{
		openFile >> info.preprogram;
		openFile >> info.useCalibration;
	}
	else
	{
		info.preprogram = false;
		info.useCalibration = false;
	}
	if ( ver > Version ( "2.12" ) )
	{
		openFile >> tmpStr;
		info.finalMoveTime = tmpStr;
	}
	else
	{
		info.finalMoveTime = str ( 1e-3 );
	}
	if ( ver > Version ( "3.1" ) && ver < Version ( "3.6" ) )
	{
		openFile >> tmpStr;
	}
	if ( ver >= Version ( "3.6" ) )
	{
		openFile >> tmpStr;
		info.rMode = rerngMode::fromStr ( tmpStr );
	}

	if ( ver > Version ( "3.1" ) )
	{
		openFile >> tmpStr;
		info.fastMoveTime = tmpStr;
	}
	else
	{
		info.fastMoveTime = str ( 1e-6 );
	}
	if ( ver < Version ( "3.6" ) && ver >= Version ( "3.4" ) )
	{
		openFile >> tmpStr;
	}
	setParams ( info );
}


void rerngGuiControl::handleNewConfig( std::ofstream& newFile )
{
	newFile << "REARRANGEMENT_INFORMATION\n";
	newFile << 0 << "\n";
	newFile << 1 << "\n";
	// move bias
	newFile << 1e-3*0.3 << "\n";
	// move speed
	newFile << 1e-6 * 60 << "\n";
	// dead time
	newFile << 75e-9 << "\n";
	newFile << "1\n";
	newFile << "0\n";
	newFile << "0\n";
	// preprogram
	newFile << "0\n";
	// use calibration
	newFile << "0\n";
	// final move time.
	newFile << "0.001\n"; 
	newFile << rerngMode::toStr ( rerngMode::mode::Lazy ) << "\n";
	// fastmovetime
	newFile << "0\n";
	newFile << "END_REARRANGEMENT_INFORMATION\n";
}


void rerngGuiControl::handleSaveConfig( std::ofstream& saveFile )
{
 	saveFile << "REARRANGEMENT_INFORMATION\n";
	// conversions happen in getParams.
	rerngGuiOptionsForm info = getParams( );
 	saveFile << info.active << "\n";
 	saveFile << info.flashingRate.expressionStr << "\n";
 	saveFile << info.moveBias.expressionStr << "\n";
	saveFile << info.moveSpeed.expressionStr << "\n";
	saveFile << info.deadTime.expressionStr << "\n";
	saveFile << info.staticMovingRatio.expressionStr << "\n";
	saveFile << info.outputInfo << "\n";
	saveFile << info.outputIndv << "\n";
	saveFile << info.preprogram << "\n";
	saveFile << info.useCalibration << "\n" << info.finalMoveTime.expressionStr << "\n";
	saveFile << rerngMode::toStr ( info.rMode ) << "\n" << info.fastMoveTime.expressionStr << "\n";
	saveFile << "END_REARRANGEMENT_INFORMATION\n";
}


void rerngGuiControl::setParams( rerngGuiOptionsForm params )
{
	experimentIncludesRerng.SetCheck( params.active );
	// convert back to MHz from Hz
	flashingRateEdit.SetWindowTextA( cstr(params.flashingRate.expressionStr) );
	movingBiasEdit.SetWindowTextA( cstr( params.moveBias.expressionStr ) );
	// convert back to ms from s
	moveSpeedEdit.SetWindowTextA( cstr( params.moveSpeed.expressionStr ) );
	outputRearrangeEvents.SetCheck( params.outputInfo );
	// convert back to ns
	deadTimeEdit.SetWindowTextA( cstr( params.deadTime.expressionStr ) );
	staticMovingRatioEdit.SetWindowTextA( cstr( params.staticMovingRatio.expressionStr ) );
	
	outputRearrangeEvents.SetCheck( params.outputInfo );
	outputIndividualEvents.SetCheck( params.outputIndv );

	useCalibration.SetCheck( params.useCalibration );
	preprogramMoves.SetCheck( params.preprogram );
	// convert back to ms
	finalMoveTimeEdit.SetWindowTextA( cstr( params.finalMoveTime.expressionStr ) );
	rerngModeCombo.SelectString ( 0, rerngMode::toStr ( params.rMode ).c_str ( ) );
	// these are displayed in us.
	fastMoveTimeEdit.SetWindowTextA( cstr( params.fastMoveTime.expressionStr ) );
}

