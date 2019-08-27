﻿#pragma once
#include "constants.h"
#include "agilentStructures.h"

const agilentSettings UWAVE_AGILENT_SETTINGS = {
	// safemode option											
	UWAVE_SAFEMODE,
	// usb address
	UWAVE_AGILENT_USB_ADDRESS,
	// sample rate in hertz
	1e6,
	// Memory location, whether the device will save waveforms to 
	// the internal 64MB Memory buffer or to an external USB drive, which
	// can (obviously) have much more space.
	"INT",
	// device name (just a convenience, so that the class instance knows 
	// which device it is
	"UWave",
	// various control IDs (no need to change)
	IDC_UWAVE_CHANNEL1_BUTTON, IDC_UWAVE_CHANNEL2_BUTTON,
	IDC_UWAVE_SYNC_BUTTON, IDC_UWAVE_AGILENT_COMBO,
	IDC_UWAVE_FUNCTION_COMBO, IDC_UWAVE_EDIT, IDC_UWAVE_PROGRAM,
	IDC_UWAVE_CALIBRATION_BUTTON,
	UWAVE_AGILENT_TRIGGER_ROW, UWAVE_AGILENT_TRIGGER_NUM,
	// Configuration file delimiter, used for saving settings for this 
	// agilent.
	"MICROWAVE_AGILENT_AWG",
	// Calibration coefficients (arb length)
	{ },
	{ "output1 off", "output2 off",
	"Source1:burst:state off",  "Source2:burst:state off",
	"output1:load INF", "output2:load INF", 
	"SOURCE1:FUNC:ARB:FILTER Normal", "SOURCE2:FUNC:ARB:FILTER Normal" }
};

const agilentSettings TOP_BOTTOM_AGILENT_SETTINGS = { TOP_BOTTOM_AGILENT_SAFEMODE, TOP_BOTTOM_AGILENT_USB_ADDRESS,
 10e6, "INT", "Top_Bottom",
IDC_TOP_BOTTOM_CHANNEL1_BUTTON, IDC_TOP_BOTTOM_CHANNEL2_BUTTON,
IDC_TOP_BOTTOM_SYNC_BUTTON, IDC_TOP_BOTTOM_AGILENT_COMBO,
IDC_TOP_BOTTOM_FUNCTION_COMBO, IDC_TOP_BOTTOM_EDIT,
IDC_TOP_BOTTOM_PROGRAM, IDC_TOP_BOTTOM_CALIBRATION_BUTTON,
TOP_BOTTOM_AGILENT_TRIGGER_ROW, TOP_BOTTOM_AGILENT_TRIGGER_NUM,
"TOP_BOTTOM_AGILENT_AWG", { 0.00705436300498, 2.64993907362, -46.4639258399, 550.927892792, -3807.13743019, 
15548.5874213, -36450.3147559, 41176.7569795, 384.815598655, -41759.1457154, 8882.70382591, 26042.5221431, 
15930.7138745, -23506.4467208, -12245.7721134, -11521.3566847, 14301.7653239, 7231.41825458, 6022.29831157, 
3131.29460931, 8311.02350961, -13655.0675627, -11139.8388811, -4957.03018498, -3668.64359645, 9417.84306993, 
12035.3963169, 1654.16454032, -16230.8390772, 8171.7015004, 5007.09037684, 5463.49980024, -6960.85369185, 
2243.57572357, -5168.88995534, -1624.67211651, 1.10973170748, -5916.23101957, 1187.70683116, -2206.88472898, 
1126.16012039, 8185.05136329, 6312.13302578, 1239.26523376, -7565.47746904, 5116.22647775, -5425.55460459, 
-6455.76453072, 2942.34478932, 2692.33111404 },
{ "Trigger:Source external", "Trigger:Slope Positive", "output1:load INF", "output2:load INF",
   "SOURCE1:FUNC:ARB:FILTER Normal", "SOURCE2:FUNC:ARB:FILTER Normal" } };

