﻿#include "stdafx.h"
#include "ServoManager.h"
#include "TextPromptDialog.h"
#include "boost/lexical_cast.hpp"

void ServoManager::initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id,
							   AiSystem* ai_in, AoSystem* ao_in, DioSystem* ttls_in, ParameterSystem* globals_in )
{
	servosHeader.sPos = {pos.x, pos.y, pos.x + 480, pos.y += 20};
	servosHeader.Create( "SERVOS", NORM_HEADER_OPTIONS, servosHeader.sPos, parent, id++ );
 	servoButton.sPos = { pos.x, pos.y, pos.x + 300, pos.y + 20 };
	servoButton.Create( "Servo-Once", NORM_PUSH_OPTIONS, servoButton.sPos, parent, IDC_SERVO_CAL );
	servoButton.setToolTip ( "For the servo to calibrate.", toolTips, parent );

	autoServoButton.sPos = { pos.x + 300, pos.y, pos.x + 480, pos.y += 20 };
	autoServoButton.Create( "Auto-Servo", NORM_CHECK_OPTIONS, autoServoButton.sPos, parent, id++ );
	autoServoButton.setToolTip ( "Automatically calibrate all servos after F1.", toolTips, parent );

	toleranceLabel.sPos = { pos.x, pos.y, pos.x + 240, pos.y + 20 };
	toleranceLabel.Create("Tolerance (V):", NORM_STATIC_OPTIONS, toleranceLabel.sPos, parent, id++ );
	toleranceLabel.setToolTip ( "The servo will judge that it's finished when it's within the tolerance of the set "
								"point.", toolTips, parent );
	toleranceEdit.sPos = { pos.x + 240, pos.y, pos.x + 480, pos.y += 20 };
	toleranceEdit.Create( NORM_EDIT_OPTIONS, toleranceEdit.sPos, parent, id++ );
	toleranceEdit.SetWindowTextA( "0.05" );
	toleranceEdit.setToolTip ( "The servo will judge that it's finished when it's within the tolerance of the set "
							   "point.", toolTips, parent );

	std::vector<LONG> positions = { 0, 110, 170, 270, 320, 370, 480 };
	servoList.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 100 };
	servoList.Create ( NORM_LISTVIEW_OPTIONS, servoList.sPos, parent, IDC_SERVO_LISTVIEW );
	servoList.InsertColumn ( 0, "Name",100 );
	servoList.InsertColumn ( 1, "Active?" );
	servoList.InsertColumn ( 2, "Set-Point" );
	servoList.InsertColumn ( 3, "Control-Value" );
	servoList.InsertColumn ( 4, "Ai", 50 );
	servoList.InsertColumn ( 5, "Ao" );
	servoList.InsertColumn ( 6, "DO-Config", 100 );
	servoList.insertBlankRow ( );
	servoList.setToolTip ( "Name: The name of the servo, gets incorperated into the name of the servo_variable.\n"
						   "Active: Whether the servo will calibrate when you auto-servoing or after servo-once\n"
						   "Set-Point: The servo\'s set point, in volts.\n"
						   "Control-Value: The current control value (in volts) which was found to make the input match the set-point\n"
						   "Ai: The analog input that the servo will watch & match to the set-point\n"
						   "Ao: The analog output the servo will use to try to change the value of the analog input\n"
						   "DO-Config: The digital output configuration the sevo will set before servoing. If a ttl is "
						   "not listed here, it will be zero\'d.\n", toolTips, parent );
	ai = ai_in;
	ao = ao_in;
	ttls = ttls_in;
	globals = globals_in;
}


void ServoManager::handleSaveMasterConfig( std::stringstream& configStream )
{
	configStream << toleranceEdit.getWindowTextAsDouble ( ) << " " << autoServoButton.GetCheck ( ) << "\n" << servos.size ( );
	for ( auto& servo : servos )
	{
		handleSaveMasterConfigIndvServo ( configStream, servo );
	}
}

