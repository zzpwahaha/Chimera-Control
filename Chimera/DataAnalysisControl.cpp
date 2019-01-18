
#include "stdafx.h"

#include "DataAnalysisControl.h"
#include "Control.h"
#include "AndorWindow.h"
#include "ProfileSystem.h"
#include "PlotDesignerDialog.h"
#include "realTimePlotterInput.h"
#include <numeric>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include "TextPromptDialog.h"
#include "Thrower.h"
#include "range.h"


using std::vector;


DataAnalysisControl::DataAnalysisControl( )
{
	grids.resize( 1 );
}


void DataAnalysisControl::initialize( cameraPositions& pos, int& id, CWnd* parent, cToolTips& tooltips, 
									  int isTriggerModeSensitive )
{
	header.setPositions( pos, 0, 0, 480, 25, true, false, true );
	header.Create("DATA ANALYSIS", NORM_HEADER_OPTIONS, header.seriesPos, parent, id++);
	header.fontType = fontTypes::HeadingFont;
	currentDataSetNumberText.setPositions( pos, 0, 0, 350, 50 );
	currentDataSetNumberText.Create( "Data Set #:", NORM_STATIC_OPTIONS, currentDataSetNumberText.seriesPos, parent, 
									 id++);
	currentDataSetNumberDisp.setPositions( pos, 350, 0, 130, 50, true );
	currentDataSetNumberDisp.Create("?", NORM_STATIC_OPTIONS, currentDataSetNumberDisp.seriesPos, parent, id++);
	currentDataSetNumberDisp.fontType = fontTypes::VeryLargeFont;
	// Atom Grid Settings
	gridHeader.setPositions( pos, 0, 0, 480, 25, true );
	gridHeader.Create( "Atom Grid Settings", NORM_STATIC_OPTIONS, gridHeader.seriesPos, parent, id++ );
	gridSelector.setPositions( pos, 0, 0, 50, 500 );
	gridSelector.Create( NORM_COMBO_OPTIONS, gridSelector.seriesPos, parent, IDC_ATOM_GRID_COMBO );
	gridSelector.AddString( "0" );
	gridSelector.AddString( "New" );
	gridSelector.SetCurSel( 0 );	
	deleteGrid.setPositions( pos, 50, 0, 50, 25 );
	deleteGrid.Create( "Del", NORM_PUSH_OPTIONS, deleteGrid.seriesPos, parent, IDC_DEL_GRID_BUTTON );	

	setGridCorner.setPositions( pos, 100, 0, 200, 25 );
	setGridCorner.Create( "Set Grid Top-Left Corner", NORM_CWND_OPTIONS | BS_PUSHLIKE | BS_CHECKBOX,
						  setGridCorner.seriesPos, parent, IDC_SET_GRID_CORNER );

	gridSpacingText.setPositions( pos, 300, 0, 120, 25 );
	gridSpacingText.Create("Pixel Spacing", NORM_STATIC_OPTIONS, gridSpacingText.seriesPos, parent, id++ );
	gridSpacing.setPositions( pos, 420, 0, 60, 25, true );
	gridSpacing.Create( NORM_EDIT_OPTIONS, gridSpacing.seriesPos, parent, id++ );
	gridSpacing.SetWindowTextA( "0" );
	gridWidthText.setPositions( pos, 0, 0, 120, 25 );
	gridWidthText.Create( "Width", NORM_STATIC_OPTIONS, gridWidthText.seriesPos, parent, id++ );
	gridWidth.setPositions( pos, 120, 0, 120, 25 );
	gridWidth.Create( NORM_EDIT_OPTIONS, gridWidth.seriesPos, parent, id++ );
	gridWidth.SetWindowText( "0" );
	gridHeightText.setPositions( pos, 240, 0, 120, 25 );
	gridHeightText.Create( "Height", NORM_STATIC_OPTIONS, gridHeightText.seriesPos, parent, id++ );
	gridHeight.setPositions( pos, 360, 0, 120, 25, true );
	gridHeight.Create( NORM_EDIT_OPTIONS, gridHeight.seriesPos, parent, id++ );
	gridHeight.SetWindowTextA( "0" );
	// 
	manualSetAnalysisLocsButton.setPositions( pos, 0, 0, 480, 25, true );
	manualSetAnalysisLocsButton.Create("Manually Set AutoAnalysis Points", NORM_CWND_OPTIONS | BS_PUSHLIKE | BS_CHECKBOX,
										manualSetAnalysisLocsButton.seriesPos, parent, IDC_SET_ANALYSIS_LOCATIONS );

	manualSetAnalysisLocsButton.EnableWindow( false );
	/// PLOTTING FREQUENCY CONTROLS
	updateFrequencyLabel1.setPositions( pos, 0, 0, 150, 25, false, false, true );
	updateFrequencyLabel1.Create("Update plots every (", NORM_STATIC_OPTIONS | ES_CENTER | ES_RIGHT, 
								  updateFrequencyLabel1.seriesPos, parent, id++);
	updateFrequencyEdit.setPositions( pos, 150, 0, 50, 25, false, false, true );
	updateFrequencyEdit.Create( NORM_EDIT_OPTIONS, updateFrequencyEdit.seriesPos, parent, id++);
	updateFrequency = 5;
	updateFrequencyEdit.SetWindowTextA("5");
	updateFrequencyLabel2.setPositions( pos, 200, 0, 280, 25, true, false, true );
	updateFrequencyLabel2.Create(") repetitions.", NORM_STATIC_OPTIONS | ES_CENTER | ES_LEFT,
								  updateFrequencyLabel2.seriesPos, parent, id++);

	plotTimerTxt.setPositions( pos, 0, 0, 360, 25 );
	plotTimerTxt.Create( "Plot Update Timer Length (ms):", NORM_STATIC_OPTIONS, plotTimerTxt.seriesPos, parent, id++ );
	plotTimerEdit.setPositions( pos, 360, 0, 120, 25, true );
	plotTimerEdit.Create( NORM_EDIT_OPTIONS, plotTimerEdit.seriesPos, parent, id++ );
	plotTimerEdit.SetWindowText( "5000" );

	/// Initialize the listview
	plotListview.setPositions( pos, 0, 0, 480, 150, true, false, true );
	plotListview.Create( NORM_LISTVIEW_OPTIONS, plotListview.seriesPos, parent, IDC_PLOTTING_LISTVIEW );
	RECT r;
	parent->GetClientRect( &r );
	// spacing is funny because initial window size is not full screen and the columns aren't autoscaled. This spacing
	// just works out.
	plotListview.InsertColumn ( 0, "Name", r.right / 3 );
	for ( auto txt : { "Active", "Details", "Edit", "Grid#" } )
	{
		plotListview.InsertColumn( 1, txt, r.right / 9 );
	}
	plotListview.SetBkColor( _myRGBs["Interactable-Bkgd"] );
	plotListview.SetTextBkColor( _myRGBs["Interactable-Bkgd"] );
	plotListview.SetTextColor( _myRGBs["AndorWin-Text"] );
	//
	reloadListView();
}


