// created by Mark O. Brown
#pragma once

#include "GeneralUtilityFunctions/my_str.h"
#include <string>
#include <vector>
#include <array>
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/1394/Basler1394InstantCamera.h>

// running in safemode means that the program doesn't actually try to connect to physical devices. Generally, it will 
// follow the exact coding logic, but at the exact point where it would normally communicate with a device, it will 
// instead simply skip this step. It might generate example data where useful / necessary (e.g. after querying a
// camera system for a picture). It can be used to build and debug other aspects of the program, such as the gui, 
// coding logic, etc.
 
#define MASTER_COMPUTER

#ifdef MASTER_COMPUTER
	constexpr bool DOFTDI_SAFEMODE = false;
	constexpr bool DDS_SAFEMODE = false;
	constexpr bool ANDOR_SAFEMODE = false;
	#ifdef _DEBUG
		constexpr bool PYTHON_SAFEMODE = true;
	#else
		constexpr bool  PYTHON_SAFEMODE = true;
	#endif
	constexpr bool DAQMX_SAFEMODE = false;
	constexpr bool ANALOG_OUT_SAFEMODE = false;

	constexpr auto CODE_ROOT = "C:\\Users\\Regal-Lab\\Code\\Chimera-Control";
	const std::string PLOT_FILES_SAVE_LOCATION = str (CODE_ROOT) + "\\Plotting";
	const std::string DATA_ANALYSIS_CODE_LOCATION = "C:\\Users\\Regal-Lab\\Code\\Data_Analysis_Control\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = str (CODE_ROOT) + "\\Default-Scripts\\";
	const std::string ACTUAL_CODE_FOLDER_PATH = str (CODE_ROOT) + "\\Chimera\\";
	const std::string PROFILES_PATH = str (CODE_ROOT) + "\\Profiles\\";
	const std::string DATA_SAVE_LOCATION = "J:\\Data Repository\\New Data Repository\\";
	const std::string MUSIC_LOCATION = str (CODE_ROOT) + "\\Final Fantasy VII - Victory Fanfare [HQ].mp3";
	const std::string FUNCTIONS_FOLDER_LOCATION = str (CODE_ROOT) + "\\Functions\\";
	const std::string MASTER_CONFIGURATION_FILE_ADDRESS (str (CODE_ROOT) + "\\Master-Configuration.txt");
	const std::string CAMERA_CAL_ROUTINE_ADDRESS = PROFILES_PATH + "Hotkey Experiments\\Camera";
	const std::string MOT_ROUTINES_ADDRESS = PROFILES_PATH + "Hotkey Experiments\\MOT";

	// location where wave data can be outputted for analyzing with another computer.
#endif
/// Random other Constants
constexpr double PI = 3.14159265358979323846264338327950288;

