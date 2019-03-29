// created by Mark O. Brown
#pragma once
#include "myButton.h"
#include "Version.h"
#include "ProfileSystem.h"
#include "ScriptStream.h"
#include "VisaFlume.h"
#include "commonTypes.h"
#include "agilentStructures.h"
#include "Script.h"
#include "Expression.h"
#include "Version.h"
#include <vector>
#include <array>
#include "Windows.h"


// A class for programming agilent machines.
// in essense this includes a wrapper around agilent's implementation of the VISA protocol. This could be pretty easily
// abstracted away from the agilent class if new systems wanted to use this functionality.
class Agilent
{
	public:
		Agilent( const agilentSettings & settings );
		~Agilent( );
		void initialize( POINT& loc, cToolTips& toolTips, CWnd* master, int& id,   
						 std::string header, UINT editHeight, COLORREF color, UINT width = 480);
		void updateButtonDisplay( int chan );
		void checkSave( std::string categoryPath, RunInfo info );
		void handleChannelPress( int chan, std::string currentCategoryPath, RunInfo currentRunInfo );
		void handleModeCombo();
		void setDC( int channel, dcInfo info );
		void setExistingWaveform( int channel, preloadedArbInfo info );
		void setSquare( int channel, squareInfo info );
		void setSine( int channel, sineInfo info );
		void outputOff(int channel);
		void handleInput( std::string categoryPath, RunInfo info );
		void handleInput( int chan, std::string categoryPath, RunInfo info );
		void setDefault( int channel );
		void prepAgilentSettings(UINT channel );
		bool connected();
		bool scriptingModeIsSelected( );
		void analyzeAgilentScript( scriptedArbInfo& infoObj, std::vector<parameterType>& vars );
		void analyzeAgilentScript ( UINT chan, std::vector<parameterType>& vars );
		HBRUSH handleColorMessage(CWnd* window, CDC* cDC);
		void handleNewConfig( std::ofstream& saveFile );
		void handleSavingConfig( std::ofstream& saveFile, std::string categoryPath, RunInfo info );
		std::string getDeviceIdentity();
		void handleOpenConfig( std::ifstream& file, Version ver );
		//void convertInputToFinalSettings(UINT chan, UINT variation, std::vector<parameterType>& variables);
		void convertInputToFinalSettings( UINT chan, std::vector<parameterType>& variables = std::vector<parameterType> ( ),
										  UINT variation = -1 );
		void updateSettingsDisplay( int chan, std::string currentCategoryPath, RunInfo currentRunInfo );
		void updateSettingsDisplay( std::string currentCategoryPath, RunInfo currentRunInfo );
		deviceOutputInfo getOutputInfo();
		void rearrange(UINT width, UINT height, fontMap fonts);
		void setAgilent( UINT variation, std::vector<parameterType>& variables);
		void setAgilent();
		void handleScriptVariation( UINT variation, scriptedArbInfo& scriptInfo, UINT channel, 
			std::vector<parameterType>& variables);
		void handleNoVariations( scriptedArbInfo& scriptInfo, UINT channel );
		void setScriptOutput(UINT varNum, scriptedArbInfo scriptInfo, UINT channel );
		// making the script public greatly simplifies opening, saving, etc. files from this script.
		Script agilentScript;
		static double convertPowerToSetPoint(double power, bool conversionOption );
		std::pair<UINT, UINT> getTriggerLine( );
		
		const std::string configDelim;

	private:
		// not that important, just used to check that number of triggers in script matches number in agilent.
		const UINT triggerRow;
		const UINT triggerNumber;
		const agilentSettings initSettings;
		minMaxDoublet chan2Range;
		VisaFlume visaFlume;
		const double sampleRate;
		const std::string load;
		const std::string filterState;
		const std::string memoryLoc;
		// since currently all visaFlume communication is done to communicate with agilent machines, my visaFlume wrappers exist
		// in this class.
		bool isConnected;
		int currentChannel;
		std::string deviceInfo;
		std::vector<minMaxDoublet> ranges;		
		deviceOutputInfo settings;
		// GUI ELEMENTS
		Control<CStatic> header;
		Control<CStatic> deviceInfoDisplay;
		Control<CButton> channel1Button;
		Control<CButton> channel2Button;
		Control<CleanCheck> syncedButton;
		Control<CleanCheck> calibratedButton;
		Control<CComboBox> settingCombo;
		Control<CStatic> optionsFormat;
		Control<CleanButton> programNow;
};


