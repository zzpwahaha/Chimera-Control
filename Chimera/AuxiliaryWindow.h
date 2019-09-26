// created by Mark O. Brown
#pragma once
#include <unordered_map>
#include <string>

#include "constants.h"
#include "MasterManager.h" 
#include "PiezoController.h"
#include "DioSystem.h"
#include "AoSystem.h"
#include "ParameterSystem.h"
#include "Script.h"
#include "RunInfo.h"
#include "RichEditControl.h"
#include "Repetitions.h"
#include "SocketWrapper.h"
#include "RhodeSchwarz.h"
#include "GpibFlume.h"
#include "MasterConfiguration.h"
#include "Agilent.h"
#include "commonTypes.h"
#include "StatusControl.h"
#include "TektronicsControl.h"
#include "AiSystem.h"
#include "ServoManager.h"
#include "DdsSystem.h"
#include "MachineOptimizer.h"
#include "colorbox.h"
#include "MasterThreadInput.h"
#include "Version.h"


// short for which agilent. Putting the agilentNames in a struct is a trick that makes using the scope whichAg:: 
// required while allowing implicit int conversion, which is useful for these. 
struct whichAg
{
	enum agilentNames
	{
		TopBottom,
		Axial,
		Flashing,
		Microwave
	};
};


class BaslerWindow;


// The Device window houses most of the controls for seeting individual devices, other than the camera which gets its 
// own control. It also houses a couple auxiliary things like variables and the SMS texting control.
class AuxiliaryWindow : public CDialog
{
	DECLARE_DYNAMIC(AuxiliaryWindow);
	public:
		AuxiliaryWindow();
		void setMenuCheck ( UINT menuItem, UINT itemState );
		BOOL handleAccelerators( HACCEL m_haccel, LPMSG lpMsg );
		void updateOptimization ( AllExperimentInput input );
		void OnRButtonUp( UINT stuff, CPoint clickLocation );
		void OnLButtonUp( UINT stuff, CPoint clickLocation );
		BOOL OnInitDialog();
		void handleOpeningConfig(std::ifstream& configFile, Version ver );
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnCancel();
		void OnSize(UINT nType, int cx, int cy);
		void OnPaint( );
		void passCommonCommand(UINT id);
		void OnTimer( UINT_PTR id );
		std::vector<PiezoCore* > getPiezoControllers ( );
		std::vector<servoInfo> getServoinfo ( );
		// the master needs to handle tooltip stuff.
		LRESULT onLogVoltsMessage( WPARAM wp, LPARAM lp );
		cToolTips toolTips;
		void handleMasterConfigSave( std::stringstream& configStream );
		void handleMasterConfigOpen( std::stringstream& configStream, Version version );
		BOOL PreTranslateMessage(MSG* pMsg);
		/// Message Map Functions
		void programPiezo1 ( );
		void programPiezo2 ( );
		void handleTtlPush(UINT id);
		void handlTtlHoldPush();
		void ViewOrChangeTTLNames();
		void ViewOrChangeDACNames();
		void Exit();
		void passRoundToDac();
		void loadFriends( MainWindow* mainWin_, ScriptingWindow* scriptWin_, AndorWindow* camWin_, 
						  BaslerWindow* basWin_ );
		std::string getOtherSystemStatusMsg();
		std::array<std::array<std::string, 16>, 4> getTtlNames();
		std::array<AoInfo, 24> getDacInfo ( );
		void GetAnalogInSnapshot( );
		std::string getVisaDeviceStatus( );
		std::string getGpibDeviceStatus( );
		void runServos( );
		LRESULT autoServo( WPARAM w, LPARAM l );
		void loadCameraCalSettings( ExperimentThreadInput* input );

