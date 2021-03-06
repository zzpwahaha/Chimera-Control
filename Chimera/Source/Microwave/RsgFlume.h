#pragma once
#include "Microwave/microwaveSettings.h"
#include "GeneralFlumes/GpibFlume.h"

// this is a small wrapper around a gpib flume to introduce microwave-specific commands.
class RsgFlume : private GpibFlume
{
	public:
		RsgFlume (short deviceID, bool safemode);
		void initiailize ();
		std::string queryIdentity ();
		void setPmSettings ();
		void setFmSettings ();
		void programSingleSetting (microwaveListEntry setting, unsigned varNumber);
		void programList (std::vector<microwaveListEntry> list, unsigned varNum);
};