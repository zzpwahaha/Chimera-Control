#include "stdafx.h"
#include "boost/cast.hpp"
#include <algorithm>
#include <numeric>
#include <fstream>
// NI's visa file. Also gets indirectly included via #include "nifgen.h".
#include "visa.h"
#include "Agilent.h"
#include "VariableSystem.h"
#include "ScriptStream.h"
#include "ProfileSystem.h"
#include "AuxiliaryWindow.h"

Agilent::Agilent(bool safemode) : visaFlume(safemode) {}


void Agilent::rearrange(UINT width, UINT height, fontMap fonts)
{
	header.rearrange(width, height, fonts);
	deviceInfoDisplay.rearrange(width, height, fonts);
	channel1Button.rearrange(width, height, fonts);
	channel2Button.rearrange(width, height, fonts);
	syncedButton.rearrange(width, height, fonts);
	settingCombo.rearrange(width, height, fonts);
	optionsFormat.rearrange(width, height, fonts);
	agilentScript.rearrange(width, height, fonts);
	programNow.rearrange( width, height, fonts );
}


std::string Agilent::getName()
{
	return name;
}


/**
 * This function tells the agilent to put out the DC default waveform.
 */
void Agilent::setDefualt( int channel )
{
	visaFlume.open();
	// turn it to the default voltage...
	//visaFlume.write( str( "APPLy:DC DEF, DEF, " ) + AGILENT_DEFAULT_DC );
	// and leave...
	visaFlume.close();
}


void Agilent::analyzeAgilentScript( scriptedArbInfo& infoObj)
{
	// open the file
	std::ifstream scriptFile( infoObj.fileAddress );
	if (!scriptFile.is_open())
	{
		thrower( "ERROR: Scripted Agilent File \"" + infoObj.fileAddress + "\" failed to open!" );
	}
	ScriptStream stream;
	stream << scriptFile.rdbuf();
	stream.seekg( 0 );
	int currentSegmentNumber = 0;
	while (!stream.eof())
	{
		// Procedurally read lines into segment informations.
		int leaveTest = infoObj.wave.readIntoSegment( currentSegmentNumber, stream );

		if (leaveTest < 0)
		{
			thrower( "ERROR: IntensityWaveform.readIntoSegment threw an error! Error occurred in segment #"
					 + str( currentSegmentNumber ) + "." );
		}
		if (leaveTest == 1)
		{
			// read function is telling this function to stop reading the file because it's at its end.
			break;
		}
		currentSegmentNumber++;
	}
}

/*
	* This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
	*/
void Agilent::selectIntensityProfile(UINT channel, int varNum)
{
	// channel can stay 1-indexed.
	if (channel != 1 && channel != 2)
	{
		thrower( "ERROR: Bad channel value inside \"selectIntensityProfile\"" );
	}
	
	if (varies || varNum == 0)
	{
		visaFlume.open();
		// Load sequence that was previously loaded.
		visaFlume.write("MMEM:LOAD:DATA \"INT:\\seq" + str(varNum) + ".seq\"");
		visaFlume.write( "SOURCE" + str(channel) + ":FUNC ARB");
		visaFlume.write( "SOURCE" + str(channel) + ":FUNC:ARB \"INT:\\seq" + str(varNum) + ".seq\"");
		// Set output impedance...
		visaFlume.write( "OUTPUT" + str( channel ) + ":LOAD " + AGILENT_LOAD);
		visaFlume.write( "SOURCE" + str( channel ) + ":VOLT:LOW " + str(ranges[varNum].min) + " V");
		visaFlume.write( "SOURCE" + str( channel ) + ":VOLT:HIGH " + str(ranges[varNum].max) + " V");
		visaFlume.write( "OUTPUT" + str( channel ) + " ON" );
		// and leave...
		visaFlume.close();
	}
}


std::string Agilent::getDeviceIdentity()
{
	std::string msg;
	try
	{
		msg = visaFlume.identityQuery();
	}
	catch (Error& err)
	{
		msg == err.what();
	}
	if ( msg == "" )
	{
		msg = "Disconnected...\n";
	}
	return msg;
}


