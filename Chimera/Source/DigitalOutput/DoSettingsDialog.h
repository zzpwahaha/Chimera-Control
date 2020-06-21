// created by Mark O. Brown
#pragma once
#include "DoSystem.h"
#include <unordered_map>

struct ttlInputStruct
{
	DoSystem* ttls;
	cToolTips toolTips;
	AuxiliaryWindow* master;
};

class TtlSettingsDialog : public CDialog
{
	public:
		TtlSettingsDialog(ttlInputStruct* inputPtr, UINT dialogResource);
		void handleOk();
		void handleCancel();
		BOOL OnInitDialog();
		void OnSize( UINT type, int w, int h );
	private:
		DECLARE_MESSAGE_MAP();
		ttlInputStruct* input;
		std::array<Control<CStatic>, 16> numberlabels;
		std::array<Control<CStatic>, 4> rowLabels;
		std::array<std::array<Control<CEdit>, 16>, 4> edits;
};
