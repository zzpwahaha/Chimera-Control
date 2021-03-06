﻿========================================================================
#    **Chimera-Control Project**
========================================================================

## This readme is old and needs updating. Contact me.

## Contact
Please feel free to contact me at any time with questions, bug reports, feature requests, or general support or for advice. I'm happy to help the project be as useful as possible to others who wish to use it. Contact me at 	Mark.O.Brown@colorado.edu with any such questions.

## Version
This readme (but not the code) was last updated on November 17th 2018. (lightly pruned on 2/23/2020 - still needs rework)

# Installation
- Everything is 64 bit at this point.

## quick mandatory checklist:
- Install Visual studio 2019
- Download all my code & included packages.
- Install NI-FGEN & NI-DAQmx
- Install The Andor SOLIS SDK
- set up a series of definitions in the "constants.h" file.

## Download my code
- The latest version of my code can always be found here: https://github.com/omarkb93/Chimera-Control
- You want to download the source code (Make sure to download the **most recent** commit, e.g. by using git clone on the repo or downloading the project as a zip) AND the release packages, where I bundle several of the packages that I use in my code. The releases are here: https://github.com/omarkb93/Chimera-Control/releases.

## What's Included?
The github package includes all of my source code (\*.h and \*.cpp files), as well as some zipped compiled packages. Specifically, these include 64- and 32-bit versions of:
- Boost 1.64.0 (zipped)
- HDF5 1.10.1 (zipped)
- Python 3.6 core DLLs and libs.

where, for example in my case $(PROJECT_DIR) = "C:\Users\Mark\Documents\Chimera-Control\Chimera".

## Install Visual Studio Community Version. The most recent version that I've used is visual studio 2019.
- The extra options don't matter very much.
- I suggest that you don't install the "Python compatibility" option and install python separately.
- Once you try to open my project, visual studio should prompt you to install the MFC tools which you need.

## Download and install the latest version of DAQmx from national instruments. They have installers for this.
If it's not there, you may have to search around for this file.
- Note that installing either daqmx or fgen should also install NI-Visa, which my code also uses.

## Download and install the latest version of NI-FGEN from national instruments. They have installers for this.
If it's not there, you may have to search around for this file.
- Note that installing either daqmx or fgen should also install NI-Visa, which my code also uses.
- Optional: Download and install Agilent's IO Libraries Suite for using agilents. My code should compile fine without this though. http://www.keysight.com/en/pd-1985909/io-libraries-suite?cc=US&lc=eng

## Install The Andor SOLIS SDK
- This is necessary to get the API which I use to talk to the Andor camera.
- I'm not including any installers or anything because this is not an open-source product, and I'd probably get in 
  trouble if Andor found out I published something on github. If you have trouble with this requirement, let me know.

## Code Setup
- If all the above works properly, the code should build and start fine, and then immediately throw errors about finding files and initializing things at you before crashing.
- You need to add a series of constant definitions in the "constants.h" file. These constants help my code locate the locations of various files and folders, as well as various settings. I use different macros here to define the constants depending on which computer I'm running the code from.
- For my laptop, this is done in the section that looks like
```
#define SPECTRE_LAPTOP
#ifdef SPECTURE_LAPTOP
...
#endif
```
- You can basically copy this dozen lines of code, rename the SPECTRE_LAPTOP constant, & make sure that all of the folders referenced therein exist.
- make sure you run the code in win32 mode.

## Python (Optional)
- There are two aspects to this. You want standard python libraries for just using python, and you want the python3X.dll, python3X_D.dll, and python3X.lib (X -> version number, "...\_D.dll" being the debug version) files.	These latter files are what my C code uses to do python programming, so compilation / running of my code will fail without these.
- I've included the python36.lib/dll and python36_D.lib/dll files in both the 32-bit and 64-bit folders.
- I suggest downloading the Anaconda Distribution (make sure to get the 32-bit version) for general python usage.
- If you want or need more modern or different python versions, you can find the release files in the anaconda distribution filesystem (find the ".anaconda" folder), but anaconda doesn't include the debug distributions. You can compile these from source if you like, but you can also get them by downloading python directly from python.org, you just need to do a custom install and specify debug files and symbols during the install.