void ServoManager::handleOpenMasterConfig( std::stringstream& configStream, Version version )
{
	if ( version < Version( "2.1" ) )
	{
		// this was before the servo manager.
		return;
	}
	double tolerance;
	configStream >> tolerance;
	toleranceEdit.SetWindowTextA( cstr( tolerance ) );
	bool autoServo;
	configStream >> autoServo;
	autoServoButton.SetCheck( autoServo );
	LONG numServosInFile;
	configStream >> numServosInFile;
	servos.clear ( );
	for ( auto servoNum : range ( numServosInFile ) )
	{
		servos.push_back ( handleOpenMasterConfigIndvServo ( configStream, version ) );
	}
	refreshAllServos ( );
}


void ServoManager::deleteServo ( )
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos ( &cursorPos );
	servoList.ScreenToClient ( &cursorPos );
	int subitemIndicator = servoList.HitTest ( cursorPos );
	LVHITTESTINFO myItemInfo;
	memset ( &myItemInfo, 0, sizeof ( LVHITTESTINFO ) );
	myItemInfo.pt = cursorPos;
	int itemIndicator = servoList.SubItemHitTest ( &myItemInfo );
	if ( itemIndicator == -1 || itemIndicator == servos.size ( ) )
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer;
	if ( UINT ( itemIndicator ) < servos.size ( ) )
	{
		answer = promptBox ( "Delete Servo " + servos[ itemIndicator ].servoName + "?", MB_YESNO );
		if ( answer == IDYES )
		{
			servoList.DeleteItem ( itemIndicator );
			servos.erase ( servos.begin ( ) + itemIndicator );
		}

	}
	else if ( UINT ( itemIndicator ) > servos.size ( ) )
	{
		answer = promptBox ( "You appear to have found a bug with the listview control... there are too many lines "
							 "in this control. Clear this line?", MB_YESNO );
		if ( answer == IDYES )
		{
			servoList.DeleteItem ( itemIndicator );
		}
	}
}



