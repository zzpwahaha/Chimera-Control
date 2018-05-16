#pragma once

#include "Communicator.h"

#include "debugInfo.h"
#include "profileSettings.h"
#include "NiawgStructures.h"
#include "rerngStructures.h"
#include "directions.h"
#include "rerngParams.h"
#include "rerngThreadInput.h"
#include "Matrix.h"
#include "ScriptStream.h"
#include "constants.h"
#include "atomGrid.h"

#include "Fgen.h"
#include <algorithm>
#include <memory>
#include <cmath>
#include <mutex>
#include <chrono>

#include <boost/container/vector.hpp>

struct MasterThreadInput;
struct seqInfo;
class NiawgWaiter;


/** 
  * One of the biggest & most complicated classes in the code.
  * Part of this class is effectively an FGEN wrapper. You could extract that if you have other devies which use fgen.
  */
class NiawgController
{
	public:
		NiawgController( UINT trigRow, UINT trigNumber, bool safemode );
		void initialize();
		void cleanupNiawg( profileSettings profile, bool masterWasRunning, seqInfo& expInfo, NiawgOutput& output,
						   Communicator* comm, bool dontGenerate );
		bool rerngThreadIsActive();
		std::string getCurrentScript();
		bool niawgIsRunning();
		void handleStartingRerng( MasterThreadInput* input, NiawgOutput& output );
		void prepareNiawg( MasterThreadInput* input, NiawgOutput& output, seqInfo& expInfo, std::string& warnings, 
						   std::vector<ViChar>& userScriptSubmit, bool& foundRearrangement, rerngOptions& rInfo,
						   rerngOptionsForm rInfoForm,
						   std::vector<parameterType>& variables );
		bool outputVaries( NiawgOutput output );
		void checkThatWaveformsAreSensible( std::string& warnings, NiawgOutput& output );		
		void handleVariations( NiawgOutput& output, std::vector<std::vector<parameterType>>& variables, UINT variation, 
							   std::vector<long>& mixedWaveSizes, std::string& warnings, debugInfo& debugOptions, 
							   UINT totalVariations );
		void analyzeNiawgScript( ScriptStream& script, NiawgOutput& output, profileSettings profile,
								 debugInfo& options, std::string& warnings, rerngOptions rInfo,
								 std::vector<parameterType>& variables );
		void flashVaries( waveInfoForm& wave );
		void rerngFormToOutput( waveInfoForm& waveForm, waveInfo& wave, std::vector<parameterType>& varibles,
								UINT variation );
		void writeStaticNiawg( NiawgOutput& output, debugInfo& options, std::vector<parameterType>& variables,
							   bool deleteWaveAfterWrite=true );
		void deleteWaveData( simpleWave& core );
		void loadCommonWaveParams( ScriptStream& script, simpleWaveForm& wave );
		void handleStandardWaveformFormSingle( NiawgOutput& output, std::string cmd, ScriptStream& script,
											   std::vector<parameterType>& variables );
		void loadWaveformParametersFormSingle( NiawgOutput& output, std::string cmd, ScriptStream& script,
											   std::vector<parameterType> variables, int axis, simpleWaveForm& wave );
		void loadFullWave( NiawgOutput& output, std::string cmd, ScriptStream& script, 
						   std::vector<parameterType>& variables, simpleWaveForm& wave );
		void setDefaultWaveforms( MainWindow* mainWin );
		static long waveformSizeCalc( double time );
		static double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType );
		// programming the device
		void restartDefault();
		void turnOffRerng( );
		void waitForRerng( );
		void programVariations( UINT variation, std::vector<long>& variedMixedSize, NiawgOutput& output );
		void programNiawg( MasterThreadInput* input, NiawgOutput& output, std::string& warnings, UINT variation, 
						   UINT totalVariations, std::vector<long>& variedMixedSize, 
						   std::vector<ViChar>& userScriptSubmit );
		void setDefaultWaveformScript( );
		void turnOff();
		void turnOn();
		// Other
		void setRunningState( bool newRunningState );
		std::pair<UINT, UINT> getTrigLines( );
		UINT getNumberTrigsInScript( );
		bool isOn( );
		void streamWaveform( );
		FgenFlume fgenConduit;
		static void smartRearrangement( Matrix<bool> source, Matrix<bool> target, niawgPair<ULONG>& finTargetPos,
										niawgPair<ULONG> finalPos, std::vector<simpleMove> &operationsMatrix,
										rerngOptions options );
		// From the single moves operationsmatrix, this function calculates parallel moves (rows and columns)
		static void optimizeMoves( std::vector<simpleMove> singleMoves, Matrix<bool> source,
								   std::vector<complexMove> &flashMoves, rerngOptions options );

	private:
		void preWriteRerngWaveforms( rerngThreadInput* input );
		static void writeToFile( std::vector<double> waveVals );
		void rerngOptionsFormToFinal( rerngOptionsForm& form, rerngOptions& data, std::vector<parameterType>& variables,
									  UINT variation );
		void finalizeScript( ULONGLONG repetitions, std::string name, std::vector<std::string> workingUserScripts,
							 std::vector<ViChar>& userScriptSubmit, bool repeatForever );
		void mixFlashingWaves( waveInfo& wave, double deadTime, double staticMovingRatio );
		std::vector<double> calcFinalPositionMove( niawgPair<ULONG> targetPos, niawgPair<ULONG> finalPos, 
												   double freqSpacing, Matrix<bool> target, 
												   niawgPair<double> cornerFreqs, double moveTime );
		void streamRerng( );
		waveInfoForm toWaveInfoForm( simpleWaveForm wave );
		void simpleFormVaries( simpleWaveForm& wave );
		void simpleFormToOutput( simpleWaveForm& formWave, simpleWave& wave,
								 std::vector<parameterType>& varibles = std::vector<parameterType>( ), UINT variation = -1 );
		void flashFormToOutput( waveInfoForm& waveForm, waveInfo& wave,
								std::vector<parameterType>& varibles = std::vector<parameterType>( ),
								UINT variation = -1 );
		void deleteRerngWave( );
		void startRerngThread( std::vector<std::vector<bool>>* atomQueue, waveInfo wave, Communicator* comm,
							   std::mutex* rerngLock, chronoTimes* andorImageTimes, chronoTimes* grabTimes,
							   std::condition_variable* rerngConditionWatcher, rerngOptions rerngInfo, atomGrid grid );
		static niawgPair<ULONG> convolve( Matrix<bool> atoms, Matrix<bool> target );
		void writeStandardWave( simpleWave& wave, debugInfo options, bool isDefault );
		void writeFlashing( waveInfo& wave, debugInfo& options, UINT variation );
		void generateWaveform( channelWave & waveInfo, debugInfo& options, long int sampleNum, double time,
							   bool powerCap = false, bool constPower = CONST_POWER_OUTPUT );
		void mixWaveforms( simpleWave& waveCore, bool writeThisToFile );
		void calcWaveData( channelWave& inputData, std::vector<ViReal64>& readData, long int sampleNum, double time,
						   bool powerCap=false, bool constPower=CONST_POWER_OUTPUT );

		void handleMinus1Phase( simpleWave& waveCore, simpleWave prevWave );
		void createFlashingWave( waveInfo& wave, debugInfo options );
		UINT writeToFileNumber = 0;
		void loadStandardInputFormType( std::string inputType, channelWaveForm &wvInfo );
		void openWaveformFiles( );
		bool isLogic( std::string command );
		void handleLogicSingle( ScriptStream& script, std::string inputs, std::string &scriptString );
		bool isSpecialWaveform( std::string command );
		void handleSpecialWaveformFormSingle( NiawgOutput& output, profileSettings profile, std::string cmd,
											  ScriptStream& scripts, debugInfo& options, rerngOptions rInfo,
											  std::vector<parameterType>& variables );
		bool isStandardWaveform( std::string command );
		bool isSpecialCommand( std::string command );
		void handleSpecialFormSingle( ScriptStream& scripts, NiawgOutput& output, std::string inputTypes, 
									  profileSettings profile, debugInfo& options, std::string& warnings );
		void finalizeStandardWave( simpleWave& wave, debugInfo& options, bool powerCap = false, 
								   bool constPower = CONST_POWER_OUTPUT );
		/// member variables
 		// Important. This can change if you change computers.
 		const ViRsrc NI_5451_LOCATION = "Dev6";
 		niawgPair<std::string> currentScripts;
		std::string currentScript;
		bool runningState;
		bool on;
		// don't take the word "library" too seriously... it's just a listing of all of the waveforms that have been 
		// already created.
		std::array<std::vector<std::string>, MAX_NIAWG_SIGNALS * 4> waveLibrary;
		//
		ViInt32 streamWaveHandle;
		std::string streamWaveName = "streamedWaveform";
		std::vector<double> streamWaveformVals;
		std::vector<ViReal64> defaultMixedWaveform;
		std::string defaultWaveName;
		std::vector<ViChar> defaultScript;
		ViSession sessionHandle;

		// not used at the moment, but could revive the phase correction feature back later.
		double calculateCorrectionTime( channelWave& wvData1, channelWave& wvData2, std::vector<double> startPhases, 
										std::string order, double time, long sampleNum);
		const UINT triggerRow;
		const UINT triggerNumber;
		/// Rearrangement stuff
		std::vector<rerngContainer<double>> moveBiasCalibrations;
		std::string rerngWaveName = "rearrangeWaveform";
		std::vector<double> rerngWaveVals;
		HANDLE rerngThreadHandle;
		static UINT __stdcall rerngThreadProcedure( LPVOID input );
		UINT triggersInScript=0;
		// true = active;
		std::atomic<bool> threadStateSignal;
		std::vector<std::vector<bool>> finalState;
		// could set different thresholds for each location in the camera if expect imbalance.
		int threshold;
		std::vector<double> makeRerngWave( rerngInfo& info, double staticMovingRatio, double moveBias, double deadTime, 
										   UINT sourceRows, UINT sourceCols, complexMove move );
		std::vector<double> makeFastRerngWave( rerngInfo& rerngSettings, UINT sourceRows, UINT sourceCols, 
											   complexMove moveInfo, rerngOptions options, double moveBias );
		// returns sign of x.
		static int sign( int );
		// returns cost, which is total travel distance. Algorithm from: 
		// http://cs.stanford.edu/group/acm/SLPC/notebook.pdf
		// You have to give it the cost matrix, and to empty vectors, in which it will write
		static double minCostMatching( Matrix<double> cost, std::vector<int> & Lmate, std::vector<int> & Rmate );
		// returns a list of single elementary (left,right,up,down) moves. Size is 4 x n_moves: Initialx,Initialy,Finalx,Finaly
		static double rearrangement( Matrix<bool> & sourceMatrix, Matrix<bool> & targetMatrix,
									 std::vector<simpleMove>& moveSequence );
		std::vector<std::string> evolveSource( Matrix<bool> source, std::vector<complexMove> flashMoves );
		// returns maximal number of moves given a targetmatrix.
		static UINT getMaxMoves( Matrix<bool> targetMatrix );
};


