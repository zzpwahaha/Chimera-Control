#pragma once
#include <atomic>
#include "PictureManager.h"
#include "PictureControl.h"
#include "Control.h"
#include "BaslerSettingsControl.h"
#include "BaslerCamera.h"
#include "PictureStats.h"
//#include "PictureSaver.h"
#include "commonTypes.h"
#include "PlotCtrl.h"

class MainWindow;
class ScriptingWindow;
class CameraWindow;
class AuxiliaryWindow;

class BaslerWindow : public CDialogEx
{
	public:
		BaslerWindow( );
		BOOL OnInitDialog();
		void handleEnter();
		void OnPaint();
		void passCommonCommand ( UINT id );
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		HCURSOR OnQueryDragIcon();
		void OnSize( UINT nType, int cx, int cy );
		void handleArmPress();
		void handleClose();
		void handleDisarmPress();
		void initializeControls();
		void passExposureMode();
		void passCameraMode();
		afx_msg LRESULT handleNewPics( WPARAM wParam, LPARAM lParam );
		void pictureRangeEditChange( UINT id );
		void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar );
		void handleSoftwareTrigger();
		void OnMouseMove(UINT flags, CPoint point);
		void OnRButtonUp( UINT stuff, CPoint clickLocation );
		void passSetLocationsButton();
		void DoDataExchange( CDataExchange* pDX );
		void loadFriends ( MainWindow* mainWin_, ScriptingWindow* scriptWin_, CameraWindow* camWin_, AuxiliaryWindow* auxWin_ );

	private:
		// for the basler window, this is typically only one picture, but I include this here anyways.
		PictureManager picManager;
		//PictureControl picture;
		BaslerSettingsControl settings;
		BaslerCameras* cameraController;
		PictureStats stats;
		//PictureSaver saver;
		unsigned int currentRepNumber;
		std::string runExposureMode;
		std::vector<std::vector<long>> images;
		unsigned int imageWidth;
		CMenu menu;
		bool isRunning;
		std::atomic<bool> triggerThreadFlag;
		PlotCtrl *horGraph, *vertGraph;
		std::vector<Gdiplus::Pen*> plotPens, brightPlotPens;
		std::vector<Gdiplus::SolidBrush*> plotBrushes, brightPlotBrushes;
		CFont* plotfont;
		MainWindow* mainWin;
		CameraWindow* camWin;
		AuxiliaryWindow* auxWin;
		ScriptingWindow* scriptWin;
		coordinate selectedPixel = { 0,0 };
	protected:
		//virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
		HICON m_hIcon;

	DECLARE_MESSAGE_MAP()
};