void ServoManager::handleListViewClick ( )
{
	LVHITTESTINFO myItemInfo = { 0 };
	GetCursorPos ( &myItemInfo.pt );
	servoList.ScreenToClient ( &myItemInfo.pt );
	servoList.SubItemHitTest ( &myItemInfo );
	int subitem, itemIndicator;
	itemIndicator = myItemInfo.iItem;
	if ( itemIndicator == -1 )
	{
		return;
	}
	CString text = servoList.GetItemText ( itemIndicator, 0 );
	if ( text == "___" )
	{
		// add a variable
		servos.resize ( servos.size ( ) + 1 );
		servos.back ( ).servoName = "noname";
		// make a new "new" row.
		servoList.InsertItem ( "noname", itemIndicator, 0 );
		servoList.SetItem ( "-", itemIndicator, 1 );
	}
	auto& servo = servos[ itemIndicator ];
	subitem = myItemInfo.iSubItem;
	/// Handle different subitem clicks
	switch ( subitem )
	{
		case 0:
		{
			/// person name
			std::string newName;
			TextPromptDialog dialog ( &newName, "Please enter a name for the servo." );
			dialog.DoModal ( );
			// make name lower case
			std::transform ( newName.begin ( ), newName.end ( ), newName.begin ( ), ::tolower );
			if ( newName == "" )
			{
				// probably canceled.
				break;
			}
			for ( auto s : servos )
			{
				if ( s.servoName == newName )
				{
					thrower ( "ERROR: An servo with name " + newName + " already exists!" );
				}
			}
			servo.servoName = newName;
			servoList.SetItem ( newName, itemIndicator, subitem );
			break;
		}
		case 1:
		{
			// active, toggles.
			servo.active = !servo.active;
			servoList.SetItem ( servo.active ? "Yes" : "No", itemIndicator, 1 );			
			break;
		}
		case 2:
		{
			std::string setPointTxt;
			TextPromptDialog dialog ( &setPointTxt, "Please enter a set point for the servo." );
			dialog.DoModal ( );
			try
			{
				servo.setPoint = boost::lexical_cast<double>( setPointTxt );
			}
			catch ( boost::bad_lexical_cast& )
			{
				thrower ( "ERROR: Failed to convert text to a double!" );
			}
			servoList.SetItem ( str(servo.setPoint), itemIndicator, subitem );
			// set point
			break;
		}
		case 3:
		{
			// control value, unresponsive
			break;
		}
		case 4:
		{
			// ai
			std::string aiTxt;
			TextPromptDialog dialog ( &aiTxt, "Please enter the analog-input for the servo to look at." );
			dialog.DoModal ( );
			try
			{
				servo.aiInputChannel = boost::lexical_cast<UINT>( aiTxt );
			}
			catch ( boost::bad_lexical_cast& )
			{
				thrower ( "ERROR: Failed to convert text to an unsigned int!" );
			}
			servoList.SetItem ( str ( servo.aiInputChannel ), itemIndicator, subitem );
			break;
		}
		case 5:
		{
			// ao
			std::string aoTxt;
			TextPromptDialog dialog ( &aoTxt, "Please enter a the analog-output for the servo to use for control." );
			dialog.DoModal ( );
			try
			{
				servo.aoControlChannel = boost::lexical_cast<UINT>( aoTxt );
			}
			catch ( boost::bad_lexical_cast& )
			{
				thrower ( "ERROR: Failed to convert text to an unsigned int!" );
			}
			servoList.SetItem ( str ( servo.aoControlChannel ), itemIndicator, subitem );
			break;
		}
		case 6:
		{
			// Digital-output config
			std::string doTxt;
			TextPromptDialog dialog ( &doTxt, "Please enter the digital outputs that must be on for the calibration. "
									  "Everthing else will be off. Please separate the row from the number with a "
									  "space and each ttl with a space. E.g. \"A 1 D 0\"" );
			dialog.DoModal ( );
			std::stringstream tmpStream(doTxt);
			std::string rowTxt;
			servo.ttlConfig.clear ( );
			while ( tmpStream >> rowTxt )
			{
				std::pair<DioRows::which, UINT> ttl;
				ttl.first = DioRows::fromStr ( rowTxt );
				tmpStream >> ttl.second;
				servo.ttlConfig.push_back ( ttl );
			}
			std::string diostring;
			for ( auto ttl : servo.ttlConfig )
			{
				diostring += DioRows::toStr(ttl.first) + " " + str(ttl.second) + " ";
			}
			servoList.SetItem ( diostring, itemIndicator, subitem );
			break;
 		}
 	}
 	refreshAllServos ( );
}


servoInfo ServoManager::handleOpenMasterConfigIndvServo ( std::stringstream& configStream, Version version )
{
	servoInfo tmpInfo;
	configStream >> tmpInfo.servoName;
	if ( version > Version ( "2.3" ) )
	{
		configStream >> tmpInfo.aiInputChannel >> tmpInfo.aoControlChannel;
	}
	else
	{
		tmpInfo.aiInputChannel = tmpInfo.aoControlChannel = 0;
	}
	configStream >> tmpInfo.active >> tmpInfo.setPoint;
	if ( version > Version ( "2.3" ) )
	{
		UINT numSettings;
		configStream >> numSettings;
		tmpInfo.ttlConfig.resize ( numSettings );
		for ( auto& ttl : tmpInfo.ttlConfig )
		{
			std::string rowStr;
			configStream >> rowStr >> ttl.second;
			ttl.first = DioRows::fromStr ( rowStr );
		}
	}
	return tmpInfo;
}


void ServoManager::refreshAllServos ( )
{
	servoList.DeleteAllItems ( );
	UINT count = 0;
	for ( auto& servo : servos )
	{
		updateServoInfo ( servo, count++ );
	}
	servoList.insertBlankRow ( );
}