void Agilent::initialize( POINT& loc, cToolTips& toolTips, CWnd* parent, int& id, std::string address, 
						  std::string headerText, UINT editHeight, std::array<UINT, 7> ids, COLORREF color )
{
	visaFlume.init( address );
	name = headerText;
	try
	{
		visaFlume.open();
		int errCode = 0;
		deviceInfo = visaFlume.identityQuery();
		isConnected = true;
	}
	catch (Error&)
	{
		deviceInfo = "Disconnected";
		isConnected = false;
	}
	
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 25 };
	header.Create( cstr(headerText), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, id++ );
	header.fontType = HeadingFont;

	deviceInfoDisplay.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	deviceInfoDisplay.Create( cstr(deviceInfo), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, deviceInfoDisplay.sPos, 
							 parent, id++ );
	deviceInfoDisplay.fontType = SmallFont;

	channel1Button.sPos = { loc.x, loc.y, loc.x += 120, loc.y + 20 };
	channel1Button.Create( "Channel 1", BS_AUTORADIOBUTTON | WS_GROUP | WS_VISIBLE | WS_CHILD, channel1Button.sPos,
						  parent, ids[0] );
	channel1Button.SetCheck( true );
	
	channel2Button.sPos = { loc.x, loc.y, loc.x += 120, loc.y + 20 };
	channel2Button.Create( "Channel 2", BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD, channel2Button.sPos, parent, ids[1] );

	syncedButton.sPos = { loc.x, loc.y, loc.x += 120, loc.y + 20 };
	syncedButton.Create( "Synced?", BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD, syncedButton.sPos, parent, ids[2]);

	programNow.sPos = { loc.x, loc.y, loc.x += 120, loc.y += 20 };
	programNow.Create( "Program Now", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, programNow.sPos, parent, ids[3] );
	
	loc.x -= 480;

	settingCombo.sPos = { loc.x, loc.y, loc.x += 240, loc.y + 200 };
	settingCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,  settingCombo.sPos, 
						 parent, ids[4] );
	settingCombo.AddString( "No Control" );
	settingCombo.AddString( "Output Off" );
	settingCombo.AddString( "DC Output" );
	settingCombo.AddString( "Single Frequency Output" );
	settingCombo.AddString( "Square Output" );
	settingCombo.AddString( "Preloaded Arbitrary Waveform" );
	settingCombo.AddString( "Scripted Arbitrary Waveform" );
	settingCombo.SetCurSel( 0 );

	optionsFormat.sPos = { loc.x, loc.y, loc.x += 240, loc.y += 25 };
	optionsFormat.Create( "---", WS_CHILD | WS_VISIBLE | SS_SUNKEN, optionsFormat.sPos, parent, id++ );
	loc.x -= 480;

	agilentScript.initialize( 480, editHeight, loc, toolTips, parent, id, "Agilent", "", { ids[5], ids[6] }, color );
	
	settings.channel[0].option = -2;
	settings.channel[1].option = -2;
	currentChannel = 1;
}


