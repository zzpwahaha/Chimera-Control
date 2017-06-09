#include "stdafx.h"
#include "ColorBox.h"
#include <tuple>

void ColorBox::initialize(POINT& pos, int& id, CWnd* parent, int length, fontMap fonts,
	std::vector<CToolTipCtrl*>& tooltips)
{
	//
	boxes.niawg.sPos = { pos.x, pos.y, long(pos.x + length/3), pos.y + 20 };
	boxes.niawg.ID = id++;
	boxes.niawg.Create("NIAWG", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS | SS_CENTER | WS_BORDER,
					 boxes.niawg.sPos, parent, boxes.niawg.ID);
	boxes.niawg.fontType = Code;
	//
	boxes.camera.sPos = { long(pos.x + length/3.0), pos.y, long(pos.x + 2*length/3.0), pos.y + 20 };
	boxes.camera.ID = id++;
	boxes.camera.Create( "CAMERA", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS | SS_CENTER | WS_BORDER, 
					  boxes.camera.sPos, parent, boxes.camera.ID );
	boxes.camera.fontType = Code;
	//
	boxes.intensity.sPos = { long(pos.x + 2*length/3.0), pos.y, pos.x + length, pos.y + 20 };
	boxes.intensity.ID = id++;
	boxes.intensity.Create( "INTENSITY", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS | SS_CENTER | WS_BORDER,
						 boxes.intensity.sPos, parent, boxes.intensity.ID );
	boxes.intensity.fontType = Code;
	pos.y += 20;
}


void ColorBox::rearrange(std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts)
{
	boxes.niawg.rearrange(cameraMode, triggerMode, width, height, fonts);
	boxes.camera.rearrange( cameraMode, triggerMode, width, height, fonts );
	boxes.intensity.rearrange( cameraMode, triggerMode, width, height, fonts );
}

CBrush* ColorBox::handleColoring( int id, CDC* pDC, brushMap brushes, rgbMap rgbs)
{
	if (id == boxes.niawg.ID)
	{
		if (colors.niawg == 'G')
		{
			// Color Green. This is the "Ready to give next waveform" color. During this color you can also press esc to exit.
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Green"]);
			return brushes["Green"];
		}
		else if (colors.niawg == 'Y')
		{
			// Color Yellow. This is the "Working" Color.
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Gold"]);
			return brushes["Gold"];
		}
		else if (colors.niawg == 'R')
		{
			// Color Red. This is a big visual signifier for when the program exited with error.
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Red"]);
			return brushes["Red"];
		}
		else
		{
			// color Blue. This is the default, ready for user input color.
			pDC->SetTextColor( rgbs["White"] );
			pDC->SetBkColor( rgbs["Dark Grey"] );
			return brushes["Dark Grey"];
		}
	}
	else if ( id == boxes.camera.ID )
	{
		if ( colors.camera == 'G' )
		{
			// Color Green. This is the "Ready to give next waveform" color. During this color you can also press esc to exit.
			pDC->SetTextColor( rgbs["White"] );
			pDC->SetBkColor( rgbs["Green"] );
			return brushes["Green"];
		}
		else if ( colors.camera == 'Y' )
		{
			// Color Yellow. This is the "Working" Color.
			pDC->SetTextColor( rgbs["White"] );
			pDC->SetBkColor( rgbs["Gold"] );
			return brushes["Gold"];
		}
		else if ( colors.camera == 'R' )
		{
			// Color Red. This is a big visual signifier for when the program exited with error.
			pDC->SetTextColor( rgbs["White"] );
			pDC->SetBkColor( rgbs["Red"] );
			return brushes["Red"];
		}
		else
		{
			// color Blue. This is the default, ready for user input color.
			pDC->SetTextColor( rgbs["White"] );
			pDC->SetBkColor( rgbs["Dark Grey"] );
			return brushes["Dark Grey"];
		}
	}
	else if ( id == boxes.intensity.ID )
	{
		if ( colors.intensity == 'G' )
		{
			// Color Green. This is the "Ready to give next waveform" color. During this color you can also press esc to exit.
			pDC->SetTextColor( rgbs["White"] );
			pDC->SetBkColor( rgbs["Green"] );
			return brushes["Green"];
		}
		else if ( colors.intensity == 'Y' )
		{
			// Color Yellow. This is the "Working" Color.
			pDC->SetTextColor( rgbs["White"] );
			pDC->SetBkColor( rgbs["Gold"] );
			return brushes["Gold"];
		}
		else if ( colors.intensity == 'R' )
		{
			// Color Red. This is a big visual signifier for when the program exited with error.
			pDC->SetTextColor( rgbs["White"] );
			pDC->SetBkColor( rgbs["Red"] );
			return brushes["Red"];
		}
		else
		{
			// color Blue. This is the default, ready for user input color.
			pDC->SetTextColor( rgbs["White"] );
			pDC->SetBkColor( rgbs["Dark Grey"] );
			return brushes["Dark Grey"];
		}
	}
	else
	{
		return NULL;
	}
}

/*
 * color should be a three-character long 
 */
void ColorBox::changeColor( colorBoxes<char> newColors )
{
	if ( newColors.niawg != '-' )
	{
		colors.niawg = newColors.niawg;
		boxes.niawg.RedrawWindow();
	}
	if ( newColors.camera != '-' )
	{
		colors.camera = newColors.camera;
		boxes.camera.RedrawWindow();
	}
	if ( newColors.intensity != '-' )
	{
		colors.intensity = newColors.intensity;
		boxes.intensity.RedrawWindow();
	}
}