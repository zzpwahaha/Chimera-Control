﻿// created by Mark O. Brown
#include "stdafx.h"
#include "functionCombo.h"
#include "ConfigurationSystems/ProfileSystem.h"

void functionCombo::loadFunctions( )
{
	ProfileSystem::reloadCombo( GetSafeHwnd( ), functionLocation, str( "*." )
								+ FUNCTION_EXTENSION, "__NONE__" );
	InsertString( 0, "Parent Script" );
}