HBRUSH Agilent::handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC)
{
	DWORD id = window->GetDlgCtrlID();
	if ( id == deviceInfoDisplay.GetDlgCtrlID() || id == channel1Button.GetDlgCtrlID()  
		 || id == channel2Button.GetDlgCtrlID() || id == syncedButton.GetDlgCtrlID() 
		 || id == settingCombo.GetDlgCtrlID() || id == optionsFormat.GetDlgCtrlID() )
	{
		cDC->SetBkColor(rGBs["Medium Grey"]);
		cDC->SetTextColor(rGBs["Solarized Base2"]);
		return *brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}


void Agilent::handleInput(int chan, std::string categoryPath, RunInfo info)
{
	if (chan != 1 && chan != 2)
	{
		thrower( "ERROR: Bad argument for agilent channel in Agilent::handleInput(...)!" );
	}
	// convert to zero-indexed
	chan -= 1;

	std::string textStr( agilentScript.getScriptText() );
	ScriptStream stream;
	stream << textStr;
	stream.seekg( 0 );
	
	switch (settings.channel[chan].option)
	{
		case -2:
			// no control.
			break;
		case -1:
			// output off.
			break;
		case 0:
			// DC.
			stream >> settings.channel[chan].dc.dcLevelInput;
			break;
		case 1:
			// sine wave
			stream >> settings.channel[chan].sine.frequencyInput;
			stream >> settings.channel[chan].sine.amplitudeInput;
			break;
		case 2:
			stream >> settings.channel[chan].square.frequencyInput;
			stream >> settings.channel[chan].square.amplitudeInput;
			stream >> settings.channel[chan].square.offsetInput;
			break;
		case 3:
			stream >> settings.channel[chan].preloadedArb.address;
			break;
		case 4:
			agilentScript.checkSave( categoryPath, info );
			settings.channel[chan].scriptedArb.fileAddress = agilentScript.getScriptPathAndName();
			break;
		default:
			thrower( "ERROR: unknown agilent option" );
	}
}


// overload for handling whichever channel is currently selected.
void Agilent::handleInput( std::string categoryPath, RunInfo info )
{
	// true -> 0 + 1 = 1
	// false -> 1 + 1 = 2
	handleInput( (!channel1Button.GetCheck()) + 1, categoryPath, info );
}


void Agilent::updateEdit( std::string currentCategoryPath, RunInfo currentRunInfo )
{
	updateEdit( (!channel1Button.GetCheck()) + 1, currentCategoryPath, currentRunInfo );
}


void Agilent::updateEdit(int chan, std::string currentCategoryPath, RunInfo currentRunInfo)
{
	// convert to zero-indexed.
	chan -= 1;
	std::string tempStr;
	switch ( settings.channel[chan].option )
	{
		case -2:
			tempStr = "";
			settingCombo.SetCurSel( 0 );
			break;
		case -1:
			tempStr = "";
			settingCombo.SetCurSel( 1 );
			break;
		case 0:
			// dc
			tempStr = settings.channel[chan].dc.dcLevelInput;
			settingCombo.SetCurSel( 2 );
			break;
		case 1:
			// sine
			tempStr = settings.channel[chan].sine.frequencyInput + " " + settings.channel[chan].sine.amplitudeInput;
			settingCombo.SetCurSel( 3 );
			break;
		case 2:
			// square
			tempStr = settings.channel[chan].square.frequencyInput + " " + settings.channel[chan].square.amplitudeInput
				+ " " + settings.channel[chan].square.offsetInput;
			settingCombo.SetCurSel( 4 );
			break;
		case 3:
			// preprogrammed
			tempStr = settings.channel[chan].preloadedArb.address;
			settingCombo.SetCurSel( 5 );
			break;
		case 4:
			// scripted
			settingCombo.SetCurSel( 6 );
			agilentScript.openParentScript( settings.channel[chan].scriptedArb.fileAddress, currentCategoryPath, 
											currentRunInfo );
			break;
		default:
			thrower( "ERROR: unrecognized agilent setting: " + settings.channel[chan].option);
	}
	if ( chan == 0 )
	{
		channel1Button.SetCheck( true );
		channel2Button.SetCheck( false );
	} 
	else
	{
		channel1Button.SetCheck( false );
		channel2Button.SetCheck( true );
	}
	agilentScript.setScriptText(tempStr);
}


void Agilent::handleChannelPress( int chan, std::string currentCategoryPath, RunInfo currentRunInfo )
{
	// convert from channel 1/2 to 0/1 to access the right array entr
	handleInput( currentChannel, currentCategoryPath, currentRunInfo );
	updateEdit( chan, currentCategoryPath, currentRunInfo );
	if (channel1Button.GetCheck())
	{
		currentChannel = 1;
	}
	else
	{
		currentChannel = 2;
	}
}


void Agilent::handleCombo()
{
	int selection = settingCombo.GetCurSel();
	int selectedChannel = int( !channel1Button.GetCheck() );
	switch (selection)
	{
		case 0:
			// do nothing
			optionsFormat.SetWindowTextA( "---" );
			settings.channel[selectedChannel].option = -2;
			break;
		case 1:
			// do nothing
			optionsFormat.SetWindowTextA( "---" );
			settings.channel[selectedChannel].option = -1;
			break;
		case 2:
			optionsFormat.SetWindowTextA( "[DC Level]" );
			settings.channel[selectedChannel].option = 0;
			break;
		case 3:
			optionsFormat.SetWindowTextA( "[Frequency(Hz)] [Amplitude(Vpp)]" );
			settings.channel[selectedChannel].option = 1;
			break;
		case 4:
			optionsFormat.SetWindowTextA( "[Frequency(Hz)] [Amplitude(Vpp)] [Offset(]" );
			settings.channel[selectedChannel].option = 2;
			break;
		case 5:
			optionsFormat.SetWindowTextA( "[Address]" );
			settings.channel[selectedChannel].option = 3;
			break;
		case 6:
			optionsFormat.SetWindowTextA( "Hover over \"?\"" );
			settings.channel[selectedChannel].option = 4;
			break;
	}
}

deviceOutputInfo Agilent::getOutputInfo()
{
	return settings;
}


void Agilent::convertInputToFinalSettings( key variableKey, UINT variation )
{
	// iterate between 0 and 1...
	try
	{
		for (auto chan : range( 2 ))
		{
			switch (settings.channel[chan].option)
			{
				case -2:
					// no control
					break;
				case -1:
					// no ouput
					break;
				case 0:
					// DC output
					settings.channel[chan].dc.dcLevel = reduce( settings.channel[chan].dc.dcLevelInput,
																variableKey, variation );
					break;
				case 1:
					// single frequency output
					// frequency
					settings.channel[chan].sine.frequency = reduce( settings.channel[chan].sine.frequencyInput, 
																	variableKey, variation );
					// amplitude
					settings.channel[chan].sine.amplitude = reduce( settings.channel[chan].sine.amplitudeInput, 
																	variableKey, variation );
					break;
				case 2:
					// Square Output
					// frequency
					settings.channel[chan].square.frequency = reduce( settings.channel[chan].square.frequencyInput, 
																	  variableKey, variation );
					// amplitude
					settings.channel[chan].square.amplitude = reduce( settings.channel[chan].square.amplitudeInput, 
																	  variableKey, variation );
					settings.channel[chan].square.offset = reduce( settings.channel[chan].square.offsetInput,
																	  variableKey, variation );
					break;
				case 3:
					// Preloaded Arb Output... no variations possible...
					break;
				case 4:
					// Scripted Arb Output... 
					handleScriptVariation( variableKey, variation, settings.channel[chan].scriptedArb, chan+1 );
					break;
				default:
					thrower( "Unrecognized Agilent Setting: " + str( settings.channel[chan].option ) );
			}
		}
	}
	catch (std::out_of_range&)
	{
		thrower( "ERROR: unrecognized variable!" );
	}
}


// version without variables
void Agilent::convertInputToFinalSettings()
{
	// iterate between 0 and 1...
	try
	{
		for (auto chan : range( 2 ))
		{
			switch (settings.channel[chan].option)
			{
				case -2:
					// no control
					break;
				case -1:
					// no ouput
					break;
				case 0:
					// DC output
					settings.channel[chan].dc.dcLevel = reduce( settings.channel[chan].dc.dcLevelInput);
					break;
				case 1:
					// single frequency output
					settings.channel[chan].sine.frequency = reduce( settings.channel[chan].sine.frequencyInput);
					settings.channel[chan].sine.amplitude = reduce( settings.channel[chan].sine.amplitudeInput);
					break;
				case 2:
					// Square Output
					settings.channel[chan].square.frequency = reduce( settings.channel[chan].square.frequencyInput );
					settings.channel[chan].square.amplitude = reduce( settings.channel[chan].square.amplitudeInput );
					settings.channel[chan].square.offset = reduce( settings.channel[chan].square.offsetInput );
					break;
				case 3:
					// Preloaded Arb Output... no variations possible...
					break;
				case 4:
					// Scripted Arb Output... 
					handleNoVariations( settings.channel[chan].scriptedArb, chan+1 );
					break;
				default:
					thrower( "Unrecognized Agilent Setting: " + str( settings.channel[chan].option ) );
			}
		}
	}
	catch (std::out_of_range&)
	{
		thrower( "ERROR: unrecognized variable!" );
	}
}



/*
This function outputs a string that contains all of the information that is set by the user for a given configuration. 
*/
void Agilent::handleSavingConfig(std::ofstream& saveFile)
{
	// make sure data is up to date.
	//handleInput();
	// start outputting.
	saveFile << "AGILENT\n";
	saveFile << str(settings.synced) << "\n";
	saveFile << "CHANNEL_1\n";
	saveFile << str(settings.channel[0].option) + "\n";
	saveFile << settings.channel[0].dc.dcLevelInput + "\n";
	saveFile << settings.channel[0].sine.amplitudeInput + "\n";
	saveFile << settings.channel[0].sine.frequencyInput + "\n";
	saveFile << settings.channel[0].square.amplitudeInput + "\n";
	saveFile << settings.channel[0].square.frequencyInput + "\n";
	saveFile << settings.channel[0].square.offsetInput + "\n";
	saveFile << settings.channel[0].preloadedArb.address + "\n";
	saveFile << settings.channel[0].scriptedArb.fileAddress + "\n";
	saveFile << "CHANNEL_2\n";
	saveFile << str( settings.channel[1].option ) + "\n";
	saveFile << settings.channel[1].dc.dcLevelInput + "\n";
	saveFile << settings.channel[1].sine.amplitudeInput + "\n";
	saveFile << settings.channel[1].sine.frequencyInput + "\n";	
	saveFile << settings.channel[1].square.amplitudeInput + "\n";
	saveFile << settings.channel[1].square.frequencyInput + "\n";
	saveFile << settings.channel[1].square.offsetInput + "\n";
	saveFile << settings.channel[1].preloadedArb.address + "\n";
	saveFile << settings.channel[1].scriptedArb.fileAddress + "\n";
	saveFile << "END_AGILENT\n";
}


void Agilent::readConfigurationFile( std::ifstream& file )
{
	ProfileSystem::checkDelimiterLine(file, "AGILENT");
	file >> settings.synced;
	ProfileSystem::checkDelimiterLine(file, "CHANNEL_1");
	// the extra step in all of the following is to remove the , at the end of each input.
	std::string input;
	file >> input;
	try
	{
		settings.channel[0].option = std::stoi( input );
	}
	catch (std::invalid_argument&)
	{
		thrower( "ERROR: Bad channel 1 option!" );
	}
	std::getline( file, settings.channel[0].dc.dcLevelInput);
	std::getline( file, settings.channel[0].sine.amplitudeInput );
	std::getline( file, settings.channel[0].sine.frequencyInput);
	std::getline( file, settings.channel[0].square.amplitudeInput);
	std::getline( file, settings.channel[0].square.frequencyInput);
	std::getline( file, settings.channel[0].square.offsetInput);
	std::getline( file, settings.channel[0].preloadedArb.address);
	std::getline( file, settings.channel[0].scriptedArb.fileAddress );
	ProfileSystem::checkDelimiterLine(file, "CHANNEL_2"); 
	file >> input;
	try
	{
		settings.channel[1].option = std::stoi(input);
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Bad channel 1 option!");
	}
	std::getline( file, settings.channel[1].dc.dcLevelInput);
	std::getline( file, settings.channel[1].sine.amplitudeInput);
	std::getline( file, settings.channel[1].sine.frequencyInput);
	std::getline( file, settings.channel[1].square.amplitudeInput);
	std::getline( file, settings.channel[1].square.frequencyInput);
	std::getline( file, settings.channel[1].square.offsetInput);
	std::getline( file, settings.channel[1].preloadedArb.address);
	std::getline( file, settings.channel[1].scriptedArb.fileAddress );
	ProfileSystem::checkDelimiterLine(file, "END_AGILENT");
}


void Agilent::outputOff( int channel )
{
	if (channel != 1 && channel != 2)
	{
		thrower( "ERROR: bad value for channel inside outputOff!" );
	}
	channel++;
	visaFlume.open();
	visaFlume.write( "OUTPUT" + str( channel ) + " OFF" );
	visaFlume.close();
}


bool Agilent::connected()
{
	return isConnected;
}


void Agilent::setDC( int channel, dcInfo info )
{
	if (channel != 1 && channel != 2)
	{
		thrower( "ERROR: Bad value for channel inside setDC!" );
	}
	visaFlume.open();
	visaFlume.write( "SOURce" + str( channel ) + ":APPLy:DC DEF, DEF, " + str( info.dcLevel ) + " V" );
	visaFlume.close();
}


void Agilent::setExistingWaveform( int channel, preloadedArbInfo info )
{
	if (channel != 1 && channel != 2)
	{
		thrower( "ERROR: Bad value for channel in setExistingWaveform!" );
	}
	visaFlume.open();
	visaFlume.write( "SOURCE" + str(channel) + ":DATA:VOL:CLEAR" );
	// Load sequence that was previously loaded.
	visaFlume.write( "MMEM:LOAD:DATA \"" + info.address + "\"" );
	// tell it that it's outputting something arbitrary (not sure if necessary)
	visaFlume.write( "SOURCE" + str( channel ) + ":FUNC ARB" );
	// tell it what arb it's outputting.
	visaFlume.write( "SOURCE" + str( channel ) + ":FUNC:ARB \"INT:\\" + info.address + "\"" );
	// Set output impedance...
	visaFlume.write( str( "OUTPUT" + str( channel ) + ":LOAD " ) + AGILENT_LOAD );
	// not really bursting... but this allows us to reapeat on triggers. Might be another way to do this.
	visaFlume.write( "SOURCE" + str( channel ) + ":BURST::MODE TRIGGERED" );
	visaFlume.write( "SOURCE" + str( channel ) + ":BURST::NCYCLES 1" );
	visaFlume.write( "SOURCE" + str( channel ) + ":BURST::PHASE 0" );
	// 
	visaFlume.write( "SOURCE" + str( channel ) + ":BURST::STATE ON" );
	visaFlume.write( "OUTPUT" + str( channel ) + " ON" );
	visaFlume.close();
}

// set the agilent to output a square wave.
void Agilent::setSquare( int channel, squareInfo info )
{
	if (channel != 1 && channel != 2)
	{
		thrower( "ERROR: Bad Value for Channel in setSquare!" );
	}
	visaFlume.open();
	visaFlume.write( "SOURCE" + str(channel) + ":APPLY:SQUARE " + str( info.frequency ) + " KHZ, "
					 + str( info.amplitude ) + " VPP, " + str( info.offset ) + " V" );
	visaFlume.close();
}


void Agilent::setSine( int channel, sineInfo info )
{
	if (channel != 1 && channel != 2)
	{
		thrower( "ERROR: Bad value for channel in setSine" );
	}
	visaFlume.open();
	visaFlume.write( "SOURCE" + str(channel) + ":APPLY:SINUSOID " + str( info.frequency ) + " KHZ, "
					 + str( info.amplitude ) + " VPP" );
	visaFlume.close();
}


/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// pilfered from myAgilent.
/// ......................................


// stuff that only has to be done once.
void Agilent::prepAgilentSettings(UINT channel)
{
	if (channel != 1 && channel != 2)
	{
		thrower( "ERROR: Bad value for channel in prepAgilentSettings!" );
	}
	visaFlume.open();
	// Set timout, sample rate, filter parameters, trigger settings.
	visaFlume.setAttribute( VI_ATTR_TMO_VALUE, 40000 );	
	visaFlume.write( "SOURCE" + str(channel) + ":FUNC:ARB:SRATE " + str( AGILENT_SAMPLE_RATE ) );
	visaFlume.write( "SOURCE" + str(channel) + ":FUNC:ARB:FILTER " + AGILENT_FILTER_STATE );
	visaFlume.write( "TRIGGER" + str( channel ) + ":SOURCE EXTERNAL" );
	visaFlume.write( "TRIGGER" + str( channel ) + ":SLOPE POSITIVE" );
	visaFlume.close();
}


void Agilent::handleScriptVariation( key varKey, UINT variation, scriptedArbInfo& scriptInfo, UINT channel )
{
	// Initialize stuff
	prepAgilentSettings( channel );
	// if varied
	if (scriptInfo.wave.isVaried())
	{
		//ScriptedAgilentWaveform scriptWave;
		UINT totalSegmentNumber = scriptInfo.wave.getSegmentNumber();
		visaFlume.open();
		// replace variable values where found
		scriptInfo.wave.replaceVarValues( varKey, variation );
		// Loop through all segments
		for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			// Use that information to write the data.
			try
			{
				scriptInfo.wave.writeData( segNumInc );
			}
			catch (Error& err)
			{
				thrower( "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #"
							+ str( totalSegmentNumber ) + ": " + err.what() );
			}
		}
		// loop through again and calc/normalize/write values.
		scriptInfo.wave.convertPowersToVoltages();
		scriptInfo.wave.calcMinMax();
		scriptInfo.wave.minsAndMaxes.resize( variation + 1 );
		scriptInfo.wave.minsAndMaxes[variation].second = scriptInfo.wave.getMaxVolt();
		scriptInfo.wave.minsAndMaxes[variation].first = scriptInfo.wave.getMinVolt();
		scriptInfo.wave.normalizeVoltages();
		

		for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			visaFlume.write( scriptInfo.wave.compileAndReturnDataSendString( segNumInc, variation,
																					totalSegmentNumber ) );
			// Select the segment
			visaFlume.write( "SOURCE" + str(channel) + ":FUNC:ARB seg" + str( segNumInc + totalSegmentNumber * variation ) );
			// Save the segment
			visaFlume.write( "MMEM:STORE:DATA \"INT:\\seg"
								+ str( segNumInc + totalSegmentNumber * variation ) + ".arb\"" );
			// increment for the next.
			visaFlume.write( "TRIGGER" + str( channel ) + ":SLOPE POSITIVE" );
		}
		// Now handle seqeunce creation / writing.
		scriptInfo.wave.compileSequenceString( totalSegmentNumber, variation );
		// submit the sequence
		visaFlume.write( scriptInfo.wave.returnSequenceString() );
		// Save the sequence
		visaFlume.write( "SOURCE" + str( channel ) + ":FUNC:ARB seq" + str( variation ) );
		visaFlume.write( "MMEM:STORE:DATA \"INT:\\seq" + str( variation ) + ".seq\"" );
		// clear temporary memory.
		visaFlume.write( "SOURCE" + str( channel ) + ":DATA:VOL:CLEAR" );
		// loop through # of variable values
		// replace variable values where found
		scriptInfo.wave.replaceVarValues( varKey, variation );
		// Loop through all segments
		for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			// Use that information to write the data.
			try
			{
				scriptInfo.wave.writeData( segNumInc );
			}
			catch (Error& err)
			{
				thrower( "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #"
							+ str( totalSegmentNumber ) + ": " + err.what() );
			}
		}
		// loop through again and calc/normalize/write values.
		scriptInfo.wave.convertPowersToVoltages();
		scriptInfo.wave.calcMinMax();
		scriptInfo.wave.minsAndMaxes.resize( variation + 1 );
		scriptInfo.wave.minsAndMaxes[variation].second = scriptInfo.wave.getMaxVolt();
		scriptInfo.wave.minsAndMaxes[variation].first = scriptInfo.wave.getMinVolt();
		scriptInfo.wave.normalizeVoltages();

		for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			visaFlume.write( scriptInfo.wave.compileAndReturnDataSendString( segNumInc, variation,
																					totalSegmentNumber ) );
			// Select the segment
			visaFlume.write( "SOURCE" + str( channel ) + ":FUNC:ARB seg" + str( segNumInc + totalSegmentNumber * variation ) );
			// Save the segment
			visaFlume.write( "MMEM:STORE:DATA \"INT:\\seg"
								+ str( segNumInc + totalSegmentNumber * variation ) + ".arb\"" );
			// increment for the next.
			visaFlume.write( "TRIGGER" + str( channel ) + ":SLOPE POSITIVE" );
		}
		// Now handle seqeunce creation / writing.
		scriptInfo.wave.compileSequenceString( totalSegmentNumber, variation );
		// submit the sequence
		visaFlume.write( scriptInfo.wave.returnSequenceString() );
		// Save the sequence
		visaFlume.write( "SOURCE" + str( channel ) + ":FUNC:ARB seq" + str( variation ) );
		visaFlume.write( "MMEM:STORE:DATA \"INT:\\seq" + str( variation ) + ".seq\"" );
		// clear temporary memory.
		visaFlume.write( "SOURCE" + str( channel ) + ":DATA:VOL:CLEAR" );
		visaFlume.close();

	}
	else
	{
		handleNoVariations(scriptInfo, channel);
	}
}

