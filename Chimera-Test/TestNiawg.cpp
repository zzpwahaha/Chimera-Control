#include "stdafx.h"
#include "CppUnitTest.h"
#include "afxwin.h"
#include "../Chimera/NiawgController.h"
#include "../Chimera/miscCommonFunctions.h"
#include "TestMacros.h"
#include <string>
#include <fstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace TestNiawg
{
	TEST_CLASS( Rearrangement )
	{
		TEST_METHOD( Simple_Rerng )
		{
			// single move; obvious solution to rearrangement algorithm
			auto trigRow = DioRows::which::A;
			UINT trigNum( 0 );
			bool safemode( true );
			NiawgController niawg( trigRow, trigNum, safemode );
			Matrix<bool> source( 6, 3 );
			Matrix<bool> target( 6, 3 );

			loadBools( source, std::vector<bool>(
			{ 0,1,0,
				0,1,0,
				0,1,0,
				0,0,1,
				0,1,0,
				0,1,0 } ) );
			loadBools( target, standardTarget );
			std::vector<simpleMove> moves;
			niawgPair<ULONG> finPos;
			niawg.smartTargettingRearrangement( source, target, finPos, { 0,0 }, moves, rerngGuiOptions( ) );
			Assert::AreEqual( size_t( 1 ), moves.size( ) );
			Assert::IsTrue( simpleMove( { 3,2,3,1 } ) == moves.front( ) );
		}
		TEST_METHOD( Complex_Rerng )
		{
			// many single moves (no parallelization), result compared to solution that's known to work.
			auto trigRow = DioRows::which::A;
			UINT trigNum( 0 );
			bool safemode( true );
			NiawgController niawg( trigRow, trigNum, safemode );
			Matrix<bool> source( 6, 3 );
			Matrix<bool> target( 6, 3 );

			loadBools( source, std::vector<bool>(
			{ 0,1,1,
				0,0,0,
				0,0,0,
				1,0,1,
				1,1,0,
				0,1,1 } ) );
			loadBools( target, standardTarget );
			std::vector<simpleMove> moves;
			niawgPair<ULONG> finPos;
			niawg.smartTargettingRearrangement ( source, target, finPos, { 0,0 }, moves, rerngGuiOptions( ),false, 
												 false );
			// the following moves were copied from a trial that I know rearranges correctly. Details of the moves
			// are hard to actualy predict.
			Assert::AreEqual( size_t( 5 ), moves.size( ) );
			Assert::IsTrue( simpleMove( { 0,2,1,2 } ) == moves[0] );
			Assert::IsTrue( simpleMove( { 1,2,1,1 } ) == moves[1] );
			Assert::IsTrue( simpleMove( { 3,0,2,0 } ) == moves[2] );
			Assert::IsTrue( simpleMove( { 2,0,2,1 } ) == moves[3] );
			Assert::IsTrue( simpleMove( { 3,2,3,1 } ) == moves[4] );
		}
		TEST_METHOD( No_Flash_Rerng )
		{
			// a predictable no-flash config.
			auto trigRow = DioRows::which::A;
			UINT trigNum( 0 );
			bool safemode( true );
			NiawgController niawg( trigRow, trigNum, safemode );
			Matrix<bool> source( 6, 3 );
			Matrix<bool> target( 6, 3 );

			loadBools( source, std::vector<bool>(
			{   0,1,0,
				0,0,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0 } ) );
			loadBools( target, std::vector<bool>(
			{   0,0,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0 } ) );
			std::vector<simpleMove> smoves;
			std::vector<complexMove> moves;
			niawgPair<ULONG> finPos;
			niawg.smartTargettingRearrangement ( source, target, finPos, { 0,0 }, smoves, rerngGuiOptions( ) );
			niawg.optimizeMoves( smoves, source, moves, rerngGuiOptions( ) );
			Assert::AreEqual( size_t( 1 ), moves.size( ) );
			Assert::AreEqual( false, moves.front( ).needsFlash );
			Assert::AreEqual( false, moves.front( ).isInlineParallel );
			Assert::IsTrue( dir::up == moves.front( ).moveDir );
			Assert::AreEqual( size_t( 1 ), moves.front( ).locationsToMove.size( ) );
			Assert::IsTrue( int_coordinate( { 0,1 } ) == moves.front( ).locationsToMove.front( ) );
		}
		TEST_METHOD( Pi_Parallel_Rerng )
		{
			auto trigRow = DioRows::which::A;
			UINT trigNum( 0 );
			bool safemode( true );
			NiawgController niawg( trigRow, trigNum, safemode );
			Matrix<bool> source( 6, 3 );
			Matrix<bool> target( 6, 3 );
			loadBools( source, std::vector<bool>(
			{ 0,1,0,
				0,0,1,
				0,1,0,
				0,0,1,
				0,1,0,
				0,1,0 } ) );
			loadBools( target, standardTarget );
			std::vector<simpleMove> smoves;
			std::vector<complexMove> moves;
			niawgPair<ULONG> finPos;
			auto opt = rerngGuiOptions ( );
			opt.parallel = parallelMoveOption::full;
			niawg.smartTargettingRearrangement ( source, target, finPos, { 0,0 }, smoves, opt );
			niawg.optimizeMoves( smoves, source, moves, opt );
			Assert::AreEqual( size_t( 1 ), moves.size( ) );
			auto move = moves.front( );
			Assert::AreEqual( false, move.isInlineParallel );
			Assert::AreEqual( true, move.needsFlash );
			Assert::IsTrue( dir::left == move.moveDir );
			Assert::AreEqual( size_t( 2 ), move.locationsToMove.size( ) );
			Assert::IsTrue( int_coordinate( { 1,2 } ) == move.locationsToMove[0] );
			Assert::IsTrue( int_coordinate( { 3,2 } ) == move.locationsToMove[1] );
		}
		private:
			std::vector<bool> standardTarget =
			{ 0,1,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0 };
	};


	TEST_CLASS ( TestNiawg )
	{
		public:
		CONNECTED_TEST ( c_Connect_To_Niawg )
		{
			NiawgController niawg ( DioRows::which::A, 0, false );
			niawg.initialize ( );
			niawg.fgenConduit.getDeviceInfo ( );
		}
		TEST_METHOD ( Init_Niawg )
		{
			DioRows::which trigRow = DioRows::which::A;
			UINT trigNum ( 0 );
			bool safemode ( true );
			NiawgController niawg ( trigRow, trigNum, safemode );
			auto res = niawg.getTrigLines ( );
			Assert::AreEqual ( DioRows::toStr(res.first), DioRows::toStr ( trigRow ) );
			Assert::AreEqual ( res.second, trigNum );
			Assert::AreEqual ( niawg.getCurrentScript ( ), std::string ( "" ) );
			UINT numTrigs = niawg.getNumberTrigsInScript ( );
			Assert::AreEqual ( UINT ( 0 ), numTrigs );
		}
		TEST_METHOD ( SimpleScript_Interpret_Script )
		{
			// assert that analyzeNiawgScript correctly creates a simple form, i.e. it reads the
			// script correctly and fills out the form correctly.
			DioRows::which trigRow = DioRows::which::A;
			UINT trigNum ( 0 );
			bool safemode ( true );
			NiawgController niawg ( trigRow, trigNum, safemode );
			ScriptStream stream ( simpleScript );
			NiawgOutput output;
			//
			niawg.analyzeNiawgScript ( stream, output, profileSettings ( ), debugInfo ( ), std::string ( ), rerngGuiOptionsForm ( ),
									   std::vector<parameterType> ( ) );
			 // asserts
			Assert::AreEqual ( size_t ( 1 ), output.waveFormInfo.size ( ) );
			Assert::AreEqual ( size_t ( 1 ), output.waves.size ( ) );
			Assert::AreEqual ( false, bool ( output.isDefault ) );
			Assert::AreEqual ( str ( "generate Waveform1\n" ), output.niawgLanguageScript );
			auto& form = output.waveFormInfo[ 0 ];
			Assert::AreEqual ( false, form.flash.isFlashing );
			Assert::AreEqual ( false, form.rearrange.isRearrangement );
			Assert::AreEqual ( false, form.isStreamed );
			auto& core = form.core;
			Assert::AreEqual ( size_t ( 2 ), core.chan.size ( ) );
			Assert::AreEqual ( std::string ( "Waveform1" ), core.name );
			Assert::AreEqual ( std::string ( "0.01" ), core.time.expressionStr );
			Assert::AreEqual ( false, core.varies );
			UINT count = 0;
			std::vector<std::string> freqs = { "70", "80" };
			for ( auto& axis : core.chan )
			{
				Assert::AreEqual ( std::string ( "#" ), axis.delim );
				Assert::AreEqual ( 0, axis.phaseOption );
				Assert::AreEqual ( 1, axis.initType );
				Assert::AreEqual ( size_t ( 1 ), axis.signals.size ( ) );
				auto& signal = axis.signals[ 0 ];
				Assert::AreEqual ( std::string ( "1" ), signal.initPower.expressionStr );
				Assert::AreEqual ( std::string ( "1" ), signal.finPower.expressionStr );
				Assert::AreEqual ( std::string ( freqs[ count ] ), signal.freqInit.expressionStr );
				Assert::AreEqual ( std::string ( freqs[ count ] ), signal.freqFin.expressionStr );
				Assert::AreEqual ( std::string ( "nr" ), signal.freqRampType );
				Assert::AreEqual ( std::string ( "nr" ), signal.powerRampType );
				count++;
			}
		}
		TEST_METHOD ( SimpleScript_WriteWave_Core )
		{
			// check that the core has the correct values after actually writing the wave. Force actual writing of the 
			// wave, don't use the library. The library is to be tested seperately.
			DioRows::which trigRow = DioRows::which::A;
			UINT trigNum ( 0 );
			bool safemode ( true );
			NiawgController niawg ( trigRow, trigNum, safemode );
			ScriptStream stream ( simpleScript );
			NiawgOutput output;
			//
			niawg.analyzeNiawgScript ( stream, output, profileSettings ( ), debugInfo ( ), std::string ( ), 
									   rerngGuiOptionsForm ( ), std::vector<parameterType> ( ) );
			niawg.writeStaticNiawg ( output, debugInfo ( ), std::vector<parameterType> ( ), true, 
									 niawgLibOption::mode::banned );
			// asserts
			auto& wave = output.waves[ 0 ];
			Assert::AreEqual ( false, wave.flash.isFlashing );
			Assert::AreEqual ( false, wave.rearrange.isRearrangement );
			Assert::AreEqual ( false, wave.isStreamed );
			auto& core = wave.core;
			Assert::AreEqual ( str ( "Waveform1" ), core.name );
			Assert::AreEqual ( size_t ( 2 ), core.chan.size ( ) );
			Assert::AreEqual ( 0.01e-3, core.time );
			Assert::AreEqual ( false, core.varies );
			Assert::AreEqual ( size_t ( 0 ), core.waveVals.size ( ) );
			Assert::AreEqual ( long ( NIAWG_SAMPLE_RATE * 0.01e-3 ), core.sampleNum() );
			UINT count = 0;
			std::vector<double> freqs = { 70.0e6, 80.0e6 };
			for ( auto& axis : core.chan )
			{
				Assert::AreEqual ( std::string ( "#" ), axis.delim );
				Assert::AreEqual ( 0, axis.phaseOption );
				Assert::AreEqual ( 1, axis.initType );
				Assert::AreEqual ( size_t ( 1 ), axis.signals.size ( ) );
				auto& signal = axis.signals[ 0 ];
				Assert::AreEqual ( 1.0, signal.initPower );
				Assert::AreEqual ( 1.0, signal.finPower );
				Assert::AreEqual ( freqs[ count ], signal.freqInit );
				Assert::AreEqual ( freqs[ count ], signal.freqFin );
				Assert::AreEqual ( std::string ( "nr" ), signal.freqRampType );
				Assert::AreEqual ( std::string ( "nr" ), signal.powerRampType );
				count++;
			}
		}
		TEST_METHOD ( VectorizedVsTraditional )
		{
			// check various properties of the wavevals and the wavevals themselves. Using actual calculations, not the
			// library.
			DioRows::which trigRow = DioRows::which::A;
			UINT trigNum ( 0 );
			bool safemode ( true );
			NiawgController niawg ( trigRow, trigNum, safemode );
			ScriptStream stream ( simpleScript );
			NiawgOutput output;
			//
			niawg.analyzeNiawgScript ( stream, output, profileSettings ( ), debugInfo ( ), std::string ( ), rerngGuiOptionsForm ( ),
									   std::vector<parameterType> ( ) );
			niawg.writeStaticNiawg ( output, debugInfo ( ), std::vector<parameterType> ( ), false, niawgLibOption::mode::banned );
			//
			auto& vals = output.waves[ 0 ].core.waveVals;
			// should be long enough to sum close to zero.
			double sum = 0;
			double max = 0, min = 0;
			for ( auto val : vals )
			{
				sum += val;
				if ( val > max )
				{
					max = val;
				}
				else if ( val < min )
				{
					min = val;
				}
			}
			// a couple sanity checks before the complete comparison.
			Assert::AreEqual ( 0.0, sum, 1e-9 );
			Assert::AreEqual ( 0.0, vals[ 0 ] );
			Assert::AreEqual ( 0.0, vals[ 1 ] );
			Assert::AreNotEqual ( vals[ 2 ], vals[ 3 ] );
			Assert::IsTrue ( max < 1.0 );
			Assert::IsTrue ( min > -1.0 );
			Assert::IsTrue ( vals.size ( ) == simpleScriptVals.size ( ) );
			for ( auto valInc : range ( vals.size ( ) ) )
			{
				Assert::AreEqual ( simpleScriptVals[ valInc ], vals[ valInc ], 1e-6 );
			}
		}
		TEST_METHOD ( SimpleScript_WriteWave_Wavevals )
		{
			// check various properties of the wavevals and the wavevals themselves. Using actual calculations, not the
			// library.
			DioRows::which trigRow = DioRows::which::A;
			UINT trigNum ( 0 );
			bool safemode ( true );
			NiawgController niawg ( trigRow, trigNum, safemode );
			ScriptStream stream ( simpleScript );
			NiawgOutput output;
			//
			niawg.analyzeNiawgScript ( stream, output, profileSettings ( ), debugInfo ( ), std::string ( ), rerngGuiOptionsForm ( ),
									   std::vector<parameterType> ( ) );
			niawg.writeStaticNiawg ( output, debugInfo ( ), std::vector<parameterType> ( ), false, niawgLibOption::mode::banned );
			//
			auto& vals = output.waves[ 0 ].core.waveVals;
			// should be long enough to sum close to zero.
			double sum = 0;
			double max = 0, min = 0;
			for ( auto val : vals )
			{
				sum += val;
				if ( val > max )
				{
					max = val;
				}
				else if ( val < min )
				{
					min = val;
				}
			}
			// a couple sanity checks before the complete comparison.
			Assert::AreEqual ( 0.0, sum, 1e-9 );
			Assert::AreEqual ( 0.0, vals[ 0 ] );
			Assert::AreEqual ( 0.0, vals[ 1 ] );
			Assert::AreNotEqual ( vals[ 2 ], vals[ 3 ] );
			Assert::IsTrue ( max < 1.0 );
			Assert::IsTrue ( min > -1.0 );
			Assert::IsTrue ( vals.size ( ) == simpleScriptVals.size ( ) );
			for ( auto valInc : range ( vals.size ( ) ) )
			{
				Assert::AreEqual ( simpleScriptVals[ valInc ], vals[ valInc ], 1e-6 );
			}
		}
		TEST_METHOD ( RampScriptTests )
		{
			// check various properties of the wavevals and the wavevals themselves. Using actual calculations, not the
			// library.
			DioRows::which trigRow = DioRows::which::A;
			UINT trigNum ( 0 );
			bool safemode ( true );
			NiawgController niawg ( trigRow, trigNum, safemode );
			ScriptStream stream ( rampScript );
			NiawgOutput output;
			//
			niawg.analyzeNiawgScript ( stream, output, profileSettings ( ), debugInfo ( ), std::string ( ), rerngGuiOptionsForm ( ),
									   std::vector<parameterType> ( ) );
			niawg.writeStaticNiawg ( output, debugInfo ( ), std::vector<parameterType> ( ), false, niawgLibOption::mode::banned );
			//
			auto& vals = output.waves[ 0 ].core.waveVals;
			// should be long enough to sum close to zero.
			double sum = 0;
			double max = 0, min = 0;
			for ( auto val : vals )
			{
				sum += val;
				if ( val > max )
				{
					max = val;
				}
				else if ( val < min )
				{
					min = val;
				}
			}
			// a couple sanity checks before the complete comparison.
			// very unclear what an appropriate sum should be. really main thing is that the average should approach zero as the wave gets long.
			Assert::AreEqual ( 0.0, sum, 1 );
			Assert::AreEqual ( 0.0, vals[ 0 ] );
			Assert::AreEqual ( 0.0, vals[ 1 ] );
			Assert::AreNotEqual ( vals[ 2 ], vals[ 3 ] );
			Assert::IsTrue ( max < 1.0 );
			Assert::IsTrue ( min > -1.0 );
		}
		TEST_METHOD ( SimpleScript_LibRead_Wavevals )
		{
			// NOTE: this requires that the wave has already been written! to get the wave written, I often will change
			// the lib mode to "allowed", run the test once, and then re-run it with lib mode "forced".
			// check various properties of the wavevals and the wavevals themselves. Using actual calculations, not the
			// library.
			DioRows::which trigRow = DioRows::which::A;
			UINT trigNum ( 0 );
			bool safemode ( true );
			NiawgController niawg ( trigRow, trigNum, safemode );
			niawg.initialize ( );
			ScriptStream stream ( simpleScript );
			NiawgOutput output;
			niawg.analyzeNiawgScript ( stream, output, profileSettings ( ), debugInfo ( ), std::string ( ), 
									   rerngGuiOptionsForm ( ), std::vector<parameterType> ( ) );
			try
			{
				niawg.writeStaticNiawg ( output, debugInfo ( ), std::vector<parameterType> ( ), false,
										 niawgLibOption::mode::allowed );
			}
			catch ( Error& err )
			{
				errBox ( err.what() );
				throw;
			}
			//
			auto& vals = output.waves[ 0 ].core.waveVals;
			// should be long enough to sum close to zero.
			double sum = 0;
			double max = 0, min = 0;
			for ( auto val : vals )
			{
				sum += val;
				if ( val > max )
				{
					max = val;
				}
				else if ( val < min )
				{
					min = val;
				}
			}
			// a couple sanity checks before the complete comparison.
			Assert::AreEqual ( 0.0, sum, 1e-9 );
			Assert::AreEqual ( 0.0, vals[ 0 ] );
			Assert::AreEqual ( 0.0, vals[ 1 ] );
			Assert::AreNotEqual ( vals[ 2 ], vals[ 3 ] );
			Assert::IsTrue ( max < 1.0 );
			Assert::IsTrue ( min > -1.0 );
			Assert::IsTrue ( vals.size ( ) == simpleScriptVals.size ( ) );
			for ( auto valInc : range ( vals.size ( ) ) )
			{
				Assert::AreEqual ( simpleScriptVals[ valInc ], vals[ valInc ], 1e-6 );
			}
		}
		TEST_METHOD ( Large_Wave )
		{
			// there are potentially memory issues with writing long waves, so make sure the code can make it through 
			// writing a 10ms wave. This can take a while in debug mode, takes about 1s in release.
			const std::string longWaveScript = "gen1const HORIZONTAL 80 1 0 # gen1const VERTICAL 70 1 0 # 10 0";
			DioRows::which trigRow = DioRows::which::A;
			UINT trigNum ( 0 );
			bool safemode ( true );
			NiawgController niawg ( trigRow, trigNum, safemode );
			ScriptStream stream ( longWaveScript );
			NiawgOutput output;
			//
			niawg.analyzeNiawgScript ( stream, output, profileSettings ( ), debugInfo ( ), std::string ( ),
									   rerngGuiOptionsForm ( ), std::vector<parameterType> ( ) );
			niawg.writeStaticNiawg ( output, debugInfo ( ), std::vector<parameterType> ( ) );
			Assert::AreEqual ( long ( NIAWG_SAMPLE_RATE * 10e-3 ), output.waves[ 0 ].core.sampleNum() );
		}
		private:
		const std::string simpleScript =
			"gen1const "
			"HORIZONTAL "
			"80 1 0 # "
			"gen1const "
			"VERTICAL "
			"70 1 0 # "
			"0.01 0";
		const std::string simpleScript_v =
			"var_v 1 freqs_h [ 80 ] "
			"var_v 1 amps_h [ 1 ] "
			"var_v 1 phases_h [ 0 ] "
			"var_v 1 freqs_v [ 70 ] "
			"gen1const_v "
			"HORIZONTAL "
			"freqs_h amps_h phases_h # "
			"gen1const_v "
			"VERTICAL "
			"freqs_v amps_h phases_h # "
			"0.01 0";
		const std::string rampScript = 
			"gen2ampramp "
			"HORIZONTAL "
			"80 lin 0.3 0.7 0 "
			"80 lin 0.7 0.3 0 # "
			"gen1freqramp "
			"VERTICAL "
			"lin 70 80 1 0 "
			"# 0.01 0";
		const std::string rampScript_V =
			"var_v 2 freqs_h [ 80 80 ] "
			"var_v 2 PRT_h [ lin lin ] "
			"var_v 2 initAmps_h [ 0.3 0.7 ] "
			"var_v 2 finAmps_h [ 0.7 0.3 ] "
			"var_v 2 phases_h [ 0 0 ] "
			"var_v 1 FRT_v [ lin ] "
			"var_v 1 initFreqs [ 70 ] "
			"var_v 1 finFreqs [ 80 ] "
			"var_v 1 phases_v [ 0 ] "
			"var_v 1 amps_v [ 1 ] "
			"gen2ampramp_v "
			"HORIZONTAL "
			"freqs_h PRT_h initAmps_h finAmps_h phases_h # "
			"gen1freqramp_v "
			"VERTICAL "
			"FRT_v initFreqs finFreqs amps_v phases_v # "
			"0.01 0";
		const std::string dblRampScript =
			"gen3amp&freqramp "
			"HORIZONTAL "
			"lin 70 80 lin 0.5 1 0 "
			"lin 60 80 lin 1 0.1 0 "
			"lin 80 90 lin 0.5 2 0 #"
			"gen3amp&freqramp "
			"VERTICAL "
			"lin 75 85 lin 0 1 0 "
			"lin 60 60.1 lin 1 0.2 0 "
			"lin 30 90 lin 0.5 20 0 #"
			"0.01 0";

		const std::string dblRampScript_v =
			"var_v 3 FRT_h [ lin lin lin ] "
			"var_v 3 initFreqs_h [ 70 60 80 ] "
			"var_v 3 finFreqs_h [ 80 80 90 ] "
			"var_v 3 PRT_h [ lin lin lin ] "
			"var_v 3 initAmps_h [ 0.5 1 0.5 ] "
			"var_v 3 finAmps_h [ 1 0.1 2 ] "
			"var_v 3 phases_h [ 0 0 0 ] "

			"var_v 3 FRT_v [ lin lin lin ] "
			"var_v 3 initFreqs_v [ 75 60 30 ] "
			"var_v 3 finFreqs_v [ 85 60.1 90 ] "
			"var_v 3 PRT_v [ lin lin lin ] "
			"var_v 3 initAmps_v [ 0 1 0.5 ] "
			"var_v 3 finAmps_v [ 1 0.2 20 ] "
			"var_v 3 phases_v [ 0 0 0 ] "

			"gen3amp&freqramp_v "
			"HORIZONTAL "
			"FRT_h initFreqs_h finFreqs_h PRT_h initAmps_h finAmps_h phases_h #"
			"gen3amp&freqramp_v "
			"VERTICAL "
			"FRT_v initFreqs_v finFreqs_v PRT_v initAmps_v finAmps_v phases_v #"
			"0.01 0";


		const std::string complexLogicScript = "repeattiltrig "
			"gen1const HORIZONTAL 80 1 0 # "
			"gen1const VERTICAL 70 1 0 # 0.01 0"
			"endrepeat "
			"gen1const HORIZONTAL 80 1 0 # "
			"gen1const VERTICAL 70 1 0 # 0.01 0";

			// this vector was outputted as the solution by the code on September 8th 2018. 
		const std::vector<double> simpleScriptVals = {
			0, 0, 0.283127, 0.288673, 0.110471, 3.53523e-17, -0.240023, -0.288673, -0.204123, -7.07046e-17,
			0.160378, 0.288673, 0.266699, 1.06057e-16, -0.0563174, -0.288673, -0.288673, -1.41409e-16, -0.0563174, 0.288673,
			0.266699, 1.76761e-16, 0.160378, -0.288673, -0.204123, -2.12114e-16, -0.240023, 0.288673, 0.110471, 2.47466e-16,
			0.283127, -0.288673, 2.47466e-16, -2.82818e-16, -0.283127, 0.288673, -0.110471, 3.18171e-16, 0.240023, -0.288673,
			0.204123, -3.53523e-16, -0.160378, 0.288673, -0.266699, -6.36699e-16, 0.0563174, -0.288673, 0.288673, -4.24228e-16,
			0.0563174, 0.288673, -0.266699, -5.65994e-16, -0.160378, -0.288673, 0.204123, -4.94932e-16, 0.240023, 0.288673,
			-0.110471, -4.9529e-16, -0.283127, -0.288673, -4.94932e-16, -5.65637e-16, 0.283127, 0.288673, 0.110471, -4.24585e-16,
			-0.240023, -0.288673, -0.204123, -6.36341e-16, 0.160378, 0.288673, 0.266699, -3.5388e-16, -0.0563174, -0.288673,
			-0.288673, -7.07046e-16, -0.0563174, 0.288673, 0.266699, -2.83176e-16, 0.160378, -0.288673, -0.204123, 1.2734e-15,
			-0.240023, 0.288673, 0.110471, -2.26362e-15, 0.283127, -0.288673, -2.83176e-16, -8.48455e-16, -0.283127, 0.288673,
			-0.110471, -4.24406e-15, 0.240023, -0.288673, 0.204123, 1.13199e-15, -0.160378, 0.288673, -0.266699, -2.12221e-15,
			0.0563174, -0.288673, 0.288673, -9.89864e-16, 0.0563174, 0.288673, -0.266699, -4.10265e-15, -0.160378, -0.288673,
			0.204123, 9.90579e-16, 0.240023, 0.288673, -0.110471, -1.9808e-15, -0.283127, -0.288673, -9.89864e-16, -1.13127e-15,
			0.283127, 0.288673, 0.110471, -3.96124e-15, -0.240023, -0.288673, -0.204123, 8.4917e-16, 0.160378, 0.288673,
			0.266699, -1.83939e-15, -0.0563174, -0.288673, -0.288673, -1.27268e-15, -0.0563174, 0.288673, 0.266699, -3.81983e-15,
			0.160378, -0.288673, -0.204123, 7.07761e-16, -0.240023, 0.288673, 0.110471, -1.69798e-15, 0.283127, -0.288673,
			2.2629e-15, -1.41409e-15, -0.283127, 0.288673, -0.110471, 4.52617e-15, 0.240023, -0.288673, 0.204123, 5.66351e-16,
			-0.160378, 0.288673, -0.266699, -5.65887e-15, 0.0563174, -0.288673, 0.288673, 2.54679e-15, 0.0563174, 0.288673,
			-0.266699, 5.65279e-16, -0.160378, -0.288673, 0.204123, 4.52724e-15, 0.240023, 0.288673, -0.110471, -1.41516e-15,
			-0.283127, -0.288673, 5.66351e-16, -1.69691e-15, 0.283127, 0.288673, 0.110471, 4.80898e-15, -0.240023, -0.288673,
			-0.204123, 8.48812e-15, 0.160378, 0.288673, 0.266699, -5.37605e-15, -0.0563174, -0.288673, -0.288673, 2.26398e-15,
			-0.0563174, 0.288673, 0.266699, 8.48098e-16, 0.160378, -0.288673, -0.204123, 4.24442e-15, -0.240023, 0.288673,
			0.110471, -1.13235e-15, 0.283127, -0.288673, 4.80898e-15, -1.97973e-15, -0.283127, 0.288673, -0.110471, 5.0918e-15,
			0.240023, -0.288673, 0.204123, 8.20531e-15, -0.160378, 0.288673, -0.266699, -5.09323e-15, 0.0563174, -0.288673,
			0.288673, 1.98116e-15, 0.0563174, 0.288673, -0.266699, 1.13092e-15, -0.160378, -0.288673, 0.204123, 3.9616e-15,
			0.240023, 0.288673, -0.110471, -8.49527e-16, -0.283127, -0.288673, -1.97973e-15, -2.26255e-15, 0.283127, 0.288673,
			0.110471, 5.37462e-15, -0.240023, -0.288673, -0.204123, 7.92249e-15, 0.160378, 0.288673, 0.266699, -4.81041e-15,
			-0.0563174, -0.288673, -0.288673, 1.69834e-15, -0.0563174, 0.288673, 0.266699, 1.41373e-15, 0.160378, -0.288673,
			-0.204123, 3.67878e-15, -0.240023, 0.288673, 0.110471, -5.66709e-16, 0.283127, -0.288673, 7.35506e-15, -2.54537e-15,
			-0.283127, 0.288673, -0.110471, -2.54715e-15, 0.240023, -0.288673, 0.204123, 7.63967e-15, -0.160378, 0.288673,
			-0.266699, -4.5276e-15, 0.0563174, -0.288673, 0.288673, 1.41552e-15, 0.0563174, 0.288673, -0.266699, -6.50804e-15,
			-0.160378, -0.288673, 0.204123, 3.39596e-15, 0.240023, 0.288673, -0.110471, -8.48848e-15, -0.283127, -0.288673,
			-4.52581e-15, -2.82818e-15, 0.283127, 0.288673, 0.110471, -2.26433e-15, -0.240023, -0.288673, -0.204123, -9.05233e-15,
			0.160378, 0.288673, 0.266699, -4.24478e-15, -0.0563174, -0.288673, -0.288673, 1.1327e-15, -0.0563174, 0.288673,
			0.266699, 1.97937e-15, 0.160378, -0.288673, -0.204123, 1.13177e-14, -0.240023, 0.288673, 0.110471, 8.20352e-15,
			0.283127, -0.288673, -6.50804e-15, 5.09359e-15, -0.283127, 0.288673, -0.110471, -1.98152e-15, 0.240023, -0.288673,
			0.204123, -1.13056e-15, -0.160378, 0.288673, -0.266699, 4.24263e-15, 0.0563174, -0.288673, 0.288673, 9.05448e-15,
			0.0563174, 0.288673, -0.266699, -5.9424e-15, -0.160378, -0.288673, 0.204123, 2.83033e-15, 0.240023, 0.288673,
			-0.110471, 2.81746e-16, -0.283127, -0.288673, 1.1327e-15, -3.39382e-15, 0.283127, 0.288673, 0.110471, -9.90329e-15,
			-0.240023, -0.288673, -0.204123, -9.61797e-15, 0.160378, 0.288673, 0.266699, -3.67914e-15, -0.0563174, -0.288673,
			-0.288673, 1.69762e-14, -0.0563174, 0.288673, 0.266699, 2.54501e-15, 0.160378, -0.288673, -0.204123, 1.07521e-14,
			-0.240023, 0.288673, 0.110471, 8.76916e-15, 0.283127, -0.288673, 4.24263e-15, 4.52795e-15, -0.283127, 0.288673,
			-0.110471, -1.41588e-15, 0.240023, -0.288673, 0.204123, -1.6962e-15, -0.160378, 0.288673, -0.266699, 4.80827e-15,
			0.0563174, -0.288673, 0.288673, 8.48884e-15, 0.0563174, 0.288673, -0.266699, -5.37677e-15, -0.160378, -0.288673,
			0.204123, 2.26469e-15, 0.240023, 0.288673, -0.110471, 8.47383e-16, -0.283127, -0.288673, -9.61797e-15, -3.95946e-15,
			0.283127, 0.288673, 0.110471, -9.33765e-15, -0.240023, -0.288673, -0.204123, -1.01836e-14, 0.160378, 0.288673,
			0.266699, -3.1135e-15, -0.0563174, -0.288673, -0.288673, 1.64106e-14, -0.0563174, 0.288673, 0.266699, 3.11064e-15,
			0.160378, -0.288673, -0.204123, 1.01865e-14, -0.240023, 0.288673, 0.110471, 9.33479e-15, 0.283127, -0.288673,
			-1.41588e-15, 3.96232e-15, -0.283127, 0.288673, -0.110471, -8.50242e-16, 0.240023, -0.288673, 0.204123, -2.26183e-15,
			-0.160378, 0.288673, -0.266699, -1.10353e-14, 0.0563174, -0.288673, 0.288673, 7.9232e-15, 0.0563174, 0.288673,
			-0.266699, -4.81113e-15, -0.160378, -0.288673, 0.204123, 1.69905e-15, 0.240023, 0.288673, -0.110471, 1.41302e-15,
			-0.283127, -0.288673, -3.95946e-15, -4.52509e-15, 0.283127, 0.288673, 0.110471, -8.77202e-15, -0.240023, -0.288673,
			-0.204123, -1.07492e-14, 0.160378, 0.288673, 0.266699, -2.54787e-15, -0.0563174, -0.288673, -0.288673, 1.5845e-14,
			-0.0563174, 0.288673, 0.266699, 3.67628e-15, 0.160378, -0.288673, -0.204123, 9.62083e-15, -0.240023, 0.288673,
			0.110471, 9.90043e-15, 0.283127, -0.288673, -7.07439e-15, 3.39668e-15, -0.283127, 0.288673, -0.110471, -2.84605e-16,
			0.240023, -0.288673, 0.204123, -2.82747e-15, -0.160378, 0.288673, -0.266699, -1.04696e-14, 0.0563174, -0.288673,
			0.288673, 7.35757e-15, 0.0563174, 0.288673, -0.266699, -4.24549e-15, -0.160378, -0.288673, 0.204123, 1.13342e-15,
			0.240023, 0.288673, -0.110471, 1.97866e-15, -0.283127, -0.288673, -1.47101e-14, -5.09073e-15, 0.283127, 0.288673,
			0.110471, -8.20638e-15, -0.240023, -0.288673, -0.204123, 5.0943e-15, 0.160378, 0.288673, 0.266699, -1.98223e-15,
			-0.0563174, -0.288673, -0.288673, 1.52793e-14, -0.0563174, 0.288673, 0.266699, 4.24192e-15, 0.160378, -0.288673,
			-0.204123, 9.05519e-15, -0.240023, 0.288673, 0.110471, 1.04661e-14, 0.283127, -0.288673, 3.67628e-15, 2.83104e-15,
			-0.283127, 0.288673, -0.110471, 2.81032e-16, 0.240023, -0.288673, 0.204123, 1.30161e-14, -0.160378, 0.288673,
			-0.266699, -9.904e-15, 0.0563174, -0.288673, 0.288673, 6.79193e-15, 0.0563174, 0.288673, -0.266699, 1.27293e-14,
			-0.160378, -0.288673, 0.204123, 1.6977e-14, 0.240023, 0.288673, -0.110471, 2.54429e-15, -0.283127, -0.288673,
			-9.05162e-15, -5.65637e-15, 0.283127, 0.288673, 0.110471, 8.76844e-15, -0.240023, -0.288673, -0.204123, 4.52867e-15,
			0.160378, 0.288673, 0.266699, 1.49926e-14, -0.0563174, -0.288673, -0.288673, -1.81047e-14, -0.0563174, 0.288673,
			0.266699, -1.16016e-14, 0.160378, -0.288673, -0.204123, 8.48955e-15, -0.240023, 0.288673, 0.110471, -5.37748e-15,
			0.283127, -0.288673, 1.4427e-14, 2.26541e-15, -0.283127, 0.288673, -0.110471, 8.46668e-16, 0.240023, -0.288673,
			0.204123, -3.95874e-15, -0.160378, 0.288673, -0.266699, 7.07082e-15, 0.0563174, -0.288673, 0.288673, 2.26355e-14,
			0.0563174, 0.288673, -0.266699, -1.95234e-14, -0.160378, -0.288673, 0.204123, -1.6407e-14, 0.240023, 0.288673,
			-0.110471, 1.95191e-14, -0.283127, -0.288673, 1.30161e-14, 1.01872e-14, 0.283127, 0.288673, 0.110471, -7.0751e-15,
			-0.240023, -0.288673, -0.204123, 3.96303e-15, 0.160378, 0.288673, 0.266699, -8.50957e-16, -0.0563174, -0.288673,
			-0.288673, -2.26112e-15, -0.0563174, 0.288673, 0.266699, 5.37319e-15, 0.160378, -0.288673, -0.204123, -8.48527e-15,
			-0.240023, 0.288673, 0.110471, -2.1221e-14, 0.283127, -0.288673, 8.76844e-15, 1.8109e-14, -0.283127, 0.288673,
			-0.110471, 1.78215e-14, 0.240023, -0.288673, 0.204123, 1.18848e-14, -0.160378, 0.288673, -0.266699, -8.77273e-15,
			0.0563174, -0.288673, 0.288673, 5.66066e-15, 0.0563174, 0.288673, -0.266699, -2.54858e-15, -0.160378, -0.288673,
			0.204123, -5.63493e-16, 0.240023, 0.288673, -0.110471, 3.67557e-15, -0.283127, -0.288673, 2.26541e-15, -6.78764e-15,
			0.283127, 0.288673, 0.110471, -2.29187e-14, -0.240023, -0.288673, -0.204123, 1.98066e-14, 0.160378, 0.288673,
			0.266699, 1.61239e-14, -0.0563174, -0.288673, -0.288673, -1.92359e-14, -0.0563174, 0.288673, 0.266699, -1.04704e-14,
			0.160378, -0.288673, -0.204123, 7.35828e-15, -0.240023, 0.288673, 0.110471, -4.24621e-15, 0.283127, -0.288673,
			-1.32993e-14, 3.39525e-14, -0.283127, 0.288673, -0.110471, 1.97794e-15, 0.240023, -0.288673, 0.204123, -5.09002e-15,
			-0.160378, 0.288673, -0.266699, 8.20209e-15, 0.0563174, -0.288673, 0.288673, 2.15042e-14, 0.0563174, 0.288673,
			-0.266699, -1.83921e-14, -0.160378, -0.288673, 0.204123, -1.75383e-14, 0.240023, 0.288673, -0.110471, 2.06504e-14,
			-0.283127, -0.288673, -8.48527e-15, 9.05591e-15, 0.283127, 0.288673, 0.110471, -5.94383e-15, -0.240023, -0.288673,
			-0.204123, 2.83176e-15, 0.160378, 0.288673, 0.266699, 2.80317e-16, -0.0563174, -0.288673, -0.288673, -3.39239e-15,
			-0.0563174, 0.288673, 0.266699, 6.50447e-15, 0.160378, -0.288673, -0.204123, -9.61654e-15, -0.240023, 0.288673,
			0.110471, -2.00898e-14, 0.283127, -0.288673, -2.54858e-15, 1.69777e-14, -0.283127, 0.288673, -0.110471, 1.89528e-14,
			0.240023, -0.288673, 0.204123, 1.07535e-14, -0.160378, 0.288673, -0.266699, -7.64146e-15, 0.0563174, -0.288673,
			0.288673, 4.52938e-15, 0.0563174, 0.288673, -0.266699, -1.41731e-15, -0.160378, -0.288673, 0.204123, -1.69477e-15,
			0.240023, 0.288673, -0.110471, 4.80684e-15, -0.283127, -0.288673, -1.92359e-14, -7.91891e-15, 0.283127, 0.288673,
			0.110471, -2.17874e-14, -0.240023, -0.288673, -0.204123, 1.86753e-14, 0.160378, 0.288673, 0.266699, -1.55632e-14,
			-0.0563174, -0.288673, -0.288673, -2.03672e-14, -0.0563174, 0.288673, 0.266699, -9.33908e-15, 0.160378, -0.288673,
			-0.204123, 6.22701e-15, -0.240023, 0.288673, 0.110471, -3.11493e-15, 0.283127, -0.288673, -2.46163e-14, 3.28212e-14,
			-0.283127, 0.288673, -0.110471, 3.10922e-15, 0.240023, -0.288673, 0.204123, -6.22129e-15, -0.160378, 0.288673,
			-0.266699, 9.33336e-15, 0.0563174, -0.288673, 0.288673, 2.03729e-14, 0.0563174, 0.288673, -0.266699, -1.72609e-14,
			-0.160378, -0.288673, 0.204123, -1.86696e-14, 0.240023, 0.288673, -0.110471, -1.10367e-14, -0.283127, -0.288673,
			2.83176e-15, 7.92463e-15, 0.283127, 0.288673, 0.110471, -4.81256e-15, -0.240023, -0.288673, -0.204123, 1.70048e-15,
			0.160378, 0.288673, 0.266699, 1.41159e-15, -0.0563174, -0.288673, -0.288673, -4.52366e-15, -0.0563174, 0.288673,
			0.266699, 7.63574e-15, 0.160378, -0.288673, -0.204123, 2.20706e-14, -0.240023, 0.288673, 0.110471, -1.89585e-14,
			0.283127, -0.288673, 1.89528e-14, 1.58464e-14, -0.283127, 0.288673, -0.110471, 2.0084e-14, 0.240023, -0.288673,
			0.204123, 9.62226e-15, -0.160378, 0.288673, -0.266699, -6.51018e-15, 0.0563174, -0.288673, 0.288673, 3.39811e-15,
			0.0563174, 0.288673, -0.266699, -2.86035e-16, -0.160378, -0.288673, 0.204123, -2.82604e-15, 0.240023, 0.288673,
			-0.110471, 5.93811e-15, -0.283127, -0.288673, -7.91891e-15, -9.05019e-15, 0.283127, 0.288673, 0.110471, -2.06561e-14,
			-0.240023, -0.288673, -0.204123, 1.7544e-14, 0.160378, 0.288673, 0.266699, -1.4432e-14, -0.0563174, -0.288673,
			-0.288673, -2.14985e-14, -0.0563174, 0.288673, 0.266699, -8.20781e-15, 0.160378, -0.288673, -0.204123, 5.09573e-15,
			-0.240023, 0.288673, 0.110471, -1.98366e-15, 0.283127, -0.288673, -3.11493e-15, 3.169e-14, -0.283127, 0.288673,
			-0.110471, 4.24049e-15, 0.240023, -0.288673, 0.204123, -7.35256e-15, -0.160378, 0.288673, -0.266699, 1.04646e-14,
			0.0563174, -0.288673, 0.288673, 1.92417e-14, 0.0563174, 0.288673, -0.266699, -1.61296e-14, -0.160378, -0.288673,
			0.204123, -1.98009e-14, 0.240023, 0.288673, -0.110471, -9.90543e-15, -0.283127, -0.288673, 1.41488e-14, 6.79336e-15,
			0.283127, 0.288673, 0.110471, -3.68128e-15, -0.240023, -0.288673, -0.204123, 5.6921e-16, 0.160378, 0.288673,
			0.266699, 2.54286e-15, -0.0563174, -0.288673, -0.288673, -5.65494e-15, -0.0563174, 0.288673, 0.266699, 8.76701e-15,
			0.160378, -0.288673, -0.204123, 2.09393e-14, -0.240023, 0.288673, 0.110471, -1.78272e-14, 0.283127, -0.288673,
			7.63574e-15, 1.47151e-14, -0.283127, 0.288673, -0.110471, 2.12153e-14, 0.240023, -0.288673, 0.204123, 8.49098e-15,
			-0.160378, 0.288673, -0.266699, -5.37891e-15, 0.0563174, -0.288673, 0.288673, 2.26684e-15, 0.0563174, 0.288673,
			-0.266699, -3.19731e-14, -0.160378, -0.288673, 0.204123, -3.95731e-15, 0.240023, 0.288673, -0.110471, 7.06939e-15,
			-0.283127, -0.288673, -2.94203e-14, -1.01815e-14, 0.283127, 0.288673, 0.110471, -1.95248e-14, -0.240023, -0.288673,
			-0.204123, 1.64128e-14, 0.160378, 0.288673, 0.266699, -1.33007e-14, -0.0563174, -0.288673, -0.288673, 1.01886e-14,
			-0.0563174, 0.288673, 0.266699, -7.07653e-15, 0.160378, -0.288673, -0.204123, 3.96446e-15, -0.240023, 0.288673,
			0.110471, -8.52386e-16, 0.283127, -0.288673, -1.4432e-14, 3.05587e-14, -0.283127, 0.288673, -0.110471, 5.37176e-15,
			0.240023, -0.288673, 0.204123, -8.48384e-15, -0.160378, 0.288673, -0.266699, 1.15959e-14, 0.0563174, -0.288673,
			0.288673, 1.81104e-14, 0.0563174, 0.288673, -0.266699, -1.49983e-14, -0.160378, -0.288673, 0.204123, -2.09321e-14,
			0.240023, 0.288673, -0.110471, -8.77416e-15, -0.283127, -0.288673, -7.35256e-15, 5.66209e-15, 0.283127, 0.288673,
			0.110471, -2.55001e-15, -0.240023, -0.288673, -0.204123, -5.62063e-16, 0.160378, 0.288673, 0.266699, 3.67414e-15,
			-0.0563174, -0.288673, -0.288673, 2.60322e-14, -0.0563174, 0.288673, 0.266699, 9.89829e-15, 0.160378, -0.288673,
			-0.204123, 1.9808e-14, -0.240023, 0.288673, 0.110471, 1.61224e-14, 0.283127, -0.288673, -3.68128e-15, 1.35839e-14,
			-0.283127, 0.288673, -0.110471, -1.04718e-14, 0.240023, -0.288673, 0.204123, -2.54587e-14, -0.160378, 0.288673,
			-0.266699, -4.24764e-15, 0.0563174, -0.288673, 0.288673, 3.39539e-14, 0.0563174, 0.288673, -0.266699, 1.97651e-15,
			-0.160378, -0.288673, 0.204123, -5.08859e-15, 0.240023, 0.288673, -0.110471, -2.46177e-14, -0.283127, -0.288673,
			-1.81032e-14, -1.13127e-14, 0.283127, 0.288673, 0.110471, -1.83936e-14, -0.240023, -0.288673, -0.204123, -1.75369e-14,
			0.160378, 0.288673, 0.266699, -1.21694e-14, -0.0563174, -0.288673, -0.288673, 9.05734e-15, -0.0563174, 0.288673,
			0.266699, 2.68731e-14, 0.160378, -0.288673, -0.204123, -2.99852e-14, -0.240023, 0.288673, 0.110471, 2.78887e-16,
			0.283127, -0.288673, 7.06939e-15, -3.62093e-14, -0.283127, 0.288673, -0.110471, 6.50304e-15, 0.240023, -0.288673,
			0.204123, 2.32033e-14, -0.160378, 0.288673, -0.266699, 1.27272e-14, 0.0563174, -0.288673, 0.288673, 1.69791e-14,
			0.0563174, 0.288673, -0.266699, -4.66854e-14, -0.160378, -0.288673, 0.204123, 1.0755e-14, 0.240023, 0.288673,
			-0.110471, -4.04613e-14, -0.283127, -0.288673, -2.88539e-14, 4.53081e-15, 0.283127, 0.288673, 0.110471, 3.13996e-14,
			-0.240023, -0.288673, -0.204123, -1.69334e-15, 0.160378, 0.288673, 0.266699, 3.76238e-14, -0.0563174, -0.288673,
			-0.288673, -7.91748e-15, -0.0563174, 0.288673, 0.266699, -2.17888e-14, 0.160378, -0.288673, -0.204123, -1.41416e-14,
			-0.240023, 0.288673, 0.110471, -1.55647e-14, 0.283127, -0.288673, -1.49983e-14, 4.5271e-14, -0.283127, 0.288673,
			-0.110471, -9.34051e-15, 0.240023, -0.288673, 0.204123, 3.90468e-14, -0.160378, 0.288673, -0.266699, -3.11636e-15,
			0.0563174, -0.288673, 0.288673, -3.28141e-14, 0.0563174, 0.288673, -0.266699, 3.10779e-15, -0.160378, -0.288673,
			0.204123, -3.90382e-14, 0.240023, 0.288673, -0.110471, 9.33193e-15, -0.283127, -0.288673, 2.60322e-14, 2.03744e-14,
			0.283127, 0.288673, 0.110471, 1.55561e-14, -0.240023, -0.288673, -0.204123, 1.41502e-14, 0.160378, 0.288673,
			0.266699, -4.38565e-14, -0.0563174, -0.288673, -0.288673, 7.92606e-15, -0.0563174, 0.288673, 0.266699, -3.76324e-14,
			0.160378, -0.288673, -0.204123, 1.70191e-15, -0.240023, 0.288673, 0.110471, 3.42285e-14, 0.283127, -0.288673,
			-4.24764e-15, -4.52223e-15, -0.283127, 0.288673, -0.110471, -2.51841e-14, 0.240023, -0.288673, 0.204123, -1.07464e-14,
			-0.160378, 0.288673, -0.266699, -1.89599e-14, 0.0563174, -0.288673, 0.288673, -1.69705e-14, 0.0563174, 0.288673,
			-0.266699, -1.27358e-14, -0.160378, -0.288673, 0.204123, 4.24421e-14, 0.240023, 0.288673, -0.110471, -6.51161e-15,
			-0.283127, -0.288673, -1.75369e-14, 3.62179e-14, 0.283127, 0.288673, 0.110471, -2.87464e-16, -0.240023, -0.288673,
			-0.204123, -3.5643e-14, 0.160378, 0.288673, 0.266699, 5.93668e-15, -0.0563174, -0.288673, -0.288673, 2.37696e-14,
			-0.0563174, 0.288673, 0.266699, 1.21608e-14, 0.160378, -0.288673, -0.204123, 1.75455e-14, -0.240023, 0.288673,
			0.110471, 1.8385e-14, 0.283127, -0.288673, 6.50304e-15, 1.13213e-14, -0.283127, 0.288673, -0.110471, -4.10276e-14,
			0.240023, -0.288673, 0.204123, 5.09716e-15, -0.160378, 0.288673, -0.266699, -3.48035e-14, 0.0563174, -0.288673,
			0.288673, -1.12699e-15, 0.0563174, 0.288673, -0.266699, 3.70574e-14, -0.160378, -0.288673, 0.204123, -7.35113e-15,
			0.240023, 0.288673, -0.110471, -2.23552e-14, -0.283127, -0.288673, 4.53081e-15, -1.35753e-14, 0.283127, 0.288673,
			0.110471, -1.6131e-14, -0.240023, -0.288673, -0.204123, 4.58373e-14, 0.160378, 0.288673, 0.266699, -9.90686e-15,
			-0.0563174, -0.288673, -0.288673, 3.96132e-14, -0.0563174, 0.288673, 0.266699, -3.68271e-15, 0.160378, -0.288673,
			-0.204123, -3.22477e-14, -0.240023, 0.288673, 0.110471, 2.54143e-15, 0.283127, -0.288673, 5.00721e-14, -3.84719e-14,
			-0.283127, 0.288673, -0.110471, 8.76558e-15, 0.240023, -0.288673, 0.204123, 2.09407e-14, -0.160378, 0.288673,
			-0.266699, 1.49897e-14, 0.0563174, -0.288673, 0.288673, 1.47166e-14, 0.0563174, 0.288673, -0.266699, -4.44229e-14,
			-0.160378, -0.288673, 0.204123, 8.49241e-15, 0.240023, 0.288673, -0.110471, -3.81987e-14, -0.283127, -0.288673,
			2.65985e-14, 6.7905e-14, 0.283127, 0.288673, 0.110471, 3.36622e-14, -0.240023, -0.288673, -0.204123, -3.95588e-15,
			0.160378, 0.288673, 0.266699, 3.98863e-14, -0.0563174, -0.288673, -0.288673, -1.018e-14, -0.0563174, 0.288673,
			0.266699, -1.95263e-14, 0.160378, -0.288673, -0.204123, -1.64042e-14, -0.240023, 0.288673, 0.110471, -1.33021e-14,
			0.283127, -0.288673, -3.76324e-14, 4.30084e-14, -0.283127, 0.288673, -0.110471, -7.07796e-15, 0.240023, -0.288673,
			0.204123, 3.67843e-14, -0.160378, 0.288673, -0.266699, -8.53816e-16, 0.0563174, -0.288673, 0.288673, -3.50766e-14,
			0.0563174, 0.288673, -0.266699, 5.37033e-15, -0.160378, -0.288673, 0.204123, -4.13008e-14, 0.240023, 0.288673,
			-0.110471, 1.15945e-14, -0.283127, -0.288673, -1.69705e-14, 1.81118e-14, 0.283127, 0.288673, 0.110471, 1.78186e-14,
			-0.240023, -0.288673, -0.204123, 1.18877e-14, 0.160378, 0.288673, 0.266699, -4.1594e-14, -0.0563174, -0.288673,
			-0.288673, 5.66351e-15, -0.0563174, 0.288673, 0.266699, -3.53698e-14, 0.160378, -0.288673, -0.204123, -5.60634e-16,
			-0.240023, 0.288673, 0.110471, 3.64911e-14, 0.283127, -0.288673, 5.93668e-15, -6.78478e-15, -0.283127, 0.288673,
			-0.110471, -2.29215e-14, 0.240023, -0.288673, 0.204123, -1.30089e-14, -0.160378, 0.288673, -0.266699, -1.66974e-14,
			0.0563174, -0.288673, 0.288673, -1.92331e-14, 0.0563174, 0.288673, -0.266699, -1.04732e-14, -0.160378, -0.288673,
			0.204123, 4.01795e-14, 0.240023, 0.288673, -0.110471, -4.24907e-15, -0.283127, -0.288673, 5.09716e-15, 3.39554e-14,
			0.283127, 0.288673, 0.110471, 1.97508e-15, -0.240023, -0.288673, -0.204123, -3.79055e-14, 0.160378, 0.288673,
			0.266699, 8.19923e-15, -0.0563174, -0.288673, -0.288673, 2.15071e-14, -0.0563174, 0.288673, 0.266699, 1.44234e-14,
			0.160378, -0.288673, -0.204123, 1.52829e-14, -0.240023, 0.288673, 0.110471, -4.49892e-14, 0.283127, -0.288673,
			-1.6131e-14, 9.05876e-15, -0.283127, 0.288673, -0.110471, -3.87651e-14, 0.240023, -0.288673, 0.204123, 2.83462e-15,
			-0.160378, 0.288673, -0.266699, -3.25409e-14, 0.0563174, -0.288673, 0.288673, -3.38953e-15, 0.0563174, 0.288673,
			-0.266699, 3.932e-14, -0.160378, -0.288673, 0.204123, -9.61368e-15, 0.240023, 0.288673, -0.110471, -2.00926e-14,
			-0.283127, -0.288673, -3.84719e-14, -1.58378e-14, 0.283127, 0.288673, 0.110471, -1.38685e-14, -0.240023, -0.288673,
			-0.204123, 4.35748e-14, 0.160378, 0.288673, 0.266699, -7.64432e-15, -0.0563174, -0.288673, -0.288673, 3.73506e-14,
			-0.0563174, 0.288673, 0.266699, -6.70569e-14, 0.160378, -0.288673, -0.204123, 3.11265e-14, -0.240023, 0.288673,
			0.110471, 4.80398e-15, 0.283127, -0.288673, 2.7438e-14, -4.07344e-14, -0.283127, 0.288673, -0.110471, 1.10281e-14,
			0.240023, -0.288673, 0.204123, 1.86782e-14, -0.160378, 0.288673, -0.266699, 1.72523e-14, 0.0563174, -0.288673,
			0.288673, 1.2454e-14, 0.0563174, 0.288673, -0.266699, -4.21603e-14, -0.160378, -0.288673, 0.204123, 6.22987e-15,
			0.240023, 0.288673, -0.110471, -3.59362e-14, -0.283127, -0.288673, 4.92326e-14, 6.56425e-14, 0.283127, 0.288673,
			0.110471, 3.59247e-14, -0.240023, -0.288673, -0.204123, -6.21843e-15, 0.160378, 0.288673, 0.266699, 4.21489e-14,
			-0.0563174, -0.288673, -0.288673, -1.24426e-14, -0.0563174, 0.288673, 0.266699, -1.72637e-14, 0.160378, -0.288673,
			-0.204123, -1.86667e-14, -0.240023, 0.288673, 0.110471, -1.10396e-14, 0.283127, -0.288673, 5.37033e-15, 4.07459e-14,
			-0.283127, 0.288673, -0.110471, -4.81542e-15, 0.240023, -0.288673, 0.204123, 3.45217e-14, -0.160378, 0.288673,
			-0.266699, -6.4228e-14, 0.0563174, -0.288673, 0.288673, -3.73392e-14, 0.0563174, 0.288673, -0.266699, 7.63288e-15,
			-0.160378, -0.288673, 0.204123, 2.20734e-14, 0.240023, 0.288673, -0.110471, 1.3857e-14, -0.283127, -0.288673,
			5.66351e-15, 1.58493e-14, 0.283127, 0.288673, 0.110471, 2.00812e-14, -0.240023, -0.288673, -0.204123, 9.62512e-15,
			0.160378, 0.288673, 0.266699, -3.93314e-14, -0.0563174, -0.288673, -0.288673, 3.40097e-15, -0.0563174, 0.288673,
			0.266699, -3.31073e-14, 0.160378, -0.288673, -0.204123, -2.82318e-15, -0.240023, 0.288673, 0.110471, 3.87536e-14,
			0.283127, -0.288673, -1.66974e-14, -9.04733e-15, -0.283127, 0.288673, -0.110471, -2.0659e-14, 0.240023, -0.288673,
			0.204123, -1.52715e-14, -0.160378, 0.288673, -0.266699, -1.44348e-14, 0.0563174, -0.288673, 0.288673, 4.41411e-14,
			0.0563174, 0.288673, -0.266699, -8.21067e-15, -0.160378, -0.288673, 0.204123, 3.7917e-14, 0.240023, 0.288673,
			-0.110471, -1.98652e-15, -0.283127, -0.288673, -3.79055e-14, 3.16928e-14, 0.283127, 0.288673, 0.110471, 4.23763e-15,
			-0.240023, -0.288673, -0.204123, -4.01681e-14, 0.160378, 0.288673, 0.266699, 1.04618e-14, -0.0563174, -0.288673,
			-0.288673, 1.92445e-14, -0.0563174, 0.288673, 0.266699, 1.66859e-14, 0.160378, -0.288673, -0.204123, 1.30204e-14,
			-0.240023, 0.288673, 0.110471, -4.27267e-14, 0.283127, -0.288673, 2.68717e-14, 6.79622e-15, -0.283127, 0.288673,
			-0.110471, -3.65025e-14, 0.240023, -0.288673, 0.204123, 5.72069e-16, -0.160378, 0.288673, -0.266699, -3.02784e-14,
			0.0563174, -0.288673, 0.288673, -5.65208e-15, 0.0563174, 0.288673, -0.266699, 4.15825e-14, -0.160378, -0.288673,
			0.204123, -1.18762e-14, 0.240023, 0.288673, -0.110471, -1.78301e-14, -0.283127, -0.288673, -1.58378e-14, -1.81004e-14,
			0.283127, 0.288673, 0.110471, -1.16059e-14, -0.240023, -0.288673, -0.204123, 4.13122e-14, 0.160378, 0.288673,
			0.266699, -5.38177e-15, -0.0563174, -0.288673, -0.288673, 3.50881e-14, -0.0563174, 0.288673, 0.266699, -6.47944e-14,
			0.160378, -0.288673, -0.204123, 2.88639e-14, -0.240023, 0.288673, 0.110471, 7.06653e-15, 0.283127, -0.288673,
			4.80398e-15, -4.2997e-14, -0.283127, 0.288673, -0.110471, 1.32907e-14, 0.240023, -0.288673, 0.204123, 1.64156e-14,
			-0.160378, 0.288673, -0.266699, 1.95148e-14, 0.0563174, -0.288673, 0.288673, 1.01915e-14, 0.0563174, 0.288673,
			-0.266699, -3.98978e-14, -0.160378, -0.288673, 0.204123, 3.96732e-15, 0.240023, 0.288673, -0.110471, -3.36736e-14,
			-0.283127, -0.288673, 6.22987e-15, 6.33799e-14, 0.283127, 0.288673, 0.110471, 3.81873e-14, -0.240023, -0.288673,
			-0.204123, -8.48098e-15, 0.160378, 0.288673, 0.266699, -2.12253e-14, -0.0563174, -0.288673, -0.288673, -1.47051e-14,
			-0.0563174, 0.288673, 0.266699, -1.50012e-14, 0.160378, -0.288673, -0.204123, -2.09293e-14, -0.240023, 0.288673,
			0.110471, -8.77702e-15, 0.283127, -0.288673, -1.72637e-14, 3.84833e-14, -0.283127, 0.288673, -0.110471, -2.55287e-15,
			0.240023, -0.288673, 0.204123, 3.22592e-14, -0.160378, 0.288673, -0.266699, -6.19655e-14, 0.0563174, -0.288673,
			0.288673, -3.96017e-14, 0.0563174, 0.288673, -0.266699, 9.89543e-15, -0.160378, -0.288673, 0.204123, 1.98109e-14,
			0.240023, 0.288673, -0.110471, 1.61196e-14, -0.283127, -0.288673, 2.82976e-14, 1.35867e-14, 0.283127, 0.288673,
			0.110471, -4.3293e-14, -0.240023, -0.288673, -0.204123, 7.36257e-15, 0.160378, 0.288673, 0.266699, -3.70689e-14,
			-0.0563174, -0.288673, -0.288673, 1.13842e-15, -0.0563174, 0.288673, 0.266699, -3.08447e-14, 0.160378, -0.288673,
			-0.204123, -5.08573e-15, -0.240023, 0.288673, 0.110471, 4.10162e-14, 0.283127, -0.288673, 2.63053e-14, -1.13099e-14,
			-0.283127, 0.288673, -0.110471, -1.83964e-14, 0.240023, -0.288673, 0.204123, -1.7534e-14, -0.160378, 0.288673,
			-0.266699, -1.21723e-14, 0.0563174, -0.288673, 0.288673, 4.18786e-14, 0.0563174, 0.288673, -0.266699, -5.94812e-15,
			-0.160378, -0.288673, 0.204123, 3.56544e-14, 0.240023, 0.288673, -0.110471, 2.76028e-16, -0.283127, -0.288673,
			-1.52715e-14, 2.94303e-14, 0.283127, 0.288673, 0.110471, 6.50018e-15, -0.240023, -0.288673, -0.204123, -4.24306e-14,
			0.160378, 0.288673, 0.266699, 1.27243e-14, -0.0563174, -0.288673, -0.288673, 1.6982e-14, -0.0563174, 0.288673,
			0.266699, 1.89485e-14, 0.160378, -0.288673, -0.204123, 1.07578e-14, -0.240023, 0.288673, 0.110471, -4.04641e-14,
			0.283127, -0.288673, 4.23763e-15, 4.53367e-15, -0.283127, 0.288673, -0.110471, -3.424e-14, 0.240023, -0.288673,
			0.204123, 6.39463e-14, -0.160378, 0.288673, -0.266699, -2.80158e-14, 0.0563174, -0.288673, 0.288673, -7.91463e-15,
			0.0563174, 0.288673, -0.266699, 4.38451e-14, -0.160378, -0.288673, 0.204123, -1.41388e-14, 0.240023, 0.288673,
			-0.110471, -1.55675e-14, -0.283127, -0.288673, -5.88405e-14, -2.03629e-14, 0.283127, 0.288673, 0.110471, -9.34337e-15,
			-0.240023, -0.288673, -0.204123, 3.90497e-14, 0.160378, 0.288673, 0.266699, -3.11922e-15, -0.0563174, -0.288673,
			-0.288673, 3.28255e-14, -0.0563174, 0.288673, 0.266699, -6.25318e-14, 0.160378, -0.288673, -0.204123, 2.66014e-14,
			-0.240023, 0.288673, 0.110471, 9.32908e-15, 0.283127, -0.288673, -1.78301e-14, 2.03772e-14, -0.283127, 0.288673,
			-0.110471, 1.55532e-14, 0.240023, -0.288673, 0.204123, 1.41531e-14, -0.160378, 0.288673, -0.266699, 2.17774e-14,
			0.0563174, -0.288673, 0.288673, 7.92892e-15, 0.0563174, 0.288673, -0.266699, -3.76352e-14, -0.160378, -0.288673,
			0.204123, 1.70477e-15, 0.240023, 0.288673, -0.110471, -3.14111e-14, -0.283127, -0.288673, 2.88639e-14, 6.11174e-14,
			0.283127, 0.288673, 0.110471, 4.04498e-14, -0.240023, -0.288673, -0.204123, -1.07435e-14, 0.160378, 0.288673,
			0.266699, -1.89628e-14, -0.0563174, -0.288673, -0.288673, -1.69677e-14, -0.0563174, 0.288673, 0.266699, -1.27386e-14,
			0.160378, -0.288673, -0.204123, -2.31918e-14, -0.240023, 0.288673, 0.110471, -6.51447e-15, 0.283127, -0.288673,
			2.5739e-14, 3.62208e-14, -0.283127, 0.288673, -0.110471, -2.90323e-16, 0.240023, -0.288673, 0.204123, 2.99966e-14,
			-0.160378, 0.288673, -0.266699, -5.97029e-14, 0.0563174, -0.288673, 0.288673, -4.18643e-14, 0.0563174, 0.288673,
			-0.266699, 1.2158e-14, -0.160378, -0.288673, 0.204123, 1.75483e-14, 0.240023, 0.288673, -0.110471, 1.83821e-14,
			-0.283127, -0.288673, -1.47051e-14, 1.13242e-14, 0.283127, 0.288673, 0.110471, -4.10305e-14, -0.240023, -0.288673,
			-0.204123, 5.10002e-15, 0.160378, 0.288673, 0.266699, 3.08304e-14, -0.0563174, -0.288673, -0.288673, -1.12413e-15,
			-0.0563174, 0.288673, 0.266699, -2.85822e-14, 0.160378, -0.288673, -0.204123, -7.34827e-15, -0.240023, 0.288673,
			0.110471, -2.2358e-14, 0.283127, -0.288673, -6.19655e-14, 5.20643e-14, -0.283127, 0.288673, -0.110471, 4.95029e-14,
			0.240023, -0.288673, 0.204123, -1.97966e-14, -0.160378, 0.288673, -0.266699, -9.90972e-15, 0.0563174, -0.288673,
			0.288673, 3.9616e-14, 0.0563174, 0.288673, -0.266699, -6.93223e-14, -0.160378, -0.288673, 0.204123, -3.22449e-14,
			0.240023, 0.288673, -0.110471, 2.53858e-15, -0.283127, -0.288673, 7.36257e-15, 2.71677e-14, 0.283127, 0.288673,
			0.110471, 8.76272e-15, -0.240023, -0.288673, -0.204123, 2.09436e-14, 0.160378, 0.288673, 0.266699, -5.06499e-14,
			-0.0563174, -0.288673, -0.288673, -5.09173e-14, -0.0563174, 0.288673, 0.266699, 2.1211e-14, 0.160378, -0.288673,
			-0.204123, 8.49527e-15, -0.240023, 0.288673, 0.110471, -3.82016e-14, 0.283127, -0.288673, -1.83964e-14, 6.79079e-14,
			-0.283127, 0.288673, -0.110471, 3.36593e-14, 0.240023, -0.288673, 0.204123, -3.95302e-15, -0.160378, 0.288673,
			-0.266699, -2.57533e-14, 0.0563174, -0.288673, 0.288673, -1.01772e-14, 0.0563174, 0.288673, -0.266699, -1.95291e-14,
			-0.160378, -0.288673, 0.204123, 4.92354e-14, 0.240023, 0.288673, -0.110471, 5.23318e-14, -0.283127, -0.288673,
			-3.62065e-14, -2.26255e-14, 0.283127, 0.288673, 0.110471, -7.08082e-15, -0.240023, -0.288673, -0.204123, 3.67871e-14,
			0.160378, 0.288673, 0.266699, -6.64934e-14, -0.0563174, -0.288673, -0.288673, -3.50738e-14, -0.0563174, 0.288673,
			0.266699, 5.36747e-15, 0.160378, -0.288673, -0.204123, 2.43388e-14, -0.240023, 0.288673, 0.110471, 1.15916e-14,
			0.283127, -0.288673, -4.04641e-14, 1.81147e-14, -0.283127, 0.288673, -0.110471, -4.7821e-14, 0.240023, -0.288673,
			0.204123, -5.37462e-14, -0.160378, 0.288673, -0.266699, 2.40399e-14, 0.0563174, -0.288673, 0.288673, -5.99704e-14,
			0.0563174, 0.288673, -0.266699, -3.53727e-14, -0.160378, -0.288673, 0.204123, -5.57775e-16, 0.240023, 0.288673,
			-0.110471, -9.47853e-14, -0.283127, -0.288673, -1.41388e-14, -7.24187e-14, 0.283127, 0.288673, 0.110471, -2.29244e-14,
			-0.240023, -0.288673, -0.204123, -1.30061e-14, 0.160378, 0.288673, 0.266699, -8.2337e-14, -0.0563174, -0.288673,
			-0.288673, 4.64065e-14, -0.0563174, 0.288673, 0.266699, -1.04761e-14, 0.160378, -0.288673, -0.204123, -2.54544e-14,
			-0.240023, 0.288673, 0.110471, 6.13848e-14, 0.283127, -0.288673, 3.10493e-15, 3.39582e-14, -0.283127, 0.288673,
			-0.110471, 1.97222e-15, 0.240023, -0.288673, 0.204123, 9.33708e-14, -0.160378, 0.288673, -0.266699, 7.38331e-14,
			0.0563174, -0.288673, 0.288673, 2.15099e-14, 0.0563174, 0.288673, -0.266699, 1.44205e-14, -0.160378, -0.288673,
			0.204123, 8.09225e-14, 0.240023, 0.288673, -0.110471, -4.49921e-14, -0.283127, -0.288673, -5.77078e-14, 9.06162e-15,
			0.283127, 0.288673, 0.110471, 2.68688e-14, -0.240023, -0.288673, -0.204123, -6.27993e-14, 0.160378, 0.288673,
			0.266699, -3.25438e-14, -0.0563174, -0.288673, -0.288673, -3.38667e-15, -0.0563174, 0.288673, 0.266699, -9.19564e-14,
			0.160378, -0.288673, -0.204123, -7.52476e-14, -0.240023, 0.288673, 0.110471, -2.00955e-14, 0.283127, -0.288673,
			-1.89628e-14, -1.5835e-14, -0.283127, 0.288673, -0.110471, -7.95081e-14, 0.240023, -0.288673, 0.204123, 4.35776e-14,
			-0.160378, 0.288673, -0.266699, -7.64717e-15, 0.0563174, -0.288673, 0.288673, -2.82833e-14, 0.0563174, 0.288673,
			-0.266699, 6.42137e-14, -0.160378, -0.288673, 0.204123, 3.11293e-14, 0.240023, 0.288673, -0.110471, 4.80112e-15,
			-0.283127, -0.288673, 2.99966e-14, 9.05419e-14, 0.283127, 0.288673, 0.110471, 7.6662e-14, -0.240023, -0.288673,
			-0.204123, 1.8681e-14, 0.160378, 0.288673, 0.266699, 1.72494e-14, -0.0563174, -0.288673, -0.288673, 7.80936e-14,
			-0.0563174, 0.288673, 0.266699, -4.21632e-14, 0.160378, -0.288673, -0.204123, 6.23273e-15, -0.240023, 0.288673,
			0.110471, 2.96977e-14, 0.283127, -0.288673, 2.46063e-14, -6.56282e-14, -0.283127, 0.288673, -0.110471, -2.97149e-14,
			0.240023, -0.288673, 0.204123, -6.21557e-15, -0.160378, 0.288673, -0.266699, -8.91275e-14, 0.0563174, -0.288673,
			0.288673, -7.80765e-14, 0.0563174, 0.288673, -0.266699, -1.72666e-14, -0.160378, -0.288673, 0.204123, -1.86639e-14,
			0.240023, 0.288673, -0.110471, -7.66792e-14, -0.283127, -0.288673, 5.20643e-14, 4.07487e-14, 0.283127, 0.288673,
			0.110471, -4.81828e-15, -0.240023, -0.288673, -0.204123, -3.11122e-14, 0.160378, 0.288673, 0.266699, 6.70426e-14,
			-0.0563174, -0.288673, -0.288673, 2.83004e-14, -0.0563174, 0.288673, 0.266699, 7.63002e-15, 0.160378, -0.288673,
			-0.204123, 8.7713e-14, -0.240023, 0.288673, 0.110471, 7.94909e-14, 0.283127, -0.288673, 6.81753e-14, 1.58521e-14,
			-0.283127, 0.288673, -0.110471, 2.00783e-14, 0.240023, -0.288673, 0.204123, 7.52647e-14, -0.160378, 0.288673,
			-0.266699, -3.93343e-14, 0.0563174, -0.288673, 0.288673, 3.40383e-15, 0.0563174, 0.288673, -0.266699, 3.25266e-14,
			-0.160378, -0.288673, 0.204123, -6.84571e-14, 0.240023, 0.288673, -0.110471, -2.6886e-14, -0.283127, -0.288673,
			8.49527e-15, -9.04447e-15, 0.283127, 0.288673, 0.110471, -8.62986e-14, -0.240023, -0.288673, -0.204123, 5.03681e-14,
			0.160378, 0.288673, 0.266699, -1.44377e-14, -0.0563174, -0.288673, -0.288673, -2.14928e-14, -0.0563174, 0.288673,
			0.266699, -7.38503e-14, 0.160378, -0.288673, -0.204123, 3.79198e-14, -0.240023, 0.288673, 0.110471, -1.98938e-15,
			0.283127, -0.288673, 4.61076e-14, -3.39411e-14, -0.283127, 0.288673, -0.110471, 6.98715e-14, 0.240023, -0.288673,
			0.204123, 2.54715e-14, -0.160378, 0.288673, -0.266699, 1.04589e-14, 0.0563174, -0.288673, 0.288673, 8.48841e-14,
			0.0563174, 0.288673, -0.266699, -4.89537e-14, -0.160378, -0.288673, 0.204123, 1.30232e-14, 0.240023, 0.288673,
			-0.110471, 2.29072e-14, -0.283127, -0.288673, -3.50738e-14, 7.24358e-14, 0.283127, 0.288673, 0.110471, -3.65054e-14,
			-0.240023, -0.288673, -0.204123, 5.74928e-16, 0.160378, 0.288673, 0.266699, 3.53555e-14, -0.0563174, -0.288673,
			-0.288673, -7.1286e-14, -0.0563174, 0.288673, 0.266699, -2.40571e-14, 0.160378, -0.288673, -0.204123, -1.18734e-14,
			-0.240023, 0.288673, 0.110471, -8.34697e-14, 0.283127, -0.288673, -4.15968e-14, 4.75392e-14, -0.283127, 0.288673,
			-0.110471, -1.16088e-14, 0.240023, -0.288673, 0.204123, -2.43217e-14, -0.160378, 0.288673, -0.266699, -7.10214e-14,
			0.0563174, -0.288673, 0.288673, 3.50909e-14, 0.0563174, 0.288673, -0.266699, 8.39521e-16, -0.160378, -0.288673,
			0.204123, -3.677e-14, 0.240023, 0.288673, -0.110471, 7.27004e-14, -0.283127, -0.288673, -1.30061e-14, 2.26426e-14,
			0.283127, 0.288673, 0.110471, 1.32878e-14, -0.240023, -0.288673, -0.204123, 8.20552e-14, 0.160378, 0.288673,
			0.266699, -4.61248e-14, -0.0563174, -0.288673, -0.288673, 1.01943e-14, -0.0563174, 0.288673, 0.266699, 2.57361e-14,
			0.160378, -0.288673, -0.204123, 6.96069e-14, -0.240023, 0.288673, 0.110471, -3.36765e-14, 0.283127, -0.288673,
			1.97222e-15, -2.25397e-15, -0.283127, 0.288673, -0.110471, -9.30891e-14, 0.240023, -0.288673, 0.204123, -7.41149e-14,
			-0.160378, 0.288673, -0.266699, -2.12282e-14, 0.0563174, -0.288673, 0.288673, -1.47023e-14, 0.0563174, 0.288673,
			-0.266699, -8.06408e-14, -0.160378, -0.288673, 0.204123, 4.47103e-14, 0.240023, 0.288673, -0.110471, -8.77988e-15,
			-0.283127, -0.288673, 9.06162e-15, -2.71506e-14, 0.283127, 0.288673, 0.110471, -6.81925e-14, -0.240023, -0.288673,
			-0.204123, 3.2262e-14, 0.160378, 0.288673, 0.266699, 3.66842e-15, -0.0563174, -0.288673, -0.288673, 9.16746e-14,
			-0.0563174, 0.288673, 0.266699, 7.55293e-14, 0.160378, -0.288673, -0.204123, 1.98137e-14, -0.240023, 0.288673,
			0.110471, 1.61167e-14, 0.283127, -0.288673, -2.00955e-14, 7.92263e-14, -0.283127, 0.288673, -0.110471, -4.32959e-14,
			0.240023, -0.288673, 0.204123, 7.36543e-15, -0.160378, 0.288673, -0.266699, 2.8565e-14, 0.0563174, -0.288673,
			0.288673, -6.44955e-14, 0.0563174, 0.288673, -0.266699, -3.08476e-14, -0.160378, -0.288673, 0.204123, -5.08287e-15,
			0.240023, 0.288673, -0.110471, -9.02602e-14, -0.283127, -0.288673, -1.00144e-13, -7.69437e-14, 0.283127, 0.288673,
			0.110471, -1.83993e-14, -0.240023, -0.288673, -0.204123, -1.75312e-14, 0.160378, 0.288673, 0.266699, -7.78119e-14,
			-0.0563174, -0.288673, -0.288673, 4.18814e-14, -0.0563174, 0.288673, 0.266699, -5.95098e-15, 0.160378, -0.288673,
			-0.204123, -2.99795e-14, -0.240023, 0.288673, 0.110471, 6.59099e-14, 0.283127, -0.288673, -4.21632e-14, 2.94331e-14,
			-0.283127, 0.288673, -0.110471, 6.49732e-15, 0.240023, -0.288673, 0.204123, 8.88457e-14, -0.160378, 0.288673,
			-0.266699, 7.83582e-14, 0.0563174, -0.288673, 0.288673, 1.69848e-14, 0.0563174, 0.288673, -0.266699, 1.89456e-14,
			-0.160378, -0.288673, 0.204123, 7.63974e-14, 0.240023, 0.288673, -0.110471, -4.0467e-14, -0.283127, -0.288673,
			5.3197e-14, 1.3581e-13, 0.283127, 0.288673, 0.110471, 3.13939e-14, -0.240023, -0.288673, -0.204123, -6.73244e-14,
			0.160378, 0.288673, 0.266699, -2.80187e-14, -0.0563174, -0.288673, -0.288673, -7.91177e-15, -0.0563174, 0.288673,
			0.266699, -8.74313e-14, 0.160378, -0.288673, -0.204123, -7.97726e-14, -0.240023, 0.288673, 0.110471, -1.55704e-14,
			0.283127, -0.288673, 6.70426e-14, -2.03601e-14, -0.283127, 0.288673, -0.110471, -7.4983e-14, 0.240023, -0.288673,
			0.204123, 3.90525e-14, -0.160378, 0.288673, -0.266699, -1.34396e-13, 0.0563174, -0.288673, 0.288673, -3.28084e-14,
			0.0563174, 0.288673, -0.266699, 6.87388e-14, -0.160378, -0.288673, 0.204123, 2.66042e-14, 0.240023, 0.288673,
			-0.110471, 9.32622e-15, -0.283127, -0.288673, 7.52647e-14, 8.60168e-14, 0.283127, 0.288673, 0.110471, 8.11871e-14,
			-0.240023, -0.288673, -0.204123, 1.41559e-14, 0.160378, 0.288673, 0.266699, 2.17745e-14, -0.0563174, -0.288673,
			-0.288673, 7.35685e-14, -0.0563174, 0.288673, 0.266699, -3.76381e-14, 0.160378, -0.288673, -0.204123, 1.70763e-15,
			-0.240023, 0.288673, 0.110471, 3.42228e-14, 0.283127, -0.288673, 4.49749e-14, -7.01532e-14, -0.283127, 0.288673,
			-0.110471, -2.51898e-14, 0.240023, -0.288673, 0.204123, -1.07407e-14, -0.160378, 0.288673, -0.266699, -8.46024e-14,
			0.0563174, -0.288673, 0.288673, -8.26015e-14, 0.0563174, 0.288673, -0.266699, -1.27415e-14, -0.160378, -0.288673,
			0.204123, -2.3189e-14, 0.240023, 0.288673, -0.110471, -7.21541e-14, -0.283127, -0.288673, -3.39411e-14, 3.62236e-14,
			0.283127, 0.288673, 0.110471, -2.93182e-16, -0.240023, -0.288673, -0.204123, -3.56373e-14, 0.160378, 0.288673,
			0.266699, 7.15677e-14, -0.0563174, -0.288673, -0.288673, 2.37753e-14, -0.0563174, 0.288673, 0.266699, 1.21551e-14,
			0.160378, -0.288673, -0.204123, 8.31879e-14, -0.240023, 0.288673, 0.110471, -4.72575e-14, 0.283127, -0.288673,
			2.29072e-14, 1.1327e-14, -0.283127, 0.288673, -0.110471, 2.46034e-14, 0.240023, -0.288673, 0.204123, 7.07396e-14,
			-0.160378, 0.288673, -0.266699, -3.48092e-14, 0.0563174, -0.288673, 0.288673, -1.12127e-15, 0.0563174, 0.288673,
			-0.266699, 3.70517e-14, -0.160378, -0.288673, 0.204123, -7.29821e-14, 0.240023, 0.288673, -0.110471, -2.23609e-14,
			-0.283127, -0.288673, -1.18734e-14, -1.35696e-14, 0.283127, 0.288673, 0.110471, -8.17735e-14, -0.240023, -0.288673,
			-0.204123, 4.5843e-14, 0.160378, 0.288673, 0.266699, -9.91258e-15, -0.0563174, -0.288673, -0.288673, -2.60179e-14,
			-0.0563174, 0.288673, 0.266699, -6.93252e-14, 0.160378, -0.288673, -0.204123, 3.33947e-14, -0.240023, 0.288673,
			0.110471, 2.53572e-15, 0.283127, -0.288673, 8.39521e-16, -3.84662e-14, -0.283127, 0.288673, -0.110471, 7.43966e-14,
			0.240023, -0.288673, 0.204123, 2.09464e-14, -0.160378, 0.288673, -0.266699, 1.4984e-14, 0.0563174, -0.288673,
			0.288673, 8.0359e-14, 0.0563174, 0.288673, -0.266699, -4.44286e-14, -0.160378, -0.288673, 0.204123, 8.49813e-15,
			0.240023, 0.288673, -0.110471, 2.74323e-14, -0.283127, -0.288673, 1.01943e-14, 6.79107e-14, 0.283127, 0.288673,
			0.110471, -3.19803e-14, -0.240023, -0.288673, -0.204123, -3.95017e-15, 0.160378, 0.288673, 0.266699, -9.13929e-14,
			-0.0563174, -0.288673, -0.288673, -7.5811e-14, -0.0563174, 0.288673, 0.266699, -1.9532e-14, 0.160378, -0.288673,
			-0.204123, -1.63985e-14, -0.240023, 0.288673, 0.110471, -7.89446e-14, 0.283127, -0.288673, -2.12282e-14, 4.30141e-14,
			-0.283127, 0.288673, -0.110471, -7.08368e-15, 0.240023, -0.288673, 0.204123, -2.88468e-14, -0.160378, 0.288673,
			-0.266699, -6.64963e-14, 0.0563174, -0.288673, 0.288673, 3.05658e-14, 0.0563174, 0.288673, -0.266699, 5.36461e-15,
			-0.160378, -0.288673, 0.204123, 8.99784e-14, 0.240023, 0.288673, -0.110471, 7.72255e-14, -0.283127, -0.288673,
			3.2262e-14, 1.81175e-14, 0.283127, 0.288673, 0.110471, 1.78129e-14, -0.240023, -0.288673, -0.204123, 7.75301e-14,
			0.160378, 0.288673, 0.266699, -4.15997e-14, -0.0563174, -0.288673, -0.288673, 5.66923e-15, -0.0563174, 0.288673,
			0.266699, 3.02612e-14, 0.160378, -0.288673, -0.204123, 6.50818e-14, -0.240023, 0.288673, 0.110471, -2.91514e-14,
			0.283127, -0.288673, -4.32959e-14, -6.77906e-15, -0.283127, 0.288673, -0.110471, -8.8564e-14, 0.240023, -0.288673,
			0.204123, -7.86399e-14, -0.160378, 0.288673, -0.266699, -1.67031e-14, 0.0563174, -0.288673, 0.288673, -1.92274e-14,
			0.0563174, 0.288673, -0.266699, -7.61157e-14, -0.160378, -0.288673, 0.204123, 4.01852e-14, 0.240023, 0.288673,
			-0.110471, -4.25478e-15, -0.283127, -0.288673, -7.69437e-14, -3.16757e-14, 0.283127, 0.288673, 0.110471, -6.36674e-14,
			-0.240023, -0.288673, -0.204123, 2.77369e-14, 0.160378, 0.288673, 0.266699, 8.19351e-15, -0.0563174, -0.288673,
			-0.288673, 8.71495e-14, -0.0563174, 0.288673, 0.266699, 8.00544e-14, 0.160378, -0.288673, -0.204123, 1.52886e-14,
			-0.240023, 0.288673, 0.110471, 2.06418e-14, 0.283127, -0.288673, -6.53636e-14, 7.47012e-14, -0.283127, 0.288673,
			-0.110471, -3.87708e-14, 0.240023, -0.288673, 0.204123, 1.34114e-13, -0.160378, 0.288673, -0.266699, 3.30901e-14,
			0.0563174, -0.288673, 0.288673, 6.22529e-14, 0.0563174, 0.288673, -0.266699, -2.63225e-14, -0.160378, -0.288673,
			0.204123, -9.60796e-15, 0.240023, 0.288673, -0.110471, -8.57351e-14, -0.283127, -0.288673, -5.48761e-14, -8.14688e-14,
			0.283127, 0.288673, 0.110471, -1.38742e-14, -0.240023, -0.288673, -0.204123, -2.20563e-14, 0.160378, 0.288673,
			0.266699, -7.32868e-14, -0.0563174, -0.288673, -0.288673, 3.73563e-14, -0.0563174, 0.288673, 0.266699, -1.32699e-13,
			0.160378, -0.288673, -0.204123, -3.45046e-14, -0.240023, 0.288673, 0.110471, 7.0435e-14, 0.283127, -0.288673,
			4.38422e-14, 2.4908e-14, -0.283127, 0.288673, -0.110471, 1.10224e-14, 0.240023, -0.288673, 0.204123, 8.43206e-14,
			-0.160378, 0.288673, -0.266699, 8.28833e-14, 0.0563174, -0.288673, 0.288673, 1.24597e-14, 0.0563174, 0.288673,
			-0.266699, 2.34707e-14, -0.160378, -0.288673, 0.204123, 7.18723e-14, 0.240023, 0.288673, -0.110471, -3.59419e-14,
			-0.283127, -0.288673, 9.84651e-14, 1.31285e-13, 0.283127, 0.288673, 0.110471, 3.5919e-14, -0.240023, -0.288673,
			-0.204123, -7.18494e-14, 0.160378, 0.288673, 0.266699, -2.34936e-14, -0.0563174, -0.288673, -0.288673, -1.24369e-14,
			-0.0563174, 0.288673, 0.266699, -8.29062e-14, 0.160378, -0.288673, -0.204123, -8.42977e-14, -0.240023, 0.288673,
			0.110471, -1.10453e-14, 0.283127, -0.288673, 2.17745e-14, -2.48852e-14, -0.283127, 0.288673, -0.110471, -7.04579e-14,
			0.240023, -0.288673, 0.204123, 3.45274e-14, -0.160378, 0.288673, -0.266699, -1.2987e-13, 0.0563174, -0.288673,
			0.288673, -3.73335e-14, 0.0563174, 0.288673, -0.266699, 7.32639e-14, -0.160378, -0.288673, 0.204123, 2.20791e-14,
			0.240023, 0.288673, -0.110471, 1.38513e-14, -0.283127, -0.288673, -1.07407e-14, 8.14917e-14, 0.283127, 0.288673,
			0.110471, -4.55613e-14, -0.240023, -0.288673, -0.204123, 9.63083e-15, 0.160378, 0.288673, 0.266699, 2.62996e-14,
			-0.0563174, -0.288673, -0.288673, 6.90434e-14, -0.0563174, 0.288673, 0.266699, -3.3113e-14, 0.160378, -0.288673,
			-0.204123, 1.28456e-13, -0.240023, 0.288673, 0.110471, 3.87479e-14, 0.283127, -0.288673, -2.93182e-16, -7.46783e-14,
			-0.283127, 0.288673, -0.110471, -2.06647e-14, 0.240023, -0.288673, 0.204123, -1.52658e-14, -0.160378, 0.288673,
			-0.266699, -8.00773e-14, 0.0563174, -0.288673, 0.288673, 4.41468e-14, 0.0563174, 0.288673, -0.266699, -8.21638e-15,
			-0.160378, -0.288673, 0.204123, -2.77141e-14, 0.240023, 0.288673, -0.110471, -6.7629e-14, -0.283127, -0.288673,
			1.1327e-14, 3.16985e-14, 0.283127, 0.288673, 0.110471, 4.23191e-15, -0.240023, -0.288673, -0.204123, -4.01624e-14,
			0.160378, 0.288673, 0.266699, 7.60928e-14, -0.0563174, -0.288673, -0.288673, 1.92502e-14, -0.0563174, 0.288673,
			0.266699, 1.66802e-14, 0.160378, -0.288673, -0.204123, 7.86628e-14, -0.240023, 0.288673, 0.110471, -4.27324e-14,
			0.283127, -0.288673, 1.08913e-13, 6.80194e-15, -0.283127, 0.288673, -0.110471, 2.91285e-14, 0.240023, -0.288673,
			0.204123, 6.62145e-14, -0.160378, 0.288673, -0.266699, -3.02841e-14, 0.0563174, -0.288673, 0.288673, -5.64636e-15,
			0.0563174, 0.288673, -0.266699, 4.15768e-14, -0.160378, -0.288673, 0.204123, -7.75072e-14, 0.240023, 0.288673,
			-0.110471, -1.78358e-14, -0.283127, -0.288673, 3.33947e-14, -1.80947e-14, 0.283127, 0.288673, 0.110471, -7.72484e-14,
			-0.240023, -0.288673, -0.204123, 4.13179e-14, 0.160378, 0.288673, 0.266699, -5.38749e-15, -0.0563174, -0.288673,
			-0.288673, -3.0543e-14, -0.0563174, 0.288673, 0.266699, -6.48001e-14, 0.160378, -0.288673, -0.204123, 2.88696e-14,
			-0.240023, 0.288673, 0.110471, 7.06081e-15, 0.283127, -0.288673, -4.44286e-14, 8.82822e-14, -0.283127, 0.288673,
			-0.110471, 7.89217e-14, 0.240023, -0.288673, 0.204123, 1.64213e-14, -0.160378, 0.288673, -0.266699, 1.95091e-14,
			0.0563174, -0.288673, 0.288673, 7.58339e-14, 0.0563174, 0.288673, -0.266699, -3.99035e-14, -0.160378, -0.288673,
			0.204123, 3.97304e-15, 0.240023, 0.288673, -0.110471, 3.19574e-14, -0.283127, -0.288673, -7.5811e-14, 6.33856e-14,
			0.283127, 0.288673, 0.110471, -2.74552e-14, -0.240023, -0.288673, -0.204123, -8.47526e-15, 0.160378, 0.288673,
			0.266699, -8.68678e-14, -0.0563174, -0.288673, -0.288673, -8.03361e-14, -0.0563174, 0.288673, 0.266699, -1.50069e-14,
			0.160378, -0.288673, -0.204123, -2.09236e-14, -0.240023, 0.288673, 0.110471, -7.44195e-14, 0.283127, -0.288673,
			-6.64963e-14, 3.8489e-14, -0.283127, 0.288673, -0.110471, -2.55859e-15, 0.240023, -0.288673, 0.204123, -3.33719e-14,
			-0.160378, 0.288673, -0.266699, -6.19712e-14, 0.0563174, -0.288673, 0.288673, 2.60407e-14, 0.0563174, 0.288673,
			-0.266699, 9.88971e-15, -0.160378, -0.288673, 0.204123, 8.54533e-14, 0.240023, 0.288673, -0.110471, 8.17506e-14,
			-0.283127, -0.288673, -5.37434e-14, 1.35924e-14, 0.283127, 0.288673, 0.110471, 2.2338e-14, -0.240023, -0.288673,
			-0.204123, 7.3005e-14, 0.160378, 0.288673, 0.266699, -3.70746e-14, -0.0563174, -0.288673, -0.288673, 1.14414e-15,
			-0.0563174, 0.288673, 0.266699, 3.47863e-14, 0.160378, -0.288673, -0.204123, 6.05567e-14, -0.240023, 0.288673,
			0.110471, -2.46263e-14, 0.283127, -0.288673, 4.27095e-14, -1.13042e-14, -0.283127, 0.288673, -0.110471, -8.40389e-14,
			0.240023, -0.288673, 0.204123, -8.3165e-14, -0.160378, 0.288673, -0.266699, -1.2178e-14, 0.0563174, -0.288673,
			0.288673, -2.37525e-14, 0.0563174, 0.288673, -0.266699, -7.15906e-14, -0.160378, -0.288673, 0.204123, 3.56601e-14,
			0.240023, 0.288673, -0.110471, -1.31003e-13, -0.283127, -0.288673, -3.16757e-14, -3.62008e-14, 0.283127, 0.288673,
			0.110471, -5.91423e-14, -0.240023, -0.288673, -0.204123, 2.32118e-14, 0.160378, 0.288673, 0.266699, 1.27186e-14,
			-0.0563174, -0.288673, -0.288673, 8.26244e-14, -0.0563174, 0.288673, 0.266699, 8.45795e-14, 0.160378, -0.288673,
			-0.204123, 1.07635e-14, -0.240023, 0.288673, 0.110471, 2.51669e-14, 0.283127, -0.288673, 2.06418e-14, 7.01761e-14,
			-0.283127, 0.288673, -0.110471, -3.42457e-14, 0.240023, -0.288673, 0.204123, 1.29589e-13, -0.160378, 0.288673,
			-0.266699, 3.76152e-14, 0.0563174, -0.288673, 0.288673, 5.77278e-14, 0.0563174, 0.288673, -0.266699, -2.17974e-14,
			-0.160378, -0.288673, 0.204123, -1.41331e-14, 0.240023, 0.288673, -0.110471, -8.121e-14, -0.283127, -0.288673,
			-9.60796e-15, -8.59939e-14, 0.283127, 0.288673, 0.110471, -9.34909e-15, -0.240023, -0.288673, -0.204123, -2.65814e-14,
			0.160378, 0.288673, 0.266699, -6.87617e-14, -0.0563174, -0.288673, -0.288673, 3.28312e-14, -0.0563174, 0.288673,
			0.266699, -1.28174e-13, 0.160378, -0.288673, -0.204123, -3.90296e-14, -0.240023, 0.288673, 0.110471, 7.49601e-14,
			0.283127, -0.288673, -1.42588e-15, 2.03829e-14, -0.283127, 0.288673, -0.110471, 1.55475e-14, 0.240023, -0.288673,
			0.204123, 7.97955e-14, -0.160378, 0.288673, -0.266699, 8.74084e-14, 0.0563174, -0.288673, 0.288673, 7.93464e-15,
			0.0563174, 0.288673, -0.266699, 2.79958e-14, -0.160378, -0.288673, 0.204123, 6.73472e-14, 0.240023, 0.288673,
			-0.110471, -3.14168e-14, -0.283127, -0.288673, 1.24597e-14, 1.2676e-13, 0.283127, 0.288673, 0.110471, 4.04441e-14,
			-0.240023, -0.288673, -0.204123, -7.63745e-14, 0.160378, 0.288673, 0.266699, -1.89685e-14, -0.0563174, -0.288673,
			-0.288673, -1.6962e-14, -0.0563174, 0.288673, 0.266699, -7.83811e-14, 0.160378, -0.288673, -0.204123, 4.24506e-14,
			-0.240023, 0.288673, 0.110471, -6.52019e-15, 0.283127, -0.288673, -2.34936e-14, -2.94103e-14, -0.283127, 0.288673,
			-0.110471, -6.59328e-14, 0.240023, -0.288673, 0.204123, 3.00023e-14, -0.160378, 0.288673, -0.266699, -1.25345e-13,
			0.0563174, -0.288673, 0.288673, -4.18585e-14, 0.0563174, 0.288673, -0.266699, 7.7789e-14, -0.160378, -0.288673,
			0.204123, 1.7554e-14, 0.240023, 0.288673, -0.110471, 1.83764e-14, -0.283127, -0.288673, 3.45274e-14, 7.69666e-14,
			0.283127, 0.288673, 0.110471, -4.10362e-14, -0.240023, -0.288673, -0.204123, 5.10574e-15, 0.160378, 0.288673,
			0.266699, 3.08247e-14, -0.0563174, -0.288673, -0.288673, 6.45183e-14, -0.0563174, 0.288673, 0.266699, -2.85879e-14,
			0.160378, -0.288673, -0.204123, 1.23931e-13, -0.240023, 0.288673, 0.110471, 4.3273e-14, 0.283127, -0.288673,
			8.57122e-14, -7.92034e-14, -0.283127, 0.288673, -0.110471, -1.61396e-14, 0.240023, -0.288673, 0.204123, -1.97909e-14,
			-0.160378, 0.288673, -0.266699, -7.55522e-14, 0.0563174, -0.288673, 0.288673, 3.96217e-14, 0.0563174, 0.288673,
			-0.266699, -3.69129e-15, -0.160378, -0.288673, 0.204123, -3.22391e-14, 0.240023, 0.288673, -0.110471, -6.31039e-14,
			-0.283127, -0.288673, 5.65951e-14, 2.71734e-14, 0.283127, 0.288673, 0.110471, 8.75701e-15, -0.240023, -0.288673,
			-0.204123, 8.6586e-14, 0.160378, 0.288673, 0.266699, 8.06179e-14, -0.0563174, -0.288673, -0.288673, 1.47251e-14,
			-0.0563174, 0.288673, 0.266699, 2.12053e-14, 0.160378, -0.288673, -0.204123, 7.41377e-14, -0.240023, 0.288673,
			0.110471, -3.82073e-14, 0.283127, -0.288673, 6.36445e-14, 2.27684e-15, -0.283127, 0.288673, -0.110471, 3.36536e-14,
			0.240023, -0.288673, 0.204123, 6.16894e-14, -0.160378, 0.288673, -0.266699, -2.5759e-14, 0.0563174, -0.288673,
			0.288673, -1.01715e-14, 0.0563174, 0.288673, -0.266699, -8.51716e-14, -0.160378, -0.288673, 0.204123, -8.20323e-14,
			0.240023, 0.288673, -0.110471, -1.33107e-14, -0.283127, -0.288673, -5.26106e-14, -2.26198e-14, 0.283127, 0.288673,
			0.110471, -7.27233e-14, -0.240023, -0.288673, -0.204123, 3.67928e-14, 0.160378, 0.288673, 0.266699, -8.62392e-16,
			-0.0563174, -0.288673, -0.288673, -3.5068e-14, -0.0563174, 0.288673, 0.266699, -6.0275e-14, 0.160378, -0.288673,
			-0.204123, 2.43445e-14, -0.240023, 0.288673, 0.110471, 1.15859e-14, 0.283127, -0.288673, -8.96967e-14, 8.37571e-14,
			-0.283127, 0.288673, -0.110471, 8.34468e-14, 0.240023, -0.288673, 0.204123, 1.18962e-14, -0.160378, 0.288673,
			-0.266699, 2.40342e-14, 0.0563174, -0.288673, 0.288673, 7.13088e-14, 0.0563174, 0.288673, -0.266699, -3.53784e-14,
			-0.160378, -0.288673, 0.204123, -5.52057e-16, 0.240023, 0.288673, -0.110471, 3.64825e-14, -0.283127, -0.288673,
			-3.0543e-14, 5.88605e-14, 0.283127, 0.288673, 0.110471, -2.29301e-14, -0.240023, -0.288673, -0.204123, -1.30004e-14,
			0.160378, 0.288673, 0.266699, -8.23427e-14, -0.0563174, -0.288673, -0.288673, -8.48612e-14, -0.0563174, 0.288673,
			0.266699, -1.04818e-14, 0.160378, -0.288673, -0.204123, -2.54486e-14, -0.240023, 0.288673, 0.110471, -6.98944e-14,
			0.283127, -0.288673, 1.95091e-14, 3.39639e-14, -0.283127, 0.288673, -0.110471, -1.29307e-13, 0.240023, -0.288673,
			0.204123, -3.78969e-14, -0.160378, 0.288673, -0.266699, -5.74461e-14, 0.0563174, -0.288673, 0.288673, 2.15156e-14,
			0.0563174, 0.288673, -0.266699, 1.44148e-14, -0.160378, -0.288673, 0.204123, 8.09282e-14, 0.240023, 0.288673,
			-0.110471, 8.62757e-14, -0.283127, -0.288673, -8.47526e-15, 9.06734e-15, 0.283127, 0.288673, 0.110471, 2.68631e-14,
			-0.240023, -0.288673, -0.204123, 6.84799e-14, 0.160378, 0.288673, 0.266699, -3.25495e-14, -0.0563174, -0.288673,
			-0.288673, 1.27893e-13, -0.0563174, 0.288673, 0.266699, 3.93114e-14, 0.160378, -0.288673, -0.204123, 5.60316e-14,
			-0.240023, 0.288673, 0.110471, -2.01012e-14, 0.283127, -0.288673, -2.55859e-15, -1.58293e-14, -0.283127, 0.288673,
			-0.110471, -7.95138e-14, 0.240023, -0.288673, 0.204123, -8.76901e-14, -0.160378, 0.288673, -0.266699, -7.65289e-15,
			0.0563174, -0.288673, 0.288673, -2.82775e-14, 0.0563174, 0.288673, -0.266699, -6.70655e-14, -0.160378, -0.288673,
			0.204123, 3.1135e-14, 0.240023, 0.288673, -0.110471, -1.26478e-13, -0.283127, -0.288673, -1.17681e-13, -4.07258e-14,
			0.283127, 0.288673, 0.110471, -5.46172e-14, -0.240023, -0.288673, -0.204123, 1.86867e-14, 0.160378, 0.288673,
			0.266699, 1.72437e-14, -0.0563174, -0.288673, -0.288673, 7.80993e-14, -0.0563174, 0.288673, 0.266699, 8.91046e-14,
			0.160378, -0.288673, -0.204123, 6.23844e-15, -0.240023, 0.288673, 0.110471, 2.9692e-14, 0.283127, -0.288673,
			-2.46263e-14, 6.5651e-14, -0.283127, 0.288673, -0.110471, -2.97206e-14, 0.240023, -0.288673, 0.204123, 1.25064e-13,
			-0.160378, 0.288673, -0.266699, 4.21403e-14, 0.0563174, -0.288673, 0.288673, 5.32027e-14, 0.0563174, 0.288673,
			-0.266699, -1.72723e-14, -0.160378, -0.288673, 0.204123, -1.86582e-14, 0.240023, 0.288673, -0.110471, -7.66849e-14,
			-0.283127, -0.288673, 3.56601e-14, 4.07544e-14, 0.283127, 0.288673, 0.110471, -4.82399e-15, -0.240023, -0.288673,
			-0.204123, -3.11064e-14, 0.160378, 0.288673, 0.266699, -6.42366e-14, -0.0563174, -0.288673, -0.288673, 2.83061e-14,
			-0.0563174, 0.288673, 0.266699, -1.23649e-13, 0.160378, -0.288673, -0.204123, -4.35547e-14, -0.240023, 0.288673,
			0.110471, -5.17883e-14, 0.283127, -0.288673, 8.45795e-14, 1.58578e-14, -0.283127, 0.288673, -0.110471, 2.00726e-14,
			0.240023, -0.288673, 0.204123, 7.52704e-14, -0.160378, 0.288673, -0.266699, -3.934e-14, 0.0563174, -0.288673,
			0.288673, 3.40954e-15, 0.0563174, 0.288673, -0.266699, 3.25209e-14, -0.160378, -0.288673, 0.204123, 6.28221e-14,
			0.240023, 0.288673, -0.110471, -2.68917e-14, -0.283127, -0.288673, 5.77278e-14, 1.22235e-13, 0.283127, 0.288673,
			0.110471, 4.49692e-14, -0.240023, -0.288673, -0.204123, -8.08996e-14, 0.160378, 0.288673, 0.266699, -1.44434e-14,
			-0.0563174, -0.288673, -0.288673, -2.1487e-14, -0.0563174, 0.288673, 0.266699, -7.3856e-14, 0.160378, -0.288673,
			-0.204123, 3.79255e-14, -0.240023, 0.288673, 0.110471, -1.9951e-15, 0.283127, -0.288673, 6.25118e-14, -3.39353e-14,
			-0.283127, 0.288673, -0.110471, -6.14077e-14, 0.240023, -0.288673, 0.204123, 2.54772e-14, -0.160378, 0.288673,
			-0.266699, -1.2082e-13, 0.0563174, -0.288673, 0.288673, -4.63836e-14, 0.0563174, 0.288673, -0.266699, 8.23141e-14,
			-0.160378, -0.288673, 0.204123, 1.30289e-14, 0.240023, 0.288673, -0.110471, 2.29015e-14, -0.283127, -0.288673,
			-5.14779e-14, 7.24415e-14, 0.283127, 0.288673, 0.110471, -3.65111e-14, -0.240023, -0.288673, -0.204123, 5.80646e-16,
			0.160378, 0.288673, 0.266699, 3.53498e-14, -0.0563174, -0.288673, -0.288673, 5.99932e-14, -0.0563174, 0.288673,
			0.266699, -2.40628e-14, 0.160378, -0.288673, -0.204123, 1.19406e-13, -0.240023, 0.288673, 0.110471, -8.34754e-14,
			0.283127, -0.288673, 4.04441e-14, -8.37285e-14, -0.283127, 0.288673, -0.110471, -1.16145e-14, 0.240023, -0.288673,
			0.204123, -2.43159e-14, -0.160378, 0.288673, -0.266699, -7.10271e-14, 0.0563174, -0.288673, 0.288673, 3.50966e-14,
			0.0563174, 0.288673, -0.266699, 8.33803e-16, -0.160378, -0.288673, 0.204123, -3.67642e-14, 0.240023, 0.288673,
			-0.110471, -5.85788e-14, -0.283127, -0.288673, -2.94103e-14, 2.26483e-14, 0.283127, 0.288673, 0.110471, -1.17991e-13,
			-0.240023, -0.288673, -0.204123, 8.20609e-14, 0.160378, 0.288673, 0.266699, 8.5143e-14, -0.0563174, -0.288673,
			-0.288673, 1.02e-14, -0.0563174, 0.288673, 0.266699, -1.05543e-13, 0.160378, -0.288673, -0.204123, -6.16608e-14,
			-0.240023, 0.288673, 0.110471, 9.75913e-14, 0.283127, -0.288673, 1.83764e-14, -2.24825e-15, -0.283127, 0.288673,
			-0.110471, 3.81787e-14, 0.240023, -0.288673, 0.204123, 5.71643e-14, -0.160378, 0.288673, -0.266699, -2.12339e-14,
			0.0563174, -0.288673, 0.288673, -1.46965e-14, 0.0563174, 0.288673, -0.266699, -8.06465e-14, -0.160378, -0.288673,
			0.204123, 4.4716e-14, 0.240023, 0.288673, -0.110471, -1.40059e-13, -0.283127, -0.288673, 1.23931e-13, 1.04129e-13,
			0.283127, 0.288673, 0.110471, 6.30753e-14, -0.240023, -0.288673, -0.204123, -9.90057e-14, 0.160378, 0.288673,
			0.266699, 3.6627e-15, -0.0563174, -0.288673, -0.288673, -3.95931e-14, -0.0563174, 0.288673, 0.266699, -5.57499e-14,
			0.160378, -0.288673, -0.204123, 1.98194e-14, -0.240023, 0.288673, 0.110471, 1.6111e-14, 0.283127, -0.288673,
			-3.69129e-15, 7.9232e-14, -0.283127, 0.288673, -0.110471, -4.33016e-14, 0.240023, -0.288673, 0.204123, 1.38645e-13,
			-0.160378, 0.288673, -0.266699, -1.02714e-13, 0.0563174, -0.288673, 0.288673, -6.44897e-14, 0.0563174, 0.288673,
			-0.266699, 1.0042e-13, -0.160378, -0.288673, 0.204123, -5.07715e-15, 0.240023, 0.288673, -0.110471, 4.10076e-14,
			-0.283127, -0.288673, 1.47251e-14, 5.43354e-14, 0.283127, 0.288673, 0.110471, -1.8405e-14, -0.240023, -0.288673,
			-0.204123, -1.75254e-14, 0.160378, 0.288673, 0.266699, -7.78176e-14, -0.0563174, -0.288673, -0.288673, 4.18871e-14,
			-0.0563174, 0.288673, 0.266699, -1.3723e-13, 0.160378, -0.288673, -0.204123, 1.013e-13, -0.240023, 0.288673,
			0.110471, 6.59042e-14, 0.283127, -0.288673, 1.05514e-13, -1.01835e-13, -0.283127, 0.288673, -0.110471, 6.4916e-15,
			0.240023, -0.288673, 0.204123, -4.2422e-14, -0.160378, 0.288673, -0.266699, -5.2921e-14, 0.0563174, -0.288673,
			0.288673, 1.69905e-14, 0.0563174, 0.288673, -0.266699, 1.89399e-14, -0.160378, -0.288673, 0.204123, 7.64031e-14,
			0.240023, 0.288673, -0.110471, -4.04727e-14, -0.283127, -0.288673, 3.67928e-14, 1.35816e-13, 0.283127, 0.288673,
			0.110471, -9.98853e-14, -0.240023, -0.288673, -0.204123, -6.73186e-14, 0.160378, 0.288673, 0.266699, 1.03249e-13,
			-0.0563174, -0.288673, -0.288673, -7.90605e-15, -0.0563174, 0.288673, 0.266699, 4.38365e-14, 0.160378, -0.288673,
			-0.204123, 5.15065e-14, -0.240023, 0.288673, 0.110471, -1.55761e-14, 0.283127, -0.288673, -4.78267e-14, -2.03543e-14,
			-0.283127, 0.288673, -0.110471, -7.49887e-14, 0.240023, -0.288673, 0.204123, 3.90582e-14, -0.160378, 0.288673,
			-0.266699, -1.34401e-13, 0.0563174, -0.288673, 0.288673, 9.84708e-14, 0.0563174, 0.288673, -0.266699, 6.87331e-14,
			-0.160378, -0.288673, 0.204123, -1.04664e-13, 0.240023, 0.288673, -0.110471, 9.3205e-15, -0.283127, -0.288673,
			-7.24129e-14, -4.52509e-14, 0.283127, 0.288673, 0.110471, -5.00921e-14, -0.240023, -0.288673, -0.204123, 1.41616e-14,
			0.160378, 0.288673, 0.266699, 2.17688e-14, -0.0563174, -0.288673, -0.288673, 7.35742e-14, -0.0563174, 0.288673,
			0.266699, -3.76438e-14, 0.160378, -0.288673, -0.204123, 1.32987e-13, -0.240023, 0.288673, 0.110471, -9.70564e-14,
			0.283127, -0.288673, 6.13791e-14, -7.01475e-14, -0.283127, 0.288673, -0.110471, 1.06078e-13, 0.240023, -0.288673,
			0.204123, -1.07349e-14, -0.160378, 0.288673, -0.266699, 4.66654e-14, 0.0563174, -0.288673, 0.288673, 4.86776e-14,
			0.0563174, 0.288673, -0.266699, -1.27472e-14, -0.160378, -0.288673, 0.204123, -2.31832e-14, 0.240023, 0.288673,
			-0.110471, -7.21598e-14, -0.283127, -0.288673, 8.09282e-14, 3.62293e-14, 0.283127, 0.288673, 0.110471, -1.31572e-13,
			-0.240023, -0.288673, -0.204123, 9.56419e-14, 0.160378, 0.288673, 0.266699, -1.90985e-13, -0.0563174, -0.288673,
			-0.288673, -1.07492e-13, -0.0563174, 0.288673, 0.266699, 1.21494e-14, 0.160378, -0.288673, -0.204123, -4.80798e-14,
			-0.240023, 0.288673, 0.110471, -4.72632e-14, 0.283127, -0.288673, 3.93114e-14, -1.19941e-13, -0.283127, 0.288673,
			-0.110471, 2.45977e-14, 0.240023, -0.288673, 0.204123, 7.07453e-14, -0.160378, 0.288673, -0.266699, -1.66088e-13,
			0.0563174, -0.288673, 0.288673, -1.11555e-15, 0.0563174, 0.288673, -0.266699, -9.42275e-14, -0.160378, -0.288673,
			0.204123, 1.89571e-13, 0.240023, 0.288673, -0.110471, -2.23666e-14, -0.283127, -0.288673, -2.82775e-14, -1.44837e-13,
			0.283127, 0.288673, 0.110471, 4.94943e-14, -0.240023, -0.288673, -0.204123, 4.58487e-14, 0.160378, 0.288673,
			0.266699, 1.21355e-13, -0.0563174, -0.288673, -0.288673, -2.60121e-14, -0.0563174, 0.288673, 0.266699, -6.93309e-14,
			0.160378, -0.288673, -0.204123, 1.64674e-13, -0.240023, 0.288673, 0.110471, 2.53e-15, 0.283127, -0.288673,
			-1.1403e-13, 9.2813e-14, -0.283127, 0.288673, -0.110471, -1.88156e-13, 0.240023, -0.288673, 0.204123, 2.09521e-14,
			-0.160378, 0.288673, -0.266699, 1.46252e-13, 0.0563174, -0.288673, 0.288673, -5.09087e-14, 0.0563174, 0.288673,
			-0.266699, -4.44343e-14, -0.160378, -0.288673, 0.204123, -1.2277e-13, 0.240023, 0.288673, -0.110471, 2.74266e-14,
			-0.283127, -0.288673, -6.20985e-15, 6.79164e-14, 0.283127, 0.288673, 0.110471, -1.63259e-13, -0.240023, -0.288673,
			-0.204123, -3.94445e-15, 0.160378, 0.288673, 0.266699, -9.13986e-14, -0.0563174, -0.288673, -0.288673, 1.86742e-13,
			-0.0563174, 0.288673, 0.266699, -1.95377e-14, 0.160378, -0.288673, -0.204123, -1.47666e-13, -0.240023, 0.288673,
			0.110471, 5.23232e-14, 0.283127, -0.288673, -4.82399e-15, 4.30198e-14, -0.283127, 0.288673, -0.110471, 1.24184e-13,
			0.240023, -0.288673, 0.204123, -2.8841e-14, -0.160378, 0.288673, -0.266699, -6.6502e-14, 0.0563174, -0.288673,
			0.288673, 1.61845e-13, 0.0563174, 0.288673, -0.266699, 5.3589e-15, -0.160378, -0.288673, 0.204123, 8.99841e-14,
			0.240023, 0.288673, -0.110471, -1.85327e-13, -0.283127, -0.288673, -1.15416e-13, 1.81232e-14, 0.283127, 0.288673,
			0.110471, 1.49081e-13, -0.240023, -0.288673, -0.204123, -5.37376e-14, 0.160378, 0.288673, 0.266699, -4.16054e-14,
			-0.0563174, -0.288673, -0.288673, -1.25599e-13, -0.0563174, 0.288673, 0.266699, 3.02555e-14, 0.160378, -0.288673,
			-0.204123, 6.50875e-14, -0.240023, 0.288673, 0.110471, -1.60431e-13, 0.283127, -0.288673, -2.68917e-14, -6.77335e-15,
			-0.283127, 0.288673, -0.110471, -8.85697e-14, 0.240023, -0.288673, 0.204123, 1.83913e-13, -0.160378, 0.288673,
			-0.266699, -1.67088e-14, 0.0563174, -0.288673, 0.288673, -1.50495e-13, 0.0563174, 0.288673, -0.266699, 5.51521e-14,
			-0.160378, -0.288673, 0.204123, 4.01909e-14, 0.240023, 0.288673, -0.110471, 1.27013e-13, -0.283127, -0.288673,
			3.79255e-14, -3.16699e-14, 0.283127, 0.288673, 0.110471, -6.36731e-14, -0.240023, -0.288673, -0.204123, 1.59016e-13,
			0.160378, 0.288673, 0.266699, 8.1878e-15, -0.0563174, -0.288673, -0.288673, 8.71552e-14, -0.0563174, 0.288673,
			0.266699, -1.82498e-13, 0.160378, -0.288673, -0.204123, 1.52943e-14, -0.240023, 0.288673, 0.110471, 1.5191e-13,
			0.283127, -0.288673, 8.23141e-14, -5.65665e-14, -0.283127, 0.288673, -0.110471, -3.87765e-14, 0.240023, -0.288673,
			0.204123, -1.28427e-13, -0.160378, 0.288673, -0.266699, 3.30844e-14, 0.0563174, -0.288673, 0.288673, 6.22586e-14,
			0.0563174, 0.288673, -0.266699, -1.57602e-13, -0.160378, -0.288673, 0.204123, -9.60224e-15, 0.240023, 0.288673,
			-0.110471, -8.57408e-14, -0.283127, -0.288673, 5.99932e-14, 1.81084e-13, 0.283127, 0.288673, 0.110471, -1.38799e-14,
			-0.240023, -0.288673, -0.204123, -1.53324e-13, 0.160378, 0.288673, 0.266699, 5.7981e-14, -0.0563174, -0.288673,
			-0.288673, 3.7362e-14, -0.0563174, 0.288673, 0.266699, 1.29842e-13, 0.160378, -0.288673, -0.204123, -3.44988e-14,
			-0.240023, 0.288673, 0.110471, -6.08442e-14, 0.283127, -0.288673, -7.10271e-14, 1.56187e-13, -0.283127, 0.288673,
			-0.110471, 1.10167e-14, 0.240023, -0.288673, 0.204123, 8.43263e-14, -0.160378, 0.288673, -0.266699, -1.79669e-13,
			0.0563174, -0.288673, 0.288673, 1.24655e-14, 0.0563174, 0.288673, -0.266699, 1.54738e-13, -0.160378, -0.288673,
			0.204123, -5.93954e-14, 0.240023, 0.288673, -0.110471, -3.59476e-14, -0.283127, -0.288673, -4.92125e-14, -1.31256e-13,
			0.283127, 0.288673, 0.110471, 3.59133e-14, -0.240023, -0.288673, -0.204123, 5.94297e-14, 0.160378, 0.288673,
			0.266699, -1.54773e-13, -0.0563174, -0.288673, -0.288673, -1.24311e-14, -0.0563174, 0.288673, 0.266699, -8.29119e-14,
			0.160378, -0.288673, -0.204123, 1.78255e-13, -0.240023, 0.288673, 0.110471, -1.1051e-14, 0.283127, -0.288673,
			1.69452e-13, -1.56153e-13, -0.283127, 0.288673, -0.110471, 6.08099e-14, 0.240023, -0.288673, 0.204123, 3.45331e-14,
			-0.160378, 0.288673, -0.266699, 1.32671e-13, 0.0563174, -0.288673, 0.288673, -3.73277e-14, 0.0563174, 0.288673,
			-0.266699, -5.80153e-14, -0.160378, -0.288673, 0.204123, 1.53358e-13, 0.240023, 0.288673, -0.110471, 1.38456e-14,
			-0.283127, -0.288673, 1.04129e-13, 8.14974e-14, 0.283127, 0.288673, 0.110471, -1.7684e-13, -0.240023, -0.288673,
			-0.204123, 9.63655e-15, 0.160378, 0.288673, 0.266699, 1.57567e-13, -0.0563174, -0.288673, -0.288673, -6.22243e-14,
			-0.0563174, 0.288673, 0.266699, -3.31187e-14, 0.160378, -0.288673, -0.204123, -1.34085e-13, -0.240023, 0.288673,
			0.110471, 3.87422e-14, 0.283127, -0.288673, 1.6111e-14, 5.66008e-14, -0.283127, 0.288673, -0.110471, -1.51944e-13,
			0.240023, -0.288673, 0.204123, -1.526e-14, -0.160378, 0.288673, -0.266699, -8.0083e-14, 0.0563174, -0.288673,
			0.288673, 1.75426e-13, 0.0563174, 0.288673, -0.266699, -8.2221e-15, -0.160378, -0.288673, 0.204123, -1.58982e-13,
			0.240023, 0.288673, -0.110471, 6.36388e-14, -0.283127, -0.288673, -1.36351e-13, 3.17042e-14, 0.283127, 0.288673,
			0.110471, 1.355e-13, -0.240023, -0.288673, -0.204123, -4.01566e-14, 0.160378, 0.288673, 0.266699, -5.51864e-14,
			-0.0563174, -0.288673, -0.288673, 1.50529e-13, -0.0563174, 0.288673, 0.266699, 1.66745e-14, 0.160378, -0.288673,
			-0.204123, 7.86685e-14, -0.240023, 0.288673, 0.110471, -1.74012e-13, 0.283127, -0.288673, -5.9567e-15, 6.80765e-15,
			-0.283127, 0.288673, -0.110471, 1.60396e-13, 0.240023, -0.288673, 0.204123, -6.50532e-14, -0.160378, 0.288673,
			-0.266699, -3.02898e-14, 0.0563174, -0.288673, 0.288673, -1.36914e-13, 0.0563174, 0.288673, -0.266699, 4.15711e-14,
			-0.160378, -0.288673, 0.204123, 5.37719e-14, 0.240023, 0.288673, -0.110471, -1.49115e-13, -0.283127, -0.288673,
			1.69905e-14, -1.80889e-14, 0.283127, 0.288673, 0.110471, -7.72541e-14, -0.240023, -0.288673, -0.204123, 1.72597e-13,
			0.160378, 0.288673, 0.266699, -5.3932e-15, -0.0563174, -0.288673, -0.288673, 1.00736e-13, -0.0563174, 0.288673,
			0.266699, 6.64677e-14, 0.160378, -0.288673, -0.204123, 2.88753e-14, -0.240023, 0.288673, 0.110471, 1.38329e-13,
			0.283127, -0.288673, 1.03249e-13, -4.29855e-14, -0.283127, 0.288673, -0.110471, -5.23575e-14, 0.240023, -0.288673,
			0.204123, 1.47701e-13, -0.160378, 0.288673, -0.266699, 1.95034e-14, 0.0563174, -0.288673, 0.288673, 7.58396e-14,
			0.0563174, 0.288673, -0.266699, -1.71183e-13, -0.160378, -0.288673, 0.204123, 3.97875e-15, 0.240023, 0.288673,
			-0.110471, -9.93218e-14, -0.283127, -0.288673, -9.22152e-14, -6.78821e-14, 0.283127, 0.288673, 0.110471, -2.74609e-14,
			-0.240023, -0.288673, -0.204123, -1.39743e-13, 0.160378, 0.288673, 0.266699, 4.44e-14, -0.0563174, -0.288673,
			-0.288673, 5.0943e-14, -0.0563174, 0.288673, 0.266699, -1.46286e-13, 0.160378, -0.288673, -0.204123, -2.09178e-14,
			-0.240023, 0.288673, 0.110471, -7.44252e-14, 0.283127, -0.288673, -5.00921e-14, 1.69768e-13, -0.283127, 0.288673,
			-0.110471, -2.56431e-15, 0.240023, -0.288673, 0.204123, 9.79073e-14, -0.160378, 0.288673, -0.266699, 6.92966e-14,
			0.0563174, -0.288673, 0.288673, 2.60464e-14, 0.0563174, 0.288673, -0.266699, 1.41157e-13, -0.160378, -0.288673,
			0.204123, -4.58144e-14, 0.240023, 0.288673, -0.110471, -4.95286e-14, -0.283127, -0.288673, -7.01475e-14, 1.44872e-13,
			0.283127, 0.288673, 0.110471, 2.23323e-14, -0.240023, -0.288673, -0.204123, 7.30107e-14, 0.160378, 0.288673,
			0.266699, -1.68354e-13, -0.0563174, -0.288673, -0.288673, 1.14986e-15, -0.0563174, 0.288673, 0.266699, -9.64929e-14,
			0.160378, -0.288673, -0.204123, -7.0711e-14, -0.240023, 0.288673, 0.110471, -2.4632e-14, 0.283127, -0.288673,
			5.91137e-14, -1.42572e-13, -0.283127, 0.288673, -0.110471, 4.72289e-14, 0.240023, -0.288673, 0.204123, 4.81141e-14,
			-0.160378, 0.288673, -0.266699, -1.43457e-13, 0.0563174, -0.288673, 0.288673, -2.37467e-14, 0.0563174, 0.288673,
			-0.266699, -7.15963e-14, -0.160378, -0.288673, 0.204123, 1.66939e-13, 0.240023, 0.288673, -0.110471, 2.64593e-16,
			-0.283127, -0.288673, 8.31936e-14, 9.50784e-14, 0.283127, 0.288673, 0.110471, 7.21255e-14, -0.240023, -0.288673,
			-0.204123, 2.32176e-14, 0.160378, 0.288673, 0.266699, 1.43986e-13, -0.0563174, -0.288673, -0.288673, -4.86433e-14,
			-0.0563174, 0.288673, 0.266699, -4.66997e-14, 0.160378, -0.288673, -0.204123, 1.42043e-13, -0.240023, 0.288673,
			0.110471, 2.51612e-14, 0.283127, -0.288673, -9.42275e-14, 7.01818e-14, -0.283127, 0.288673, -0.110471, -1.65525e-13,
			0.240023, -0.288673, 0.204123, -1.67904e-15, -0.160378, 0.288673, -0.266699, -9.3664e-14, 0.0563174, -0.288673,
			0.288673, -7.35399e-14, 0.0563174, 0.288673, -0.266699, -2.18031e-14, -0.160378, -0.288673, 0.204123, -1.45401e-13,
			0.240023, 0.288673, -0.110471, 5.00578e-14, -0.283127, -0.288673, -2.60121e-14, 4.52852e-14, 0.283127, 0.288673,
			0.110471, -1.40628e-13, -0.240023, -0.288673, -0.204123, -2.65756e-14, 0.160378, 0.288673, 0.266699, -6.87674e-14,
			-0.0563174, -0.288673, -0.288673, 1.6411e-13, -0.0563174, 0.288673, 0.266699, 3.09349e-15, 0.160378, -0.288673,
			-0.204123, 9.22495e-14, -0.240023, 0.288673, 0.110471, 7.49544e-14, 0.283127, -0.288673, 1.46252e-13, 2.03887e-14,
			-0.283127, 0.288673, -0.110471, 1.46815e-13, 0.240023, -0.288673, 0.204123, -5.14722e-14, -0.160378, 0.288673,
			-0.266699, -4.38708e-14, 0.0563174, -0.288673, 0.288673, 1.39214e-13, 0.0563174, 0.288673, -0.266699, 2.79901e-14,
			-0.160378, -0.288673, 0.204123, 6.73529e-14, 0.240023, 0.288673, -0.110471, -1.62696e-13, -0.283127, -0.288673,
			-3.94445e-15, -4.50794e-15, 0.283127, 0.288673, 0.110471, -9.08351e-14, -0.240023, -0.288673, -0.204123, 1.86178e-13,
			0.160378, 0.288673, 0.266699, -1.89742e-14, -0.0563174, -0.288673, -0.288673, -1.4823e-13, -0.0563174, 0.288673,
			0.266699, 5.28867e-14, 0.160378, -0.288673, -0.204123, 4.24563e-14, -0.240023, 0.288673, 0.110471, -1.37799e-13,
			0.283127, -0.288673, 1.24184e-13, -2.94045e-14, -0.283127, 0.288673, -0.110471, -6.59385e-14, 0.240023, -0.288673,
			0.204123, 1.61282e-13, -0.160378, 0.288673, -0.266699, 5.92239e-15, 0.0563174, -0.288673, 0.288673, 8.94206e-14,
			0.0563174, 0.288673, -0.266699, -1.84764e-13, -0.160378, -0.288673, 0.204123, 1.75598e-14, 0.240023, 0.288673,
			-0.110471, 1.49644e-13, -0.283127, -0.288673, 1.81232e-14, -5.43011e-14, 0.283127, 0.288673, 0.110471, -4.10419e-14,
			-0.240023, -0.288673, -0.204123, 1.36385e-13, 0.160378, 0.288673, 0.266699, 3.0819e-14, -0.0563174, -0.288673,
			-0.288673, 6.4524e-14, -0.0563174, 0.288673, 0.266699, -1.59867e-13, 0.160378, -0.288673, -0.204123, -7.33684e-15,
			-0.240023, 0.288673, 0.110471, -8.80062e-14, 0.283127, -0.288673, -1.60431e-13, 1.83349e-13, -0.283127, 0.288673,
			-0.110471, -1.61453e-14, 0.240023, -0.288673, 0.204123, -1.51059e-13, -0.160378, 0.288673, -0.266699, 5.57156e-14,
			0.0563174, -0.288673, 0.288673, 3.96274e-14, 0.0563174, 0.288673, -0.266699, 1.27576e-13, -0.160378, -0.288673,
			0.204123, -3.22334e-14, 0.240023, 0.288673, -0.110471, -6.31096e-14, -0.283127, -0.288673, 4.01909e-14, 1.58453e-13,
			0.283127, 0.288673, 0.110471, 8.75129e-15, -0.240023, -0.288673, -0.204123, 8.65917e-14, 0.160378, 0.288673,
			0.266699, -1.81935e-13, -0.0563174, -0.288673, -0.288673, 1.47309e-14, -0.0563174, 0.288673, 0.266699, 1.52473e-13,
			0.160378, -0.288673, -0.204123, -5.713e-14, -0.240023, 0.288673, 0.110471, -3.8213e-14, 0.283127, -0.288673,
			8.00487e-14, -1.28991e-13, -0.283127, 0.288673, -0.110471, 3.36479e-14, 0.240023, -0.288673, 0.204123, 6.16951e-14,
			-0.160378, 0.288673, -0.266699, -1.57038e-13, 0.0563174, -0.288673, 0.288673, -1.01657e-14, 0.0563174, 0.288673,
			-0.266699, -8.51773e-14, -0.160378, -0.288673, 0.204123, 1.8052e-13, 0.240023, 0.288673, -0.110471, -1.33164e-14,
			-0.283127, -0.288673, -2.00288e-13, -1.53887e-13, 0.283127, 0.288673, 0.110471, 5.85445e-14, -0.240023, -0.288673,
			-0.204123, 3.67986e-14, 0.160378, 0.288673, 0.266699, 1.30405e-13, -0.0563174, -0.288673, -0.288673, -3.50623e-14,
			-0.0563174, 0.288673, 0.266699, -6.02807e-14, 0.160378, -0.288673, -0.204123, 1.55624e-13, -0.240023, 0.288673,
			0.110471, 1.15802e-14, 0.283127, -0.288673, 5.7981e-14, 8.37628e-14, -0.283127, 0.288673, -0.110471, -1.79106e-13,
			0.240023, -0.288673, 0.204123, 1.1902e-14, -0.160378, 0.288673, -0.266699, 1.55302e-13, 0.0563174, -0.288673,
			0.288673, -5.99589e-14, 0.0563174, 0.288673, -0.266699, -3.53841e-14, -0.160378, -0.288673, 0.204123, -1.3182e-13,
			0.240023, 0.288673, -0.110471, 3.64768e-14, -0.283127, -0.288673, 8.43263e-14, 5.88662e-14, 0.283127, 0.288673,
			0.110471, -1.54209e-13, -0.240023, -0.288673, -0.204123, -1.29946e-14, 0.160378, 0.288673, 0.266699, -8.23484e-14,
			-0.0563174, -0.288673, -0.288673, 1.77691e-13, -0.0563174, 0.288673, 0.266699, -2.73034e-13, 0.160378, -0.288673,
			-0.204123, -1.56716e-13, -0.240023, 0.288673, 0.110471, 6.13734e-14, 0.283127, -0.288673, 3.59133e-14, 3.39697e-14,
			-0.283127, 0.288673, -0.110471, 1.33234e-13, 0.240023, -0.288673, 0.204123, -3.78912e-14, -0.160378, 0.288673,
			-0.266699, -5.74518e-14, 0.0563174, -0.288673, 0.288673, 1.52795e-13, 0.0563174, 0.288673, -0.266699, 1.44091e-14,
			-0.160378, -0.288673, 0.204123, 8.09339e-14, 0.240023, 0.288673, -0.110471, -1.76277e-13, -0.283127, -0.288673
		};
	};
}
