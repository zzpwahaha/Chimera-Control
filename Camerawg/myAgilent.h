#pragma once
#include <vector>
#include "myMath.h"
#include "Windows.h"
#include "constants.h"
#include "externals.h"
#include "NiawgController.h"
#include "boost/cast.hpp"
#include <algorithm>
#include "ScriptStream.h"

struct profileSettings;

/*
 * This Namespace includes all of my function handling for interacting withe agilent waveform generator. It includes:
 * The Segment Class
 * The IntensityWaveform Class
 * The agilentDefault function
 * The agilentErrorCheck function
 * The selectIntensityProfile function
 */
namespace myAgilent
{

	class Segment
	{
		public:
			Segment();
			~Segment();
			int assignSegType(int segTypeIn);
			int assignRampType(std::string rampTypeIn);
			int assignInitValue(double initValIn);
			int assignFinValue(double finValIn);
			int assignContinuationType(int contTypeIn);
			int assignTime(double timeIn);
			int	assignRepeatNum(int repeatNumIn);
			int returnSegmentType();
			long returnDataSize();
			int returnRepeatNum();
			int returnContinuationType();
			int assignVarNum(int vNumIn);
			int assignSegVarNames(std::vector<std::string> vNamesIn);
			int assignVarLocations(std::vector<int> varLocationsIn);
			int returnSegVarNamesSize();
			std::string returnVarName(int varNameIdentifier);
			int returnVarLocation(int varNameIdentifier);
			int assignDataVal(int dataNum, double val);
			double returnDataVal(long dataNum);
			int calcData();
			double returnTime();
		private:
			int segmentType;
			std::string rampType;
			int repeatNum;
			double initValue;
			double finValue;
			double time;
			// values such as repeat, repeat until trigger, no repeat, etc.
			int continuationType;
			std::vector<double> dataArray;
			int varNum;
			std::vector<std::string> segVarNames;
			std::vector<int> varLocations;
	};


	class IntensityWaveform
	{
		public:
			IntensityWaveform();
			~IntensityWaveform();
			int readIntoSegment(int segNum, ScriptStream& scriptName, std::vector<variable> singletons, profileSettings profile);
			int writeData(int SegNum);
			std::string compileAndReturnDataSendString(int segNum, int varNum, int totalSegNum);
			void compileSequenceString(int totalSegNum, int sequenceNum);
			std::string returnSequenceString();
			bool returnIsVaried();
			int replaceVarValues(std::string varName, double varValue);
			int convertPowersToVoltages();
			int normalizeVoltages();
			int calcMinMax();
			double returnMaxVolt();
			double returnMinVolt();
		private:
			std::vector<myAgilent::Segment> waveformSegments;
			double maxVolt;
			double minVolt;
			int segmentNum;
			std::string totalSequence;
			bool varies;
	};

	/*
	 * The agilentDefalut function restores the status of the Agilent to be outputting the default DC level (full depth traps). It returns an error code.
	 */
	int agilentDefault();
	
	/*
	]---- This function is used to analyze a given intensity file. It's used to analyze all of the basic intensity files listed in the sequence of 
	]- configurations, but also recursively to analyze nested intensity scripts.
	*/
	bool analyzeIntensityScript(ScriptStream& intensityFile, myAgilent::IntensityWaveform* intensityWaveformData, int& currentSegmentNumber, 
								std::vector<variable> singletons, profileSettings profile);
	/*
	 * The programIntensity function reads in the intensity script file, interprets it, creates the segments and sequences, and outputs them to the andor to be
	 * ready for usage. 
	 * 
	 */
	void programIntensity( int varNum, std::vector<variable> varNames, std::vector<std::vector<double> > varValues, bool& intensityVaried,
						   std::vector<myMath::minMaxDoublet>& minsAndMaxes, std::vector<std::fstream>& intensityFiles,
						   std::vector<variable> singletons, profileSettings profile );

	int agilentErrorCheck(long status, unsigned long vi);
	
	void setIntensity(int varNum, bool intensityIsVaried, std::vector<myMath::minMaxDoublet> intensityMinMax);

}