void Agilent::handleNoVariations(scriptedArbInfo& scriptInfo, UINT channel)
{
	// Initialize stuff
	prepAgilentSettings(channel);

	//ScriptedAgilentWaveform scriptWave;
	UINT totalSegmentNumber = scriptInfo.wave.getSegmentNumber();
	visaFlume.open();
	scriptInfo.wave.replaceVarValues();
	// else not varying. Loop through all segments once.
	for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
	{
		// Use that information to write the data.
		try
		{
			scriptInfo.wave.writeData( segNumInc );
		}
		catch (Error& err)
		{
			thrower( "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #"
					 + str( totalSegmentNumber ) + "." + err.what() );
		}
	}

	// no reassignment nessesary, no variables
	scriptInfo.wave.convertPowersToVoltages();
	scriptInfo.wave.calcMinMax();
	scriptInfo.wave.minsAndMaxes.resize( 1 );
	scriptInfo.wave.minsAndMaxes[0].second = scriptInfo.wave.getMaxVolt();
	scriptInfo.wave.minsAndMaxes[0].first = scriptInfo.wave.getMinVolt();
	scriptInfo.wave.normalizeVoltages();
	visaFlume.write( "SOURCE" + str( channel ) + ":DATA:VOL:CLEAR" );
	for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
	{
		visaFlume.write( str( "OUTPUT" + str( channel ) + ":LOAD " ) + AGILENT_LOAD );
		visaFlume.write( str( "SOURCE" + str( channel ) + ":VOLT:LOW " ) + str( scriptInfo.wave.minsAndMaxes[0].first ) + " V" );
		visaFlume.write( str( "SOURCE" + str( channel ) + ":VOLT:HIGH " ) + str( scriptInfo.wave.minsAndMaxes[0].second ) + " V" );
		visaFlume.write( scriptInfo.wave.compileAndReturnDataSendString( segNumInc, 0, totalSegmentNumber ) );
		// don't think I need this line.
		visaFlume.write( "MMEM:STORE:DATA \"INT:\\seg" + str( segNumInc ) + ".arb\"" );
	}

	// Now handle seqeunce creation / writing.
	scriptInfo.wave.compileSequenceString( totalSegmentNumber, 0 );
	// submit the sequence
	visaFlume.write( scriptInfo.wave.returnSequenceString() );
	// Save the sequence
	visaFlume.write( "SOURCE" + str( channel ) + ":FUNC:ARB seq" + str( 0 ) );
	visaFlume.write( "MMEM:STORE:DATA \"INT:\\seq" + str( 0 ) + ".seq\"" );
	// clear temporary memory.
	visaFlume.write( "SOURCE" + str( channel ) + ":DATA:VOL:CLEAR" );
	visaFlume.close();
}

