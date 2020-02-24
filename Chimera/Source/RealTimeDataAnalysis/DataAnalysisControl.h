// created by Mark O. Brown
#pragma once

#include "CustomMfcControlWrappers/Control.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "Plotting/PlotDialog.h"
#include "Plotting/PlottingInfo.h"
#include "Python/EmbeddedPythonHandler.h"
#include "ExperimentThread/Communicator.h"
#include "ConfigurationSystems/Version.h"
#include "atomGrid.h"
#include "Plotting/tinyPlotInfo.h"
#include "ParameterSystem/Expression.h"
#include "CustomMfcControlWrappers/MyListCtrl.h"
#include <deque>
#include <map>

struct realTimePlotterInput;
struct cameraPositions;

// variation data is to be organized
// variationData[datasetNumber][groupNumber][variationNumber];
typedef std::vector<std::vector<std::vector<double>>> variationData;
typedef std::vector<std::vector<double>> avgData;


class DataAnalysisControl
{
	public:
		DataAnalysisControl( );
		bool wantsThresholdAnalysis ( );
		void initialize( cameraPositions& pos, int& id, CWnd* parent, cToolTips& tooltips, int isTriggerModeSensitive );
		ULONG getPlotFreq( );
		void handleOpenConfig( std::ifstream& file, Version ver );
		void handleNewConfig( std::ofstream& file );
		void handleSaveConfig(std::ofstream& file );
		void handleDoubleClick( fontMap* fonts, UINT currentPicsPerRepetition );
		void handleRClick( );
		void rearrange( AndorRunModes::mode cameraMode, AndorTriggerMode::mode trigMode, int width, int height, fontMap fonts );
		void updateDataSetNumberEdit( int number );
		void analyze( std::string date, long runNumber, long accumulations, EmbeddedPythonHandler* pyHandler,
					  Communicator* comm );
		void onManualButtonPushed( );
		void onCornerButtonPushed( );
		void handlePictureClick( coordinate location );
		std::vector<coordinate> getAnalysisLocs( );
		atomGrid getAtomGrid( UINT which );
		std::vector<atomGrid> getGrids( );
		atomGrid getCurrentGrid( );
		UINT getSelectedGridNumber( );
		void clearAtomLocations( );
		bool getLocationSettingStatus( );
		std::vector<std::string> getActivePlotList( );
		void reloadListView( );
		bool buttonClicked( );
		void handleAtomGridCombo( );
		void reloadGridCombo( UINT num );
		void fillPlotThreadInput( realTimePlotterInput* input );
		void loadGridParams( atomGrid grid );
		static unsigned __stdcall plotterProcedure( void* voidInput );
		void saveGridParams( );
		void handleDeleteGrid( );
		void updatePlotTime ( );
		std::atomic<UINT>& getPlotTime( );
		// an "alias template". effectively a local using std::vector; declaration. makes these declarations much more
		// readable. I very rarely use things like this.
		template<class T> using vector = std::vector<T>;
		// subroutine for handling atom & count plots
		static void handlePlotAtoms( 
			PlottingInfo plotInfo, UINT repNum, vector<vector<std::pair<double, ULONG>> >& finData, 
			std::vector<std::shared_ptr<std::vector<dataPoint>>> dataContainers, 
			UINT variationNumber, vector<vector<bool>>& pscSatisfied, 
			int plotNumberCount, vector<vector<int> > atomPresent, UINT plottingFrequency, UINT groupNum, 
			UINT picsPerVariation );
		static void handlePlotHist( 
			PlottingInfo plotInfo, vector<vector<long>> countData,  
			vector<vector<std::deque<double>>>& finData, vector<vector<bool>>pscSatisfied, 
			vector<vector<std::map<int, std::pair<int, ULONG>>>>& histData,
			std::vector<std::shared_ptr<std::vector<dataPoint>>> dataArrays, UINT groupNum );
		static void determineWhichPscsSatisfied(
			PlottingInfo& info, UINT groupSize, vector<vector<int>> atomPresentData, vector<vector<bool>>& pscSatisfied );
		bool getDrawGridOption ( );
	private:
		// real time plotting
		ULONG updateFrequency;
		Control<CStatic> updateFrequencyLabel1;
		Control<CStatic> updateFrequencyLabel2;
		Control<CEdit> updateFrequencyEdit;

		Control<CStatic> header;
		Control<MyListCtrl> plotListview;
		std::vector<tinyPlotInfo> allTinyPlots;
		// other data analysis
		bool currentlySettingGridCorner;
		bool currentlySettingAnalysisLocations;
		Control<CStatic> currentDataSetNumberText;
		Control<CStatic> currentDataSetNumberDisp;
		Control<CleanPush> manualSetAnalysisLocsButton;

		Control<CComboBox> gridSelector;
		Control<CStatic> gridHeader;
		Control<CleanPush> setGridCorner;
		Control<CStatic> gridSpacingText;
		Control<CEdit> gridSpacing;
		Control<CStatic> gridWidthText;
		Control<CEdit> gridWidth;
		Control<CStatic> gridHeightText;
		Control<CEdit> gridHeight;

		Control<CleanCheck> autoThresholdAnalysisButton;
		Control<CleanCheck> displayGridBtn;


		Control<CStatic> plotTimerTxt;
		Control<CEdit> plotTimerEdit;
		std::atomic<UINT> plotTime=5000;

		std::vector<atomGrid> grids;
		UINT selectedGrid = 0;
		Control<CleanPush> deleteGrid;

		std::vector<coordinate> atomLocations;
		bool threadNeedsCounts;
};