		void updateAgilent( whichAg::agilentNames name );
		void newAgilentScript( whichAg::agilentNames name );
		void openAgilentScript( whichAg::agilentNames name, CWnd* parent );
		void saveAgilentScript( whichAg::agilentNames name );
		void saveAgilentScriptAs( whichAg::agilentNames name, CWnd* parent );
		void handleAgilentEditChange( UINT id );
		void drawVariables(UINT id, NMHDR* pNMHDR, LRESULT* pResultf);
		void handleEnter();
		void fillMasterThreadInput(ExperimentThreadInput* input);
		void changeBoxColor(systemInfo<char> colors);
		void DacEditChange(UINT id);
		void SetDacs();
		
		fontMap getFonts();

		void handleAbort();
		void zeroTtls();
		void zeroDacs();

		void handleAgilentOptions( UINT id );

		void loadMotSettings(ExperimentThreadInput* input);
		void loadTempSettings ( ExperimentThreadInput* input );

		void handleTektronicsButtons(UINT id);
		void invalidateSaved ( UINT id );
		void sendErr(std::string msg);
		void sendStatus(std::string msg);

		std::vector<parameterType> getAllVariables();

		void GlobalVarDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void GlobalVarRClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ConfigVarsColumnClick(NMHDR * pNotifyStruct, LRESULT * result);
		void clearVariables();
		void addVariable(std::string name, bool constant, double value);
		void ConfigVarsDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ConfigVarsSingleClick ( NMHDR * pNotifyStruct, LRESULT * result );
		void ConfigVarsRClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ServoRClick ( NMHDR * pNotifyStruct, LRESULT * result );
		void ServoDblClick ( NMHDR * pNotifyStruct, LRESULT * result );
		void DdsRClick ( NMHDR * pNotifyStruct, LRESULT * result );
		void DdsDblClick ( NMHDR * pNotifyStruct, LRESULT * result );


		void OptParamDblClick ( NMHDR * pNotifyStruct, LRESULT * result );
		void OptParamRClick ( NMHDR * pNotifyStruct, LRESULT * result );

		UINT getTotalVariationNumber();
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		std::pair<UINT, UINT> getTtlBoardSize();
		UINT getNumberOfDacs();
		void setVariablesActiveState(bool active);
		void passTopBottomTekProgram();
		void passEoAxialTekProgram();
		Agilent& whichAgilent( UINT id );
		void handleAgilentCombo( UINT id );
		void autoOptimize ( );
		DioSystem& getTtlBoard ( );
		AoSystem& getAoSys ( );
		AiSystem& getAiSys ( );
		RohdeSchwarz& getRsg ( );
		TektronicsAfgControl& getTopBottomTek ( );
		TektronicsAfgControl& getEoAxialTek( );
		ParameterSystem& getGlobals ( );
		DdsCore& getDds ( );
		void programDds ( );
	private:
		DECLARE_MESSAGE_MAP();		

		MainWindow* mainWin;
		ScriptingWindow* scriptWin;
		AndorWindow* camWin;
		BaslerWindow* basWin;

		CMenu menu;
		std::string title;
		toolTipTextMap toolTipText;
		/// control system classes
		RohdeSchwarz RhodeSchwarzGenerator;
		std::array<Agilent, 4> agilents;

		std::vector<PlotCtrl*> aoPlots;
		std::vector<PlotCtrl*> ttlPlots;
		std::vector<std::vector<pPlotDataVec>> ttlData, dacData;
		UINT NUM_DAC_PLTS = 3;
		UINT NUM_TTL_PLTS = 4;
		
 		DioSystem ttlBoard;
 		AoSystem aoSys;
		AiSystem aiSys;
 		MasterConfiguration masterConfig{ MASTER_CONFIGURATION_FILE_ADDRESS };
		TektronicsAfgControl topBottomTek, eoAxialTek;
		ServoManager servos;
		MachineOptimizer optimizer;
		ColorBox boxes;
		ParameterSystem configParameters, globalParameters;
		DdsSystem dds;
		
		PiezoController piezo1, piezo2;

		ColorBox statusBox;
};
