#pragma once
#include "python.h"
#include <string>
//#include "SMSTextingControl.h"

struct personInfo;

class EmbeddedPythonHandler
{
	public:
		// constructor is important.
		EmbeddedPythonHandler();
		// for full data analysis set.
		bool runDataAnalysis(std::string analysisType, std::string date, long runNumber,
			long accumulations, std::string completeName, std::vector<std::pair<int, int>> atomLocations);
		// for texting.
		bool sendText(personInfo person, std::string msg, std::string subject, std::string baseEmail,
			std::string password);
		// for a single python command.
		std::string run(std::string cmd);
		bool flush();
	private:
		PyObject* autoAnalysisModule;
		PyObject* singleAtomAnalysisFunction;
		PyObject* atomPairAnalysisFunction;
		PyObject* mainModule;
		PyObject* errorCatcher;
};