const agilentSettings AXIAL_AGILENT_SETTINGS = { AXIAL_AGILENT_SAFEMODE, AXIAL_AGILENT_USB_ADDRESS,
1e6, "INT", "Axial",
IDC_AXIAL_CHANNEL1_BUTTON, IDC_AXIAL_CHANNEL2_BUTTON,
IDC_AXIAL_SYNC_BUTTON, IDC_AXIAL_AGILENT_COMBO,
IDC_AXIAL_FUNCTION_COMBO, IDC_AXIAL_EDIT, IDC_AXIAL_PROGRAM,
IDC_AXIAL_CALIBRATION_BUTTON,
AXIAL_AGILENT_TRIGGER_ROW, AXIAL_AGILENT_TRIGGER_NUM,
"AXIAL_AGILENT_AWG",{ },
{ "Trigger:Source immediate", "Trigger:Slope Positive", "output1:load INF", "output2:load INF",
  "SOURCE1:FUNC:ARB:FILTER Normal", "SOURCE2:FUNC:ARB:FILTER Normal" }
};

const agilentSettings FLASHING_AGILENT_SETTINGS = { FLASHING_SAFEMODE, FLASHING_AGILENT_USB_ADDRESS,
1e6, "INT", "Flashing",
IDC_FLASHING_CHANNEL1_BUTTON, IDC_FLASHING_CHANNEL2_BUTTON,
IDC_FLASHING_SYNC_BUTTON, IDC_FLASHING_AGILENT_COMBO,
IDC_FLASHING_FUNCTION_COMBO, IDC_FLASHING_EDIT,
IDC_FLASHING_PROGRAM, IDC_FLASHING_CALIBRATION_BUTTON,
FLASHING_AGILENT_TRIGGER_ROW, FLASHING_AGILENT_TRIGGER_NUM,
"FLASHING_AGILENT_AWG", { },
{ "output1 off", "output2 off",
"Source1:burst:state off",
"Source1:Apply:Square 2 MHz, 3 VPP, 1.5 V",
"Source1:Function:Square:DCycle 42",
"Source2:burst:state off",
"Source2:Apply:Square 2 MHz, 3 VPP, 1.5 V",
"Source2:Function:Square:DCycle 50",
"Source1:burst:state off", "Source2:burst:state off",
"output1 on", "output2 on", "output1:load INF", "output2:load INF",
"SOURCE1:FUNC:ARB:FILTER Normal", "SOURCE2:FUNC:ARB:FILTER Normal",
"SOURCE1:PHASE 0", "SOURCE2:PHASE 165",
"SOURCE1:PHASE:SYNCHRONIZE", "SOURCE2:PHASE:SYNCHRONIZE" }
}; 


const agilentSettings INTENSITY_AGILENT_SETTINGS = { INTENSITY_SAFEMODE, INTENSITY_AGILENT_USB_ADDRESS,
													1e6, "USB", "Intensity",
													IDC_INTENSITY_CHANNEL1_BUTTON, IDC_INTENSITY_CHANNEL2_BUTTON,
													IDC_INTENSITY_SYNC_BUTTON, IDC_INTENSITY_AGILENT_COMBO,
													IDC_INTENSITY_FUNCTION_COMBO, IDC_INTENSITY_EDIT,
													IDC_INTENSITY_PROGRAM, IDC_INTENSITY_CALIBRATION_BUTTON,
													INTENSITY_AGILENT_TRIGGER_ROW, INTENSITY_AGILENT_TRIGGER_NUM, 
													"INTENSITY_AGILENT_AWG",{ 0.00102751, -0.02149967 },
													{ "Trigger:Source external", "Trigger:Slope Positive", 
													  "output1:load INF", "output2:load INF",
													  "SOURCE1:FUNC:ARB:FILTER Normal", "SOURCE2:FUNC:ARB:FILTER Normal" }
};