## HDF5 (Optional)
- I've included HDF5 packages that I compiled myself using visual studio 2017 (necessary for compatibility) as zipped files in the project under the packages\ folder. You should just need to un-zip them to the correct location and it should work fine. I've included both release and debug versions of the files. By default, my project will look for the files in "C:\Program Files(x86)\HDF_Group\HDF5\HDF5-1.10.1\Debug" or "...\Release".
- IF you want to recompile the packages (otherwise skip this) (for example you want a newer HDF5 version), then there are some instructions on the HDF5 website, but I've also included my shortened version which worked for me.
- Download and unzip the CMake version of the HDF5 Source (can find on website, it should be the CMake-hdf5-XXX.zip file). It's important to unzip this to a file address that has NO SPACES in its path.
- in the unzipped file,	open the file "HDF5config.cmake". Find the part that sets the CTEST_CMAKE_GENERATOR constant. Add the following text to the logic structure there:

		elseif (${BUILD_GENERATOR} STREQUAL "VS2017")
			set (CTEST_CMAKE_GENERATOR "Visual Studio 15 2017")

At the time of this writing, the HDF5 cmake system is not yet aware of Visual studio 2017, and this change is necessary to fix that.
- run the following command:

		ctest -S HDF5config.cmake,BUILD_GENERATOR=VS2017 -C Release -V -O hdf5.log

or replace Release with Debug to get the debug version. HDF5 will now build, it will take some time.

- After the build finishes, go into the new "build" folder and find the HDF5-XXX.zip file. This contains the include, lib, and dll files that you need. Unzip it to the desired location, make sure the visual studio project is pointed to it, and you're good to go.

## Setting up Visual Studio
- If you follow the above directions carefully and are a bit lucky, my code my just compile after installing everything. However, some files may have shifted around to different locations during your install process, and you'll need to redirect my project to the proper locations for those files.
- In order to compile, the following need to be set correctly (all of which can be found in the project properties seen by right-clicking on the project in visual studio & selecting properties):
	- "Include Directories"
		- Otherwise includes, like "#include <boost...>" or "#include 'visa.h'" will fail. This is pretty obvious.
	- "Library Directories"
		- Tells the compiler where to look for the necessary \*.lib files. If it can't find the libraries, it will either throw a "can't open X.lib" or "unresolved external symbol" errors.
	- "Additional Dependencies"
		- Tells the compiler the names of the libraries it needs to make sure to open. You shouldn't need to change this.
- If the code ever seems like it's having trouble finding some files, you can always look at the values set in these places to see exactly where visual studio is looking for various files.

## Some light suggestions
- I recommend using Git for file management and versioning. It's what I use, and while you don't need git to download my repository or use it, I think it's good solution to a variety of problems. If you like these types of things, also install a git gui client, my favorite is SmartGit. http://www.syntevo.com/smartgit/
- I suggest using powershell over cmd when it comes to windows command line prompts. It's built in to windows & great; really cmd is horribly outdated and everything should probably use powershell.

# Main Systems Overview

## TTL System
We use a Viewpoint DIO64 card for an extensive and important digital output system. We call it the TTL system. the TTL system triggers virtually everything in the system to control the timing of everything in the experiment.

## DAC System
We use 3 (at the moment) national instruments Analog out PXI cards.

## Agilent System
We use a series of agilent arbitrary function generators during the experiment. they are programmed via Agilent's VISA protocol on a usb cable.

## Tektronics System
We use a couple tektronics generators for some higher frequency RF. They are programmed via visa protocol on a usb cable.

## Rhode Schwartz System
The Rhode-Schwartz generator (RSG) is a nice microwave generator we use in the experiment. It's programmed via GPIB. At this time it's the only GPIB system we use.