UINT DataAnalysisControl::getPlotTime( )
{
	CString txt;
	plotTimerEdit.GetWindowText( txt );
	std::string tmpStr( txt );
	try
	{
		return boost::lexical_cast<unsigned long>( tmpStr );
	}
	catch ( boost::bad_lexical_cast& )
	{
		throwNested ( "ERROR: plot time failed to convert to an unsigned integer!" );
	}
}


void DataAnalysisControl::handleDeleteGrid( )
{
	if ( grids.size() == 1 )
	{
		thrower ( "ERROR: You are not allowed to delete the last grid for data analysis!" );
	}
	grids.erase( grids.begin( ) + selectedGrid );
	gridSelector.ResetContent( );
	UINT count = 0;
	for ( auto grid : grids )
	{
		std::string txt( str( count++ ) );
		gridSelector.AddString( cstr( txt ) );
	}
	gridSelector.AddString( "New" );
	gridSelector.SetCurSel( 0 );
	selectedGrid = 0;
	loadGridParams( grids[0] );
}


ULONG DataAnalysisControl::getPlotFreq( )
{
	CString txt;
	updateFrequencyEdit.GetWindowText(txt);
	try
	{
		updateFrequency = boost::lexical_cast<long>( str( txt ) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		throwNested ( "ERROR: Failed to convert plotting update frequency to an integer! text was: " + str( txt ) );
	}
	return updateFrequency;
}


void DataAnalysisControl::handleOpenConfig( std::ifstream& file, Version ver )
{
	UINT numGrids;
	if ( ver > Version( "3.0" ) )
	{
		file >> numGrids;
	}
	else
	{
		numGrids = 1;
	}

	if ( numGrids <= 0 )
	{
		numGrids = 1;
	}
	grids.resize( numGrids );
	for ( auto& grid : grids )
	{
		file >> grid.topLeftCorner.row >> grid.topLeftCorner.column >> grid.width >> grid.height >> grid.pixelSpacing;
	}
	reloadGridCombo( grids.size( ) );
	gridSelector.SetCurSel( 0 );
	// load the grid parameters for that selection.
	loadGridParams( grids[0] );
	selectedGrid = 0;
	if ( ver > Version( "2.7" ) )
	{
		ProfileSystem::checkDelimiterLine( file, "BEGIN_ACTIVE_PLOTS" );
		UINT numPlots = 0;
		file >> numPlots;
		file.get( );
		vector<std::string> activePlotNames;
		vector<UINT> whichGrids;
		for ( auto pltInc : range( numPlots ) )
		{
			std::string tmp;
			std::getline( file, tmp );
			activePlotNames.push_back( tmp );
			if ( ver > Version( "3.0" ) )
			{
				UINT which;
				file >> which;
				file.get( );
				whichGrids.push_back( which );
			}
			else
			{
				whichGrids.push_back( 0 );
			}
		}
		UINT counter = 0;
		for ( auto& pltInfo : allTinyPlots )
		{
			bool found = false;
			UINT activeCounter = -1;
			for ( auto activePlt : activePlotNames )
			{
				activeCounter++;
				if ( activePlt == pltInfo.name )
				{
					pltInfo.isActive = true;
					pltInfo.whichGrid = whichGrids[activeCounter];
					plotListview.SetItem( "YES", counter, 4 );
					plotListview.SetItem( str(pltInfo.whichGrid), counter, 1);
					found = true;
					break;
				}
			}
			if ( !found )
			{
				pltInfo.isActive = false;
				plotListview.SetItem( "NO", counter, 4 );
			}
			counter++;
		}
		ProfileSystem::checkDelimiterLine( file, "END_ACTIVE_PLOTS" );
	}
}


void DataAnalysisControl::handleNewConfig( std::ofstream& file )
{
	file << "DATA_ANALYSIS\n";
	file << 1 << "\n";
	file << 0 << " " << 0 << "\n";
	file << 0 << " " << 0 << " " << 0 << "\n";
	file << "BEGIN_ACTIVE_PLOTS\n";
	file << "0\n";
	file << "END_ACTIVE_PLOTS\n";
	file << "END_DATA_ANALYSIS\n";
}


void DataAnalysisControl::handleSaveConfig( std::ofstream& file )
{
	file << "DATA_ANALYSIS\n";
	file << grids.size( ) << "\n";
	for ( auto grid : grids )
	{
		file << grid.topLeftCorner.row << " " << grid.topLeftCorner.column << "\n";
		file << grid.width << " " << grid.height << " " << grid.pixelSpacing << "\n";
	}
	file << "BEGIN_ACTIVE_PLOTS\n";
	UINT activeCount = 0;
	for ( auto miniPlot : allTinyPlots )
	{
		if ( miniPlot.isActive )
		{
			activeCount++;
		}
	}
	file << activeCount << "\n";
	for ( auto miniPlot : allTinyPlots )
	{
		if ( miniPlot.isActive )
		{
			file << miniPlot.name << "\n";
			file << miniPlot.whichGrid << "\n";
		}
	}
	file << "END_ACTIVE_PLOTS\n";
	file << "END_DATA_ANALYSIS\n";
}


unsigned __stdcall DataAnalysisControl::plotterProcedure(void* voidInput)
{
	realTimePlotterInput* input = (realTimePlotterInput*)voidInput;
	// make vector of plot information classes.
	vector<PlottingInfo> allPlots;
	/// open files
	for (auto plotInc : range(input->plotInfo.size()))
	{
		std::string tempFileName = PLOT_FILES_SAVE_LOCATION + "\\" + input->plotInfo[plotInc].name +  "."
			+ PLOTTING_EXTENSION;
		allPlots.push_back(PlottingInfo::PlottingInfo(tempFileName));
		allPlots[plotInc].setGroups(input->analysisLocations);
	}
	if (allPlots.size() == 0)
	{
		// no plots to run so just quit.
		return 0;
	}
	/// check pictures per experiment
	for (auto plotInc : range(allPlots.size()))
	{
		if (allPlots[0].getPicNumber() != allPlots[plotInc].getPicNumber())
		{
			errBox("ERROR: Number of pictures per experiment don't match between plots. The plotting thread will "
					"close.");
			return 0;
		}
	}
	int totalNumberOfPixels = 0;
	int numberOfLossDataPixels = 0;	
	/// figure out which pixels need any data
	for (auto plotInc : range(allPlots.size()))
	{
		for (auto pixelInc : range(allPlots[plotInc].getPixelNumber()))
		{
			UINT whichGrid = input->plotInfo[plotInc].whichGrid;
			UINT groupNum = input->grids[whichGrid].height * input->grids[whichGrid].width;
			for (auto groupInc : range(groupNum))
			{
				bool alreadyExists = false;
			}
		}
	}
	// 
	if (totalNumberOfPixels == 0)
	{
		// no locations selected for analysis; quit.
		// return 0; ??? why is this commented out?
	}
	/// Initialize Arrays for data. (using std::vector above)
	// thinking about making these experiment-picture sized and resetting after getting the needed data out of them.
	
	//vector<vector<long>> countData( groupNum );
	//vector<vector<int> > atomPresentData( groupNum );
	// countData[gridNumber][pixel Indicator][picture number indicator] = pixelCount;
	vector<vector<vector<long>>> countData( input->grids.size() );	
	vector<vector<vector<int>>> atomPresentData( input->grids.size() );
	for ( auto gridCount : range(input->grids.size( )) )
	{
		UINT groupNum = input->grids[gridCount].height * input->grids[gridCount].width;
		countData[gridCount] = vector<vector<long>>( groupNum );
		atomPresentData[gridCount] = vector<vector<int>>( groupNum );
	}
	// atomPresentData[pixelIndicator][picture number] = true if atom present, false if atom not present;
	// finalData[plot][dataset][group][repetitionNumber];
	vector<vector<vector<vector<long> > > > finalCountData( allPlots.size( ) );
	vector<vector<vector<std::pair<double, ULONG> > > > finalDataNew( allPlots.size( ) );
	vector<variationData> finalAvgs( allPlots.size( ) ), finalErrorBars( allPlots.size( ) ), 
		finalXVals( allPlots.size( ) );
	// Averaged over all pixels (avgAvg is the average of averages over repetitions)
	vector<avgData> avgAvg( allPlots.size( )), avgErrBar( allPlots.size( )), avgXVals( allPlots.size( ));
	// newData[plot][dataSet][group] = true if new data so change some vector sizes.
	vector<vector<vector<bool> > > newData( allPlots.size( ));
	vector<vector<vector<std::deque<double>>>> finalHistData( allPlots.size( ) );
	vector<vector<vector<std::map<int, std::pair<int, ULONG>>>>> histogramData( allPlots.size( ));
	for (auto plotInc : range(allPlots.size()))
	{
		UINT datasetNumber = allPlots[plotInc].getDataSetNumber( );
		histogramData[plotInc].resize( datasetNumber );
		finalCountData[plotInc].resize( datasetNumber );
		finalDataNew[plotInc].resize( datasetNumber );
		finalAvgs[plotInc].resize( datasetNumber );
		finalErrorBars[plotInc].resize( datasetNumber );
		finalXVals[plotInc].resize( datasetNumber );
		avgAvg[plotInc].resize( datasetNumber );
		avgErrBar[plotInc].resize( datasetNumber );
		avgXVals[plotInc].resize( datasetNumber );
		for (auto dataSetInc : range(allPlots[plotInc].getDataSetNumber()))
		{
			UINT whichGrid = input->plotInfo[plotInc].whichGrid;
			UINT groupNum = input->grids[whichGrid].height * input->grids[whichGrid].width;
			histogramData[plotInc][dataSetInc].resize( groupNum );
			finalCountData[plotInc][dataSetInc].resize( groupNum );
			finalAvgs[plotInc][dataSetInc].resize( groupNum );
			finalDataNew[plotInc][dataSetInc].resize( groupNum );
			finalErrorBars[plotInc][dataSetInc].resize( groupNum );
			finalXVals[plotInc][dataSetInc].resize( groupNum );
		}
	}
	UINT noAtomsCounter = 0, atomCounterTotal = 0, currentThreadPictureNumber = 1, plotNumberCount = 0;
	/// Start loop waiting for plots
	while ((*input->active || (input->atomQueue->size() > 0)) && (!*input->aborting))
	{
		// if no image, continue. 0th element is queue, 2nd element is grid num, always at least 1 grid.
		if (input->atomQueue->size() == 0)
		{
			continue;
		}
		if ( input->needsCounts )
		{
			// this part of code hasn't been implemented properly in a while, trying to maintain for later fix. Feb 13th 2018
			imageQueue tempImage( input->grids.size( ) );
			std::lock_guard<std::mutex> locker( *input->plotLock );
			if ( input->imQueue->size( ) == 0 )
			{
				// strange... spurious wakeups or memory corruption happening here?
				continue;
			}
			tempImage = input->imQueue->front( );
			if ( tempImage.size( ) == 0 )
			{
				// strange... spurious wakeups or memory corruption happening here?
				continue;
			}
			/// for all pixels... gather count information
			for ( auto gridCount : range( input->grids.size() ) )
			{
				UINT locIndex = 0;
				for ( auto row : range( input->grids[gridCount].width ) )
				{
					for ( auto column : range( input->grids[gridCount].height ) )
					{
						countData[gridCount][locIndex].push_back( tempImage[gridCount].image[locIndex] );
						locIndex++;
					}
				}
			}
		}
		/// get all the atom data
		bool thereIsAtLeastOneAtom = false;
		for ( auto gridCount : range( input->grids.size( ) ) )
		{
			UINT groupNum = input->grids[gridCount].height * input->grids[gridCount].width;
			for ( auto pixelInc : range( groupNum ) )
			{
				// look at the most recent image.
				if ( input->atomQueue->at( 0 )[gridCount].image[pixelInc] )
				{
					thereIsAtLeastOneAtom = true;
					atomPresentData[gridCount][pixelInc].push_back( 1 );
				}
				else
				{
					atomPresentData[gridCount][pixelInc].push_back( 0 );
				}
			}
		}
		if ( thereIsAtLeastOneAtom )
		{
			noAtomsCounter = 0;
		}
		else
		{
			noAtomsCounter++;
		}
		if (noAtomsCounter >= input->alertThreshold && input->wantAtomAlerts )
		{
			input->comm->sendNoAtomsAlert( );
		}
		/// check if have enough data to plot
		if (currentThreadPictureNumber % allPlots[0].getPicNumber() != 0)
		{
			// In this case, not enough data to plot a point yet, but I've just analyzed a pic, so remove that pic.
			std::lock_guard<std::mutex> locker(*input->plotLock);
			if ( input->needsCounts && input->imQueue->size( ) > 0 )
			{
				input->imQueue->erase( input->imQueue->begin( ) );
			}
			if ( input->atomQueue->size( ) > 0 )
			{
				input->atomQueue->erase( input->atomQueue->begin( ) );
			}
			currentThreadPictureNumber++;
			// wait for next picture.
			continue;
		}
		UINT variationNum = (currentThreadPictureNumber-1) / (input->picsPerVariation);
		plotNumberCount++;
		for ( auto plotI : range( allPlots.size( ) ) )
		{
			/// Check Post-Selection Conditions
			UINT whichGrid = input->plotInfo[plotI].whichGrid;
			UINT groupNum = input->grids[whichGrid].height * input->grids[whichGrid].width;
			vector<vector<bool> > satisfiesPsc( allPlots[plotI].getDataSetNumber( ), vector<bool>( groupNum, true ) );
			determineWhichPscsSatisfied( allPlots[plotI], groupNum, atomPresentData[whichGrid], satisfiesPsc );
			// split into one of two big subroutines. The handling here is encapsulated into functions mostly just for 
			// organization purposes.
			if ( allPlots[plotI].getPlotType( ) == "Atoms" )
			{
				DataAnalysisControl::handlePlotAtoms( 
					allPlots[plotI], currentThreadPictureNumber, finalDataNew[plotI], input->dataArrays[plotI], 
					variationNum, satisfiesPsc, plotNumberCount, atomPresentData[whichGrid], input->plottingFrequency, 
					groupNum, input->picsPerVariation );
			}
			else if ( allPlots[plotI].getPlotType( ) == "Pixel Counts" )
			{
				// TODO: Reimplement this here.
			}
			else if ( allPlots[plotI].getPlotType( ) == "Pixel Count Histograms" )
			{
				DataAnalysisControl::handlePlotHist( allPlots[plotI], countData[whichGrid], finalHistData[plotI],
													 satisfiesPsc, histogramData[plotI],  input->dataArrays[plotI],
													 groupNum );
			}
		}
		/// clear data
		// all pixels being recorded, not pixels in a data set.
		for ( auto gridCount : range( input->grids.size( ) ) )
		{
			UINT groupNum = input->grids[gridCount].height * input->grids[gridCount].width;
			for (auto pixelI : range(groupNum))
			{
				countData[gridCount][pixelI].clear();
				atomPresentData[gridCount][pixelI].clear();
			}
		}
		// finally, remove the data from the queue.
		std::lock_guard<std::mutex> locker(*input->plotLock);
		if (input->needsCounts)
		{
			// delete the first entry of the Queue which has just been handled.
			if ( input->imQueue->size( ) != 0 )
			{
				input->imQueue->erase( input->imQueue->begin( ) );
			}
		}
		input->atomQueue->erase(input->atomQueue->begin());
		currentThreadPictureNumber++;
	}
	return 0;
}


void DataAnalysisControl::determineWhichPscsSatisfied( 
	PlottingInfo& info, UINT groupSize, vector<vector<int>> atomPresentData, vector<vector<bool>>& pscSatisfied)
{
	// There's got to be a better way to iterate through these guys...
	for ( auto dataSetI : range( info.getDataSetNumber( ) ) )
	{
		for ( auto groupI : range( groupSize ) )
		{
			for ( auto conditionI : range( info.getConditionNumber( ) ) )
			{
				for ( auto pixelI : range( info.getPixelNumber( ) ) )
				{
					for ( auto picI : range( info.getPicNumber( ) ) )
					{
						// test if condition exists
						int condition = info.getPostSelectionCondition( dataSetI, conditionI, pixelI, picI );
						if ( condition == 0 )
						{
							continue;
						}
						if ( condition == 1 && atomPresentData[groupI][picI] != 1 )
						{
							pscSatisfied[dataSetI][groupI] = false;
						}
						else if ( condition == -1 && atomPresentData[groupI][picI] != 0 )
						{
							pscSatisfied[dataSetI][groupI] = false;
						}
					}
				}
			}
		}
	}
}


void DataAnalysisControl::handlePlotAtoms( PlottingInfo plotInfo, UINT pictureNumber,
										   vector<vector<std::pair<double, ULONG>> >& finData,
										   std::vector<std::shared_ptr<std::vector<dataPoint>>> dataContainers,
										   UINT variationNumber, vector<vector<bool>>& pscSatisfied,
										   int plotNumberCount, vector<vector<int> > atomPresent, UINT plottingFrequency,
										   UINT groupNum, UINT picsPerVariation )
{
	if (pictureNumber % picsPerVariation == plotInfo.getPicNumber())
	{
		// first pic of new variation, so need to update x vals.
		finData = vector<vector<std::pair<double, ULONG>>>(  plotInfo.getDataSetNumber( ), 
															vector<std::pair<double, ULONG>>( groupNum, { 0,0 } ) );
	}
	/// Check Data Conditions
	for (auto dataSetI : range(plotInfo.getDataSetNumber()))
	{
		for (auto groupI :range(groupNum))
		{
			if (pscSatisfied[dataSetI][groupI] == false)
			{
				// no new data.
				continue;
			}
			bool dataVal = true;
			for (auto pixelI : range(plotInfo.getPixelNumber()))
			{
				for (auto picI : range(plotInfo.getPicNumber()))
				{
					// check if there is a condition at all
					int truthCondition = plotInfo.getResultCondition(dataSetI, pixelI, picI);
					if (truthCondition == 0)
					{
						continue;
					}
					int pixel = groupI;
					if (truthCondition == 1 && atomPresent[pixel][picI] != 1)
					{
						dataVal = false;
					}
					// ?? This won't happen... see above continue...
					else if (truthCondition == 0 && atomPresent[groupI][picI] != 0)
					{
						dataVal = false;
					}
				}
			}
			finData[dataSetI][groupI].first += dataVal;
			finData[dataSetI][groupI].second++;
		}
	}
	// Core data structures have been updated. return if not time for a plot update yet.
	if ( plotNumberCount % plottingFrequency != 0 )
	{
		return;
	}
	/// Calculate averages and standard devations for Data sets AND groups...
	for ( auto dataSetI : range(plotInfo.getDataSetNumber( )))
	{
		UINT avgId = dataContainers.size() - dataSetI - 1;
		for ( auto groupI : range( groupNum))
		{
			// Will be function fo groupI and dataSetI; TBD			
			UINT dataId = (dataSetI+1) * groupI;
			// check if first picture of set
			// ?????????????????????? I don't know what this is doing anymore.
			if ( pictureNumber % plottingFrequency != 0 )
			{
				continue;
			}
			// calculate new data points
			double mean = finData[dataSetI][groupI].first / finData[dataSetI][groupI].second;
			double error = mean * ( 1 - mean ) / std::sqrt( finData[dataSetI][groupI].second );
			dataContainers[dataId]->at( variationNumber ).y = mean;
			dataContainers[dataId]->at( variationNumber ).err = error;
		}
		/// calculate averages
		double avgAvgVal = 0, avgErrsVal = 0;
		std::pair<double, ULONG> allDataTempNew(0,0);
		for ( auto data : finData[dataSetI] )
		{
			allDataTempNew.first += data.first;
			allDataTempNew.second += data.second;
		}
		double mean = allDataTempNew.first / allDataTempNew.second;
		double error = mean * ( 1 - mean ) / std::sqrt( allDataTempNew.second );
		dataContainers[avgId]->at( variationNumber ).y = mean;
		dataContainers[avgId]->at( variationNumber ).err = error;
	}
	/// FITTING
	/*
	for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
	{
		if (plotInfo.whenToFit(dataSetI) == REAL_TIME_FIT 
			|| (plotInfo.whenToFit(dataSetI) == FIT_AT_END && pictureNumber == input->picsPerVariation))
		{
			for (UINT groupInc = 0; groupInc < groupNum; groupInc++)
			{
				std::string fitNum = str( groupNum * dataSetI + groupInc);
				// in this case, fitting.
				switch (plotInfo.getFitOption(dataSetI))
				{
					// TODO: Reimplement autofit here?
				}
			}
		}
	}
	*/
}


// using vector = std::vector
void DataAnalysisControl::handlePlotHist( PlottingInfo plotInfo, vector<vector<long>> countData, 
										  vector<vector<std::deque<double>>>& finData, vector<vector<bool>> pscSatisfied, 
										  vector<vector<std::map<int, std::pair<int, ULONG>>>>& histData,
										  std::vector<std::shared_ptr<vector<dataPoint>>> dataArrays, UINT groupNum )
{
	/// options are fundamentally different for histograms.
	// load pixel counts
	for ( auto dataSetI : range(plotInfo.getDataSetNumber( )) )
	{
		for ( auto groupI : range( groupNum ) )
		{
			if ( pscSatisfied[dataSetI][groupI] == false )
			{
				// no new data.
				continue;
			}
			double binWidth = plotInfo.getDataSetHistBinWidth( dataSetI );
			for ( auto pixelI : range( plotInfo.getPixelNumber( ) ) )
			{
				for ( auto picI : range( plotInfo.getPicNumber( ) ) )
				{
					// check if there is a condition at all
					if ( plotInfo.getResultCondition( dataSetI, pixelI, picI ) )
					{
						int binNum = std::round( double( countData[groupI].end( )[-int(plotInfo.getPicNumber( )) 
														 + int(picI)] ) / binWidth );
						/*
						if ( binNum >= histData[ dataSetI ][ groupI ].size ( ) )
						{

						}
						else
						{

						}
						*/

						if ( histData[dataSetI][groupI].find( binNum ) == histData[dataSetI][groupI].end( ) )
						{
							// if bin doesn't exist
							histData[dataSetI][groupI][binNum] = { binNum * binWidth, 1 };
						}
						else
						{
							histData[dataSetI][groupI][binNum].second++;
						}
					}
				}
			}	
			// find the range of bins
			int min_bin = INT_MAX, max_bin = -INT_MAX;
			for ( auto p : histData[ dataSetI ][ groupI ] )
			{
				if ( p.first < min_bin )
				{
					min_bin = p.first;
				}
				if ( p.first > max_bin )
				{
					max_bin = p.first;
				}
			}
			/// check for empty data points and fill them with zeros.
			for ( auto bin_i : range ( max_bin-min_bin ) )
			{
				auto binNum = bin_i + min_bin;
				if ( histData[ dataSetI ][ groupI ].find ( binNum ) == histData[ dataSetI ][ groupI ].end ( ) )
				{
					// if bin doesn't exist
					histData[ dataSetI ][ groupI ][ binNum ] = { binNum * binWidth, 0 };
				}
			}

			// Will be function fo groupI and dataSetI; TBD			
			UINT dataId = (dataSetI + 1) * groupI;
			// calculate new data points
			UINT count = 0;
			dataArrays[dataId]->resize( histData[dataSetI][groupI].size( ) );
			for ( auto& bin : histData[dataSetI][groupI] )
			{
				dataArrays[dataId]->at( count ).x = bin.second.first;
				dataArrays[dataId]->at( count ).y = bin.second.second; 
				dataArrays[dataId]->at( count ).err = 0;
				count++;
			}
		}
	}
}


atomGrid DataAnalysisControl::getAtomGrid( UINT which )
{
	// update the current grid by load stuff from edits before returning.
	if ( grids[which].topLeftCorner == coordinate( 0, 0 ) )
	{
		// don't try updating, it's not used.
		return grids[which];
	}
	try
	{
		CString txt;
		gridSpacing.GetWindowTextA( txt );
		grids[which].pixelSpacing = boost::lexical_cast<long>( str( txt ) );
		gridWidth.GetWindowTextA( txt );
		grids[which].width = boost::lexical_cast<long>( str( txt ) );
		gridHeight.GetWindowTextA( txt );
		grids[which].height = boost::lexical_cast<long>( str( txt ) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		throwNested ( "ERROR: Grid parameters failed to convert to longs!" );
	}
	return grids[which];
}


void DataAnalysisControl::fillPlotThreadInput(realTimePlotterInput* input)
{
	vector<tinyPlotInfo> usedPlots;
	input->plotInfo.clear();

	for (auto plt : allTinyPlots )
	{
		if (plt.isActive)
		{
			input->plotInfo.push_back(plt);
		}
	}
	input->analysisLocations = getAnalysisLocs();
	input->grids = getGrids();
	input->plottingFrequency = updateFrequency;
	// as I fill the input, also check this, which is necessary info for plotting.
	threadNeedsCounts = false;
	for (auto plt : input->plotInfo)
	{
		PlottingInfo info(PLOT_FILES_SAVE_LOCATION + "\\" + plt.name + "." + PLOTTING_EXTENSION);
		if (info.getPlotType() != "Atoms")
		{
			threadNeedsCounts = true;
		}
	}
	input->needsCounts = threadNeedsCounts;
}

std::vector<atomGrid> DataAnalysisControl::getGrids( )
{
	return grids;
}


void DataAnalysisControl::rearrange( AndorRunModes cameraMode, AndorTriggerMode trigMode, int width, int height, 
									 fontMap fonts)
{
	updateFrequencyLabel1.rearrange(cameraMode, trigMode, width, height, fonts);
	updateFrequencyLabel2.rearrange(cameraMode, trigMode, width, height, fonts);
	updateFrequencyEdit.rearrange(cameraMode, trigMode, width, height, fonts);
	header.rearrange(cameraMode, trigMode, width, height, fonts);
	plotListview.rearrange(cameraMode, trigMode, width, height, fonts);
	currentDataSetNumberDisp.rearrange( cameraMode, trigMode, width, height, fonts );
	currentDataSetNumberText.rearrange( cameraMode, trigMode, width, height, fonts );
	manualSetAnalysisLocsButton.rearrange( cameraMode, trigMode, width, height, fonts );
	gridHeader.rearrange( cameraMode, trigMode, width, height, fonts );
	setGridCorner.rearrange( cameraMode, trigMode, width, height, fonts );
	gridSpacingText.rearrange( cameraMode, trigMode, width, height, fonts );
	gridSpacing.rearrange( cameraMode, trigMode, width, height, fonts );
	gridWidthText.rearrange( cameraMode, trigMode, width, height, fonts );
	gridWidth.rearrange( cameraMode, trigMode, width, height, fonts );
	gridHeightText.rearrange( cameraMode, trigMode, width, height, fonts );
	gridHeight.rearrange( cameraMode, trigMode, width, height, fonts );
	gridSelector.rearrange( cameraMode, trigMode, width, height, fonts );
	deleteGrid.rearrange( cameraMode, trigMode, width, height, fonts );
	plotTimerTxt.rearrange( cameraMode, trigMode, width, height, fonts );
	plotTimerEdit.rearrange( cameraMode, trigMode, width, height, fonts );
}


void DataAnalysisControl::handleAtomGridCombo( )
{
	saveGridParams( );
	int sel = gridSelector.GetCurSel( );
	if ( sel == -1 )
	{
		return;
	}
	else if ( sel == grids.size() )
	{
		reloadGridCombo( sel + 1 );
	}
	else if (sel > grids.size())
	{
		thrower ( "ERROR: Bad value for atom grid combobox selection???  (A low level bug, this shouldn't happen)" );
	}
	gridSelector.SetCurSel( sel );
	// load the grid parameters for that selection.
	loadGridParams( grids[sel] );
	selectedGrid = sel;
}


void DataAnalysisControl::reloadGridCombo( UINT num )
{
	grids.resize( num );
	gridSelector.ResetContent( );
	UINT count = 0;
	for ( auto grid : grids )
	{
		std::string txt( str( count++ ) );
		gridSelector.AddString( cstr( txt ) );
	}
	gridSelector.AddString( "New" );
}


void DataAnalysisControl::loadGridParams( atomGrid grid )
{
	std::string txt = str( grid.pixelSpacing );
	gridSpacing.SetWindowText(cstr(txt));
	txt = str( grid.width );
	gridWidth.SetWindowText(cstr(txt));
	txt = str( grid.height );
	gridHeight.SetWindowText( cstr( txt ) );
}


void DataAnalysisControl::saveGridParams( )
{
	CString txt;
	try
	{
		gridSpacing.GetWindowText( txt );
		grids[selectedGrid].pixelSpacing = boost::lexical_cast<long>( str(txt) );
		gridHeight.GetWindowText( txt );
		grids[selectedGrid].height = boost::lexical_cast<long>( str( txt ) );
		gridWidth.GetWindowText( txt );
		grids[selectedGrid].width = boost::lexical_cast<long>( str( txt ) );
	}
	catch ( boost::bad_lexical_cast&)
	{
		throwNested ( "ERROR: failed to convert grid parameters to longs while saving grid data!" );
	}
}


vector<std::string> DataAnalysisControl::getActivePlotList()
{
	vector<std::string> list;
	for ( auto plot : allTinyPlots )
	{
		if ( plot.isActive ) 
		{
			list.push_back( plot.name );
		}
	}
	return list;
}



bool DataAnalysisControl::getLocationSettingStatus()
{
	return currentlySettingAnalysisLocations;
}


void DataAnalysisControl::updateDataSetNumberEdit( int number )
{
	if ( number > 0 )
	{
		currentDataSetNumberDisp.SetWindowTextA( cstr( number ) );
	}
	else
	{
		currentDataSetNumberDisp.SetWindowTextA( "None" );
	}
}


void DataAnalysisControl::analyze( std::string date, long runNumber, long accumulations, 
								   EmbeddedPythonHandler* pyHandler, Communicator* comm )
{
	// Note: python is initialized in the constructor for the data handler object. 
	// Get information to send to the python script from inputParam
	//pyHandler->runDataAnalysis( date, runNumber, accumulations, atomLocations );
}


bool DataAnalysisControl::buttonClicked()
{
	return (manualSetAnalysisLocsButton.GetCheck() || setGridCorner.GetCheck());

}


void DataAnalysisControl::onCornerButtonPushed( )
{
	if ( setGridCorner.GetCheck( ) )
	{
		// if pressed currently, then upress it.
		setGridCorner.SetCheck( 0 );
		setGridCorner.SetWindowTextA( "Set Grid Top-Left Corner" );
		currentlySettingGridCorner = false;
	}
	else
	{
		// else press it.
		atomLocations.clear( );
		grids[0].topLeftCorner = { 0,0 };
		setGridCorner.SetCheck( 1 );
		setGridCorner.SetWindowTextA( "Right-Click Top-Left Corner of New Grid Location" );
		currentlySettingGridCorner = true;
	}
}


// handles the pressing of the analysis points button.
// TODO: handle different cases where single locations or pairs of locations are being analyzed. 
void DataAnalysisControl::onManualButtonPushed()
{	
	if ( manualSetAnalysisLocsButton.GetCheck() )
	{
		// if pressed currently, then upress it.
		manualSetAnalysisLocsButton.SetCheck(0);
		manualSetAnalysisLocsButton.SetWindowTextA( "Set Analysis Points" );
		currentlySettingAnalysisLocations = false;
	}
	else
	{
		// else press it.
		atomLocations.clear();
		grids[0].topLeftCorner = { 0,0 };
		manualSetAnalysisLocsButton.SetCheck( 1 );
		manualSetAnalysisLocsButton.SetWindowTextA( "Right-Click Relevant Points and Reclick" );
		currentlySettingAnalysisLocations = true;
	}
}


UINT DataAnalysisControl::getSelectedGridNumber( )
{
	return selectedGrid;
}


atomGrid DataAnalysisControl::getCurrentGrid( )
{
	return getAtomGrid( selectedGrid );
}


void DataAnalysisControl::handlePictureClick( coordinate location )
{
	if ( setGridCorner.GetCheck( ) )
	{
		grids[selectedGrid].topLeftCorner = location;
		onCornerButtonPushed( );
	}
	else if ( manualSetAnalysisLocsButton.GetCheck( ) )
	{
		bool exists = false;
		for ( UINT locInc = 0; locInc < atomLocations.size( ); locInc++ )
		{
			if ( location.row == atomLocations[locInc].row && location.column == atomLocations[locInc].column )
			{
				exists = true;
			}
		}
		if ( !exists )
		{
			atomLocations.push_back( location );
		}
	}
}


vector<coordinate> DataAnalysisControl::getAnalysisLocs()
{
	return atomLocations;
}


void DataAnalysisControl::clearAtomLocations()
{
	atomLocations.clear();
}


/*
 * User wants to change or view something about the plot. Figure out what based on click location and do it.
 */
void DataAnalysisControl::handleDoubleClick(fontMap* fonts, UINT currentPicsPerRepetition)
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	plotListview.ScreenToClient(&cursorPos);
	int subitemIndicator;
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator;
	plotListview.SubItemHitTest(&myItemInfo);
	itemIndicator = myItemInfo.iItem;
	subitemIndicator = myItemInfo.iSubItem;
	if (itemIndicator == -1)
	{
		// user didn't click in an item.
		return;
	}
	if ( itemIndicator == allTinyPlots.size())
	{
		// new plot, open plot creator.
		PlotDesignerDialog dlg(fonts, currentPicsPerRepetition);
		dlg.DoModal();
		reloadListView();
		return;
	}
	/// Handle different subitem clicks
	switch (subitemIndicator)
	{
		case 0:
		{
			//... clearly I haven't actually implemented anything here....
			// prompt for a name
			std::string newName;
			if (newName == "")
			{
				// probably canceled.
				break;
			}
			// rename the file 
			// ...???
			// update the screen
			plotListview.SetItem(newName, itemIndicator, subitemIndicator);
			break;
		}
		case 1:
		{
			// which grid
			std::string gridStr;
			TextPromptDialog dialog( &gridStr, "Which # atom grid should this plot use?" );
			dialog.DoModal( );
			UINT gridNum;
			try
			{
				gridNum = boost::lexical_cast<unsigned long>( gridStr );
			}
			catch ( boost::bad_lexical_cast&)
			{
				throwNested ( "ERROR: bad value for grid #! Expecting a positive integer." );
			}
			if ( gridNum >= grids.size( ) )
			{
				thrower ( "ERROR: Grid number picked is larger than the number of grids available!" );
			}
			allTinyPlots[itemIndicator].whichGrid = gridNum;
			plotListview.SetItem( str ( gridNum ), itemIndicator, subitemIndicator );
			break;
		}
		case 2:
		{
			// edit existing plot file using the plot designer.
			try
			{
				PlotDesignerDialog dlg(fonts, PLOT_FILES_SAVE_LOCATION + "\\" + allTinyPlots[itemIndicator].name + "."
										+ PLOTTING_EXTENSION);
				dlg.DoModal();
			}
			catch (Error& err)
			{
				errBox(err.trace());
			}
			reloadListView();
			break;
		}
		case 3:
		{
			/// view plot settings.
			try
			{
				infoBox(PlottingInfo::getAllSettingsStringFromFile(
					PLOT_FILES_SAVE_LOCATION + "\\" + allTinyPlots[itemIndicator].name + "."+  PLOTTING_EXTENSION));
			}
			catch (Error& err)
			{
				errBox(err.trace());
			}
			break;
		}
		case 4:
		{
			/// toggle active
			allTinyPlots[itemIndicator].isActive = !allTinyPlots[itemIndicator].isActive;
			plotListview.SetItem( allTinyPlots[ itemIndicator ].isActive ? "YES" : "NO", itemIndicator, subitemIndicator );
			break;
		}
	}
}


void DataAnalysisControl::reloadListView()
{
	vector<std::string> names = ProfileSystem::searchForFiles(PLOT_FILES_SAVE_LOCATION, str("*.") + PLOTTING_EXTENSION);
	plotListview.DeleteAllItems();
	allTinyPlots.clear();
	for (auto item : range(names.size()))
	{
		plotListview.InsertItem(names[item], item, 0);
		plotListview.SetItem( "0", item, 1 );
		plotListview.SetItem ( "[ ]", item, 2 );
		plotListview.SetItem ( "[ ]", item, 3 );
		plotListview.SetItem("NO", item, 4);
		tinyPlotInfo tempInfo;
		PlottingInfo info( PLOT_FILES_SAVE_LOCATION + "\\" + names[item] + "." + PLOTTING_EXTENSION );
		tempInfo.isHist = (info.getPlotType( ) == "Pixel Count Histograms");
		tempInfo.name = names[item];
		tempInfo.isActive = false;
		tempInfo.whichGrid = 0;		
		allTinyPlots.push_back(tempInfo);
	}
	plotListview.insertBlankRow ( );

}


void DataAnalysisControl::handleRClick()
{
	// delete...
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	plotListview.ScreenToClient(&cursorPos);
	int subitemIndicator = plotListview.HitTest(cursorPos);
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator = plotListview.SubItemHitTest(&myItemInfo);
	if (itemIndicator == -1 || itemIndicator == allTinyPlots.size())
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer = promptBox( "Delete Plot " + allTinyPlots[itemIndicator].name + "?", MB_YESNO );
	if (answer == IDYES)
	{
		int result = DeleteFile(cstr(PLOT_FILES_SAVE_LOCATION + "\\" + allTinyPlots[itemIndicator].name + "."
									  + PLOTTING_EXTENSION));
		if (!result)
		{
			errBox("Failed to delete script file! Error code: " + str(GetLastError()));
			return;
		}
		plotListview.DeleteItem(itemIndicator);
		allTinyPlots.erase(allTinyPlots.begin() + itemIndicator);
	}
}

