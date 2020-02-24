#include "stdafx.h"
#include "NiawgSystem.h"
#include "ConfigurationSystems/ProfileSystem.h"

NiawgSystem::NiawgSystem (DoRows::which trigRow, UINT trigNumber, bool safemode) : core (trigRow, trigNumber, safemode)
{};

void NiawgSystem::initialize (int& id, POINT& loc, CWnd* parent, cToolTips& tooltips)
{
	niawgHeader.sPos = { loc.x, loc.y, loc.x + 640, loc.y += 30 };
	niawgHeader.Create ("NIAWG SYSTEM", NORM_HEADER_OPTIONS, niawgHeader.sPos, parent, id++);
	niawgHeader.fontType = fontTypes::HeadingFont;

	controlNiawg.sPos = { loc.x, loc.y, loc.x + 640, loc.y += 25 };
	controlNiawg.Create ("Control System?", NORM_CHECK_OPTIONS, controlNiawg.sPos, parent, id++);
	controlNiawg.SetCheck (true);

	rearrangeCtrl.initialize (id, loc, parent, tooltips);
	niawgScript.initialize (640, 400, loc, tooltips, parent, id, "NIAWG", "NIAWG Script", { IDC_NIAWG_FUNCTION_COMBO,
							IDC_NIAWG_EDIT }, _myRGBs["Interactable-Bkgd"]);
}


void NiawgSystem::rearrange (UINT width, UINT height, fontMap fonts)
{
	niawgHeader.rearrange (width, height, fonts);
	controlNiawg.rearrange (width, height, fonts);
	niawgScript.rearrange (width, height, fonts);
	rearrangeCtrl.rearrange (width, height, fonts);
}

void NiawgSystem::handleSaveConfig (std::ofstream& saveFile)
{
	saveFile << "NIAWG_INFORMATION\n";
	saveFile << controlNiawg.GetCheck ();
	saveFile << "END_NIAWG_INFORMATION\n";
	rearrangeCtrl.handleSaveConfig (saveFile);
}

bool NiawgSystem::getControlNiawgFromConfig ( std::ifstream& openfile, Version ver )
{
	if (ver < Version ("4.12")) { return true; }
	bool opt;
	openfile >> opt;
	return opt;
}

void NiawgSystem::handleOpenConfig (std::ifstream& openfile, Version ver)
{
	controlNiawg.SetCheck ( ProfileSystem::stdGetFromConfig (openfile, "NIAWG_INFORMATION",
							NiawgSystem::getControlNiawgFromConfig, Version ("4.12")) );
	ProfileSystem::standardOpenConfig (openfile, "REARRANGEMENT_INFORMATION", &rearrangeCtrl);
}