# Some Code Systems
## MFC and Win32
win32 is the C core of the GUI program. It interacts directly with windows. MFC (Microsoft foundation classes) is a system of classes that act as wrappers around win32, making writing guis much easier. An early implementation of the code was written in raw win32. Unfortunately, since this is written in MFC at this point, the project is incompatible with UNIX based OSs (linux & MacOS). In retrospect, Qt probably would have been a better more modern choice, but I've never wanted to go back and rewrite the gui.
## thrower("Error message") and the Error class
This is a little macro that I use extensively for throwing errors. It throws a custom exception object "Error".	The macro adds the file and line number to the throw function call so that it is easy to find from the gui where the throw happened, for debugging purposes. There's also a throwNested("nested error") which can be used to report a chain of erros, e.g. basically making a stack trace of the error.

The core of this was stolen from a stack overflow page: http://stackoverflow.com/questions/348833/how-to-know-the-exact-line-of-code-where-where-an-exception-has-been-caused

## EmbeddedPythonHandler
this is an embedded python interpreter that you can send python commands to. Sending commands to it is like sending commands to a python IDLE command prompt. It can pop or throw errors when python throws errors. Some things are just that much easier to do in python that implementing this was worth it.

Note that I avoid extensive usage of the Python-C api by using the simplistic "run this line in the python interpreter" approach. The Python-C api can be used to generate real python objects and functions etc. in C	(rather than in a string), but I found it rather hard to use properly because of memory management issues.

## ScriptStream
This is a custom stream object (similar to std::iostream or std::stringstream) that I use to manage my scripting languages. Primarily, it does some standard preprocessing of strings from a file as they get outputted into std::string objects.

## Texting system
This uses python to notify users of interesting events via text, for example it can notify the user when an experiment ends. It uses python to send an email to a phone-provider (i.e. verizon, AT&T) specific email address which forwards a text to your phone. Since this requires an email be sent, it requires you to log in to an email account. Note that google warns that the simplistic method I use to do this emailing is not very secure, and that I had to specifically allow such methods to be used with my email address. The password that you enter into the system is not held particularly securely. After one person enters the password into the program instance, it would not be very hard for someone else to extract the password from the program.

# Simple program modification instructions

## "I want to add a GUI element to an existing system (e.g. adding another button to the ttl system)"
- Add the element as a private member of the system (e.g. "Control<CButton> myButton") initialize the element in the class's initialize function.
- add a rearrange() command inside the system's rearrange function (e.g. myButton.rearrange(...))
- Add any special handling (e.g. handling button press, edit change) to the window to which the control belongs.

## "I want to create a new system (e.g. adding a control for a different type of function generator)."
- Create a class for that system. You may consider copying and renaming the class of another similar system already in this code if it is similar.
- Give it whatever gui elements and internal data structures you wish.
- Assuming the system has gui elements, create an "Initialize" and "Rearrange" functions, similar to those in other systems for initializing and moving around controls.
- Of course create functions and handling for any tasks you want the system to be able to do.
- Create an instance of the class object as a private member of the window to which it belongs (the "parent window")
- call the "Initialize" function you created in the OnInitDialog() function of the parent window.
- call the "Rearrange" function you created in the OnMove() function of the parent window.

## "I want to change what happens during the experiment procedure."
- Most likely you want to change ExperimentManager::experimentThreadProcedure function. Read the function carefully.

## "I want to add / change scripting keywords / scripting functionality."
- You want to add or modify the big if / else if / else if control structure in ExperimentManager::analyzeMasterScript and ExperimentManager::analyzeFunction functions, or the equivalent functions in the NIAWG and agilent systems.

## "I want to change the data output formatting / contents."
- You will want to change / add functions in the DataLogger class, and call any new functions in the commonFunctions::logData function.

## "I want to plot something in real time from the program."
- I've made the plotCtrl class for this. see example implementations of the dacs, ttls, oscilloscope views, and real time data analysis.
- It is also not hard to do plotting through the embedded python handler.
