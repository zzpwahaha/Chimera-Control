#pragma once

#include <vector>
#include <array>
#include "Windows.h"
#include "ProfileSystem.h"
#include "KeyHandler.h"
#include "ScriptStream.h"
#include "VisaFlume.h"
#include "commonTypes.h"
#include "KeyHandler.h"
#include "agilentStructures.h"
#include "Script.h"


// A class for programming agilent machines.
// in essense this includes a wrapper around agilent's implementation of the VISA protocol. This could be pretty easily
// abstracted away from the agilent class if new systems wanted to use this functionality.
class Agilent
{
	public:
		Agilent( bool safemode );
		void initialize( POINT& loc, cToolTips& toolTips, CWnd* master, int& id,  std::string address, 
						 std::string header, UINT editHeight, std::array<UINT, 7> ids, COLORREF color );
		void handleChannelPress( int chan, std::string currentCategoryPath, RunInfo currentRunInfo );
		void handleCombo();
		void setDC( int channel, dcInfo info );
		void setExistingWaveform( int channel, preloadedArbInfo info );
		void setSquare( int channel, squareInfo info );
		void setSine( int channel, sineInfo info );
		void outputOff(int channel);
		void handleInput( std::string categoryPath, RunInfo info );
		void handleInput( int chan, std::string categoryPath, RunInfo info );
		void setDefualt( int channel );
		void prepAgilentSettings(UINT channel);
		bool connected();
		void analyzeAgilentScript( scriptedArbInfo& infoObj );
		HBRUSH handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC);
		void handleSavingConfig(std::ofstream& saveFile);
		std::string getDeviceIdentity();
		std::string getName();
		void readConfigurationFile( std::ifstream& file );
		//void setScript( int varNum, key variableKey, scriptedArbInfo& scriptInfo );
		void selectIntensityProfile( UINT channel, int varNum );
		void convertInputToFinalSettings( key variableKey, UINT variation );
		void convertInputToFinalSettings();
		void updateEdit( int chan, std::string currentCategoryPath, RunInfo currentRunInfo );
		void updateEdit( std::string currentCategoryPath, RunInfo currentRunInfo );
		deviceOutputInfo getOutputInfo();
		void rearrange(UINT width, UINT height, fontMap fonts);
		void setAgilent( key varKey, UINT variation );
		void setAgilent();
		void handleScriptVariation( key varKey, UINT variation, scriptedArbInfo& scriptInfo, UINT channel );
		void handleNoVariations( scriptedArbInfo& scriptInfo, UINT channel );
		void setScriptOutput(UINT varNum, scriptedArbInfo scriptInfo, UINT channel );
		// making the script public greatly simplifies opening, saving, etc. files from this script.
		Script agilentScript;

	private:
		std::string name;
		minMaxDoublet chan2Range;
		VisaFlume visaFlume;
		// since currently all visaFlume communication is done to communicate with agilent machines, my visaFlume wrappers exist
		// in this class.
		bool varies;
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
		Control<CButton> syncedButton;
		Control<CComboBox> settingCombo;
		Control<CStatic> optionsFormat;
		Control<CButton> programNow;
};