void ServoManager::updateServoInfo ( servoInfo& s, UINT which )
{
	servoList.InsertItem ( s.servoName, which, 0 );
	servoList.SetItem ( s.active ? "Yes" : "No", which, 1 );
	servoList.SetItem ( str ( s.setPoint ), which, 2 );
	servoList.SetItem ( str ( s.controlValue ), which, 3 );
	servoList.SetItem ( str ( s.aiInputChannel ), which, 4 );
	servoList.SetItem ( str ( s.aoControlChannel ), which, 5 );
	std::string digitalOutConfigString;
	for ( auto val : s.ttlConfig )
	{
		digitalOutConfigString += DioRows::toStr( val.first ) + " " + str ( val.second ) + " ";
	}
	servoList.SetItem ( str ( digitalOutConfigString ), which, 6 );
}


void ServoManager::handleSaveMasterConfigIndvServo ( std::stringstream& configStream, servoInfo& servo )
{
	configStream << servo.servoName << " " << servo.aiInputChannel << " " << servo.aoControlChannel << " "
		<< servo.active << " " << servo.setPoint << " " << servo.ttlConfig.size ( ) << " ";
	for ( auto& ttl : servo.ttlConfig )
	{
		configStream << DioRows::toStr(ttl.first) << " " << ttl.second << " ";
	}
	configStream << "\n";
}



void ServoManager::rearrange( UINT width, UINT height, fontMap fonts )
{
	servosHeader.rearrange( width, height, fonts );
	servoButton.rearrange( width, height, fonts );
	autoServoButton.rearrange( width, height, fonts );
	toleranceLabel.rearrange( width, height, fonts );
	toleranceEdit.rearrange( width, height, fonts );
	attemptLimitLabel.rearrange( width, height, fonts );
	attemptLimitEdit.rearrange( width, height, fonts );
	servoList.rearrange ( width, height, fonts );
}


bool ServoManager::autoServo( )
{
	return autoServoButton.GetCheck( );
}


void ServoManager::runAll( )
{
	UINT count = 0;
	for ( auto& servo : servos )
	{
		calibrate ( servo, count++ );
	}
}


void ServoManager::calibrate( servoInfo& s, UINT which )
{
	if ( !s.active )
	{
		return;
	}
	double tolerance = toleranceEdit.getWindowTextAsDouble();
	double gain = 0.05;
	double sp = s.setPoint;
	ttls->zeroBoard ( );
	for ( auto ttl : s.ttlConfig )
	{
		ttls->forceTtl ( int(ttl.first), ttl.second, 1 );
	}
	UINT attemptLimit = 10;
	UINT count = 0;
	UINT aiNum = s.aiInputChannel;
	UINT aoNum = s.aoControlChannel;
	while ( count++ < attemptLimit )
	{
		double avgVal = ai->getSingleChannelValue(aiNum, 10);
		double percentDif = (sp - avgVal) / sp;
		if ( fabs(percentDif)  < tolerance )
		{
			// found a good value.
			break;
		}
		else
		{
			// modify dac value.
			double currVal = ao->getDacValue( aoNum );
			try
			{
				ao->setSingleDac( aoNum, currVal + gain * percentDif, ttls );
			}
			catch ( Error& )
			{
				// happens if servo value gives result out of range of dacs.
				count = attemptLimit;
				break;
			}
			// there's a break built in here in order to let the laser settle.
			Sleep( 100 );
			setControlDisplay ( which, ao->getDacValue( aoNum ) );
		}
	}
	setControlDisplay ( which, ao->getDacValue( aoNum ) );
	s.servoed = count != attemptLimit;
	if ( !s.servoed )
	{
		errBox( "ERROR: servo failed to servo!" );
	}
	else
	{
		globals->adjustVariableValue( str(s.servoName + "__servo_value",13, false, true), ao->getDacValue( aoNum ) );
	}
}


void ServoManager::setControlDisplay (UINT which, double value )
{
	servoList.SetItem ( str ( value ), which, 3 );
}