/*
* This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
*/
void Agilent::setScriptOutput( UINT varNum, scriptedArbInfo scriptInfo, UINT channel )
{
	if (scriptInfo.wave.isVaried() || varNum == 0)
	{
		visaFlume.open();
		// Load sequence that was previously loaded.

		visaFlume.write( "MMEM:LOAD:DATA \"INT:\\seq" + str( varNum ) + ".seq\"" );
		visaFlume.write( "SOURCE" + str(channel) + ":FUNC ARB" );
		visaFlume.write( "SOURCE" + str( channel ) + ":FUNC:ARB \"INT:\\seq" + str( varNum ) + ".seq\"" );
		visaFlume.write( "OUTPUT" + str( channel ) + ":LOAD " + AGILENT_LOAD );
		visaFlume.write( "SOURCE" + str( channel ) + ":VOLT:OFFSET " + str( (scriptInfo.wave.minsAndMaxes[varNum].first 
																			  + scriptInfo.wave.minsAndMaxes[varNum].second) / 2 ) + " V" );
		visaFlume.write( "SOURCE" + str( channel ) + ":VOLT:LOW " + str( scriptInfo.wave.minsAndMaxes[varNum].first ) + " V" );
		visaFlume.write( "SOURCE" + str( channel ) + ":VOLT:HIGH " + str( scriptInfo.wave.minsAndMaxes[varNum].second ) + " V" );
		visaFlume.write( "OUTPUT" + str( channel ) + " ON" );
		// and leave...
		visaFlume.close();
	}
}


void Agilent::setAgilent( key varKey, UINT variation )
{
	if (!connected())
	{
		return;
	}
	convertInputToFinalSettings( varKey, variation );
	deviceOutputInfo info = getOutputInfo();
	for (auto chan : range( 2 ))
	{
		switch (info.channel[chan].option)
		{
			case -2:
				// don't do anything.
				break;
			case -1:
				outputOff( chan );
				break;
			case 0:
				setDC( chan, info.channel[chan].dc );
				break;
			case 1:
				setSine( chan, info.channel[chan].sine );
				break;
			case 2:
				setSquare( chan, info.channel[chan].square );
				break;
			case 3:
				setExistingWaveform( chan, info.channel[chan].preloadedArb );
				break;
			case 4:
				analyzeAgilentScript( info.channel[chan].scriptedArb );
				setScriptOutput( variation, info.channel[chan].scriptedArb, chan );
				break;
			default:
				thrower( "ERROR: unrecognized channel 1 setting: " + str( info.channel[chan].option ) );
		}
	}
}


void Agilent::setAgilent()
{
	if (!connected())
	{
		return;
	}
	convertInputToFinalSettings();
	deviceOutputInfo info = getOutputInfo();
	for (auto chan : range( 2 ))
	{
		switch (info.channel[chan].option)
		{
			case -2:
				// don't do anything.
				break;
			case -1:
				outputOff( chan+1 );
				break;
			case 0:
				setDC( chan+1, info.channel[chan].dc );
				break;
			case 1:
				setSine( chan+1, info.channel[chan].sine );
				break;
			case 2:
				setSquare( chan+1, info.channel[chan].square );
				break;
			case 3:
				setExistingWaveform( chan+1, info.channel[chan].preloadedArb );
				break;
			case 4:
				analyzeAgilentScript( info.channel[chan].scriptedArb );
				setScriptOutput( 0, info.channel[chan].scriptedArb, chan+1 );
				break;
			default:
				thrower( "ERROR: unrecognized channel " + str(chan+1) + " setting: " + str( info.channel[chan].option ) );
		}
	}
}