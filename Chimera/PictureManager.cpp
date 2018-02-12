#include "stdafx.h"
#include "PictureManager.h"


void PictureManager::setPalletes(std::array<int, 4> palleteIds)
{
	for (int picInc = 0; picInc < 4; picInc++)
	{
		pictures[picInc].updatePalette(palettes[palleteIds[picInc]]);
	}
}

void PictureManager::handleMouse( CPoint point )
{
	for ( auto& pic : pictures )
	{
		pic.handleMouse( point );
	}
}

void PictureManager::setAlwaysShowGrid(bool showOption, CDC* easel)
{
	alwaysShowGrid = showOption;
	if (alwaysShowGrid)
	{
		if (!pictures[1].isActive())
		{
			pictures[0].drawGrid(easel, gridBrush);
			return;
		}
		for (auto& pic : pictures)
		{
			pic.drawGrid(easel, gridBrush);
		}
	}
}


void PictureManager::redrawPictures( CDC* easel, coordinate selectedLocation, std::vector<coordinate> analysisLocs,
									 atomGrid gridInfo, bool forceGrid )
{
	if (!pictures[1].isActive())
	{
		pictures[0].redrawImage(easel);
		if (alwaysShowGrid || forceGrid )
		{
			pictures[0].drawGrid(easel, gridBrush);
		}
		drawDongles(easel, selectedLocation, analysisLocs, gridInfo);
		return;
	}
	for (auto& pic : pictures)
	{
		pic.redrawImage(easel);
		if (alwaysShowGrid || forceGrid )
		{
			pic.drawGrid(easel, gridBrush);
		}
	}
	drawDongles(easel, selectedLocation, analysisLocs, gridInfo);
}


/*
 *  
 */
void PictureManager::drawDongles( CDC* dc, coordinate selectedLocation, std::vector<coordinate> analysisLocs, 
								  atomGrid gridInfo )
{
	for (auto& pic : pictures)
	{
		pic.drawCircle(dc, selectedLocation);
		pic.drawAnalysisMarkers(dc, analysisLocs, gridInfo);
	}
}


void PictureManager::setNumberPicturesActive( int numberActive )
{
	int count = 1;
	for (auto& pic : pictures)
	{
		pic.setActive( count <= numberActive );
		count++;
	}
}

void PictureManager::handleEditChange( UINT id )
{
	for (auto& pic : pictures)
	{
		pic.handleEditChange( id );
	}
}


void PictureManager::setAutoScalePicturesOption(bool autoScaleOption)
{
	autoScalePictures = autoScaleOption;
}


void PictureManager::handleNewConfig( std::ofstream& newFile )
{
	newFile << "PICTURE_MANAGER\n";

	for ( auto& pic : pictures )
	{
		std::pair<UINT, UINT> sliderLoc = { 0, 1000 };
		newFile << sliderLoc.first << " " << sliderLoc.second << "\n";
	}
	newFile << 0 << " ";
	newFile << 0 << " ";
	newFile << 0 << " ";
	newFile << 0 << " ";
	newFile << "END_PICTURE_MANAGER\n";
}


void PictureManager::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "PICTURE_MANAGER\n";

	for (auto& pic : pictures)
	{
		std::pair<UINT, UINT> sliderLoc = pic.getSliderLocations();
		saveFile << sliderLoc.first << " " << sliderLoc.second << "\n";
	}
	saveFile << autoScalePictures << " ";
	saveFile << specialGreaterThanMax << " ";
	saveFile << specialLessThanMin << " ";
	saveFile << alwaysShowGrid << " ";

	saveFile << "END_PICTURE_MANAGER\n";
}


void PictureManager::handleOpenConfig(std::ifstream& configFile, int versionMajor, int versionMinor )
{
	ProfileSystem::checkDelimiterLine(configFile, "PICTURE_MANAGER");
	std::array<int, 4> maxes, mins;
	for (int sliderInc = 0; sliderInc < 4; sliderInc++)
	{
		configFile >> mins[sliderInc];
		configFile >> maxes[sliderInc];
	}
	configFile >> autoScalePictures;
	configFile >> specialGreaterThanMax;
	configFile >> specialLessThanMin;
	configFile >> alwaysShowGrid;
	UINT count = 0;
	for (auto& pic : pictures)
	{
		pic.setSliderPositions(mins[count], maxes[count]);
		count++;
	}
	configFile.get();
	ProfileSystem::checkDelimiterLine(configFile, "END_PICTURE_MANAGER");
}


void PictureManager::setSpecialLessThanMin(bool option)
{
	specialLessThanMin = option;
}


void PictureManager::setSpecialGreaterThanMax(bool option)
{
	specialGreaterThanMax = option;
}


void PictureManager::drawPicture( CDC* deviceContext, int pictureNumber, std::vector<long> picData, 
								 std::pair<UINT, UINT> minMaxPair )
{
	std::tuple<bool, int, int> autoScaleInfo = std::make_tuple(autoScalePictures, minMaxPair.first, minMaxPair.second);
	pictures[pictureNumber].drawPicture( deviceContext, picData, autoScaleInfo, specialLessThanMin, 
										specialGreaterThanMax);
	if (alwaysShowGrid)
	{
		pictures[pictureNumber].drawGrid(deviceContext, gridBrush);
	}
}

void PictureManager::handleScroll(UINT nSBCode, UINT nPos, CScrollBar* scrollbar)
{
	if (nSBCode == SB_THUMBPOSITION || nSBCode == SB_THUMBTRACK)
	{
		int id = scrollbar->GetDlgCtrlID();
		for (auto& control : pictures)
		{
			control.handleScroll(id, nPos);
		}
	}
}

coordinate PictureManager::handleRClick( CPoint clickLocation )
{
	coordinate location;
	for (auto& pic : pictures)
	{
		try
		{
			location = pic.checkClickLocation( clickLocation );
			return location;
		}
		catch(Error&){}
		// checkClickLocation throws if not found. Continue looking.
	}
	thrower( "not found" );
}


void PictureManager::setSinglePicture( CWnd* parent, imageParameters imageParams)
{
	for (UINT picNum = 0; picNum < 4; picNum++)
	{
		if (picNum < 1)
		{
			pictures[picNum].setActive(true);
		}
		else
		{
			pictures[picNum].setActive(false);
		}
	}
	pictures.front().setPictureArea( picturesLocation, picturesWidth , picturesHeight);
	pictures.front().setSliderLocations(parent);
	pictures.front( ).setCursorValueLocations( parent );
	setParameters( imageParams );	
}


void PictureManager::resetPictureStorage()
{
	for (auto& pic : pictures)
	{
		pic.resetStorage();
	}
}


void PictureManager::setMultiplePictures( CWnd* parent, imageParameters imageParams, UINT numberActivePics )
{
	for (UINT picNum = 0; picNum < 4; picNum++)
	{
		if (picNum < numberActivePics)
		{
			pictures[picNum].setActive(true);
		}
		else
		{
			pictures[picNum].setActive(false);
		}
	}

	POINT loc = picturesLocation;
	int picWidth = 550;
	int picHeight = 420;
	pictures[0].setPictureArea( loc, picWidth, picHeight );
	loc.x += 550;
	pictures[1].setPictureArea( loc, picWidth, picHeight );
	loc.x -= 550;
	loc.y += 445;
	pictures[2].setPictureArea( loc, picWidth, picHeight );
	loc.x += 550;
	pictures[3].setPictureArea( loc, picWidth, picHeight );
	setParameters( imageParams );
	setPictureSliders( parent );
	for ( auto& pic : pictures )
	{
		pic.setCursorValueLocations( parent );
	}
}


void PictureManager::setPictureSliders(CWnd* parent)
{
	for (auto& pic : pictures)
	{
		pic.setSliderLocations(parent);
	}
}


void PictureManager::drawBackgrounds(CDC* easel)
{
	for (auto& pic : pictures)
	{
		pic.drawBackground(easel);
	}
}


void PictureManager::initialize( POINT& loc, CWnd* parent, int& id, cToolTips& tooltips,
								 CBrush* defaultBrush)
{
	picturesLocation = loc;
	picturesWidth = 550 * 2;
	picturesHeight = 460 * 2 + 5;
	gridBrush = defaultBrush;
	//
	pictures[0].initialize( loc, parent, id, 550, 440, { IDC_PICTURE_1_MIN_EDIT, IDC_PICTURE_1_MAX_EDIT } );
	loc.x += 550;
	pictures[1].initialize(loc, parent, id, 550, 440, { IDC_PICTURE_2_MIN_EDIT, IDC_PICTURE_2_MAX_EDIT } );
	loc.x -= 550;
	loc.y += 445;
	pictures[2].initialize(loc, parent, id, 550, 440, { IDC_PICTURE_3_MIN_EDIT, IDC_PICTURE_3_MAX_EDIT } );
	loc.x += 550;
	pictures[3].initialize(loc, parent, id, 550, 440, { IDC_PICTURE_4_MIN_EDIT, IDC_PICTURE_4_MAX_EDIT } );
	loc.y += 440;
	loc.x -= 550;
	createPalettes( parent->GetDC() );
	for (auto& pic : pictures)
	{
		pic.updatePalette( palettes[2] );
	}
	// initialize to one. This matches the camera settings initialization.
	setNumberPicturesActive( 1 );
}


void PictureManager::refreshBackgrounds(CDC* easel)
{
	if (!pictures[1].isActive())
	{
		pictures[0].drawBackground( easel );
	}
	else
	{
		for (auto& picture : pictures)
		{
			picture.drawBackground( easel );
		}
	}
}


void PictureManager::drawGrids(CDC* easel)
{
	for (auto& picture : pictures)
	{
		picture.drawGrid(easel, gridBrush );
	}
}


void PictureManager::setParameters(imageParameters parameters)
{
	for (auto& picture : pictures)
	{
		picture.recalculateGrid(parameters);
	}
}


void PictureManager::rearrange(std::string cameraMode, std::string triggerMode, int width, int height, 
							   fontMap fonts)
{
	for (auto& control : pictures)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
}


void PictureManager::createPalettes( CDC* dc )
{
	struct
	{
		WORD Version;
		WORD NumberEntries;
		PALETTEENTRY aEntries[256];
	} Palette = { 0x300, 256 };

	GetSystemPaletteEntries( *dc, 0, 256, Palette.aEntries );
	// this is the parula colormap from matlab. It looks nice :D
	double virida[256][3] =
	{
		// special entry
		{ 0 , 0 , 1 },
		//
		{ 0.26851 , 0.009605 , 0.335427 },
		{ 0.269944 , 0.014625 , 0.341379 },
		{ 0.271305 , 0.019942 , 0.347269 },
		{ 0.272594 , 0.025563 , 0.353093 },
		{ 0.273809 , 0.031497 , 0.358853 },
		{ 0.274952 , 0.037752 , 0.364543 },
		{ 0.276022 , 0.044167 , 0.370164 },
		{ 0.277018 , 0.050344 , 0.375715 },
		{ 0.277941 , 0.056324 , 0.381191 },
		{ 0.278791 , 0.062145 , 0.386592 },
		{ 0.279566 , 0.067836 , 0.391917 },
		{ 0.280267 , 0.073417 , 0.397163 },
		{ 0.280894 , 0.078907 , 0.402329 },
		{ 0.281446 , 0.08432 , 0.407414 },
		{ 0.281924 , 0.089666 , 0.412415 },
		{ 0.282327 , 0.094955 , 0.417331 },
		{ 0.282656 , 0.100196 , 0.42216 },
		{ 0.28291 , 0.105393 , 0.426902 },
		{ 0.283091 , 0.110553 , 0.431554 },
		{ 0.283197 , 0.11568 , 0.436115 },
		{ 0.283229 , 0.120777 , 0.440584 },
		{ 0.283187 , 0.125848 , 0.44496 },
		{ 0.283072 , 0.130895 , 0.449241 },
		{ 0.282884 , 0.13592 , 0.453427 },
		{ 0.282623 , 0.140926 , 0.457517 },
		{ 0.28229 , 0.145912 , 0.46151 },
		{ 0.281887 , 0.150881 , 0.465405 },
		{ 0.281412 , 0.155834 , 0.469201 },
		{ 0.280868 , 0.160771 , 0.472899 },
		{ 0.280255 , 0.165693 , 0.476498 },
		{ 0.279574 , 0.170599 , 0.479997 },
		{ 0.278826 , 0.17549 , 0.483397 },
		{ 0.278012 , 0.180367 , 0.486697 },
		{ 0.277134 , 0.185228 , 0.489898 },
		{ 0.276194 , 0.190074 , 0.493001 },
		{ 0.275191 , 0.194905 , 0.496005 },
		{ 0.274128 , 0.199721 , 0.498911 },
		{ 0.273006 , 0.20452 , 0.501721 },
		{ 0.271828 , 0.209303 , 0.504434 },
		{ 0.270595 , 0.214069 , 0.507052 },
		{ 0.269308 , 0.218818 , 0.509577 },
		{ 0.267968 , 0.223549 , 0.512008 },
		{ 0.26658 , 0.228262 , 0.514349 },
		{ 0.265145 , 0.232956 , 0.516599 },
		{ 0.263663 , 0.237631 , 0.518762 },
		{ 0.262138 , 0.242286 , 0.520837 },
		{ 0.260571 , 0.246922 , 0.522828 },
		{ 0.258965 , 0.251537 , 0.524736 },
		{ 0.257322 , 0.25613 , 0.526563 },
		{ 0.255645 , 0.260703 , 0.528312 },
		{ 0.253935 , 0.265254 , 0.529983 },
		{ 0.252194 , 0.269783 , 0.531579 },
		{ 0.250425 , 0.27429 , 0.533103 },
		{ 0.248629 , 0.278775 , 0.534556 },
		{ 0.246811 , 0.283237 , 0.535941 },
		{ 0.244972 , 0.287675 , 0.53726 },
		{ 0.243113 , 0.292092 , 0.538516 },
		{ 0.241237 , 0.296485 , 0.539709 },
		{ 0.239346 , 0.300855 , 0.540844 },
		{ 0.237441 , 0.305202 , 0.541921 },
		{ 0.235526 , 0.309527 , 0.542944 },
		{ 0.233603 , 0.313828 , 0.543914 },
		{ 0.231674 , 0.318106 , 0.544834 },
		{ 0.229739 , 0.322361 , 0.545706 },
		{ 0.227802 , 0.326594 , 0.546532 },
		{ 0.225863 , 0.330805 , 0.547314 },
		{ 0.223925 , 0.334994 , 0.548053 },
		{ 0.221989 , 0.339161 , 0.548752 },
		{ 0.220057 , 0.343307 , 0.549413 },
		{ 0.21813 , 0.347432 , 0.550038 },
		{ 0.21621 , 0.351535 , 0.550627 },
		{ 0.214298 , 0.355619 , 0.551184 },
		{ 0.212395 , 0.359683 , 0.55171 },
		{ 0.210503 , 0.363727 , 0.552206 },
		{ 0.208623 , 0.367752 , 0.552675 },
		{ 0.206756 , 0.371758 , 0.553117 },
		{ 0.204903 , 0.375746 , 0.553533 },
		{ 0.203063 , 0.379716 , 0.553925 },
		{ 0.201239 , 0.38367 , 0.554294 },
		{ 0.19943 , 0.387607 , 0.554642 },
		{ 0.197636 , 0.391528 , 0.554969 },
		{ 0.19586 , 0.395433 , 0.555276 },
		{ 0.1941 , 0.399323 , 0.555565 },
		{ 0.192357 , 0.403199 , 0.555836 },
		{ 0.190631 , 0.407061 , 0.556089 },
		{ 0.188923 , 0.41091 , 0.556326 },
		{ 0.187231 , 0.414746 , 0.556547 },
		{ 0.185556 , 0.41857 , 0.556753 },
		{ 0.183898 , 0.422383 , 0.556944 },
		{ 0.182256 , 0.426184 , 0.55712 },
		{ 0.180629 , 0.429975 , 0.557282 },
		{ 0.179019 , 0.433756 , 0.55743 },
		{ 0.177423 , 0.437527 , 0.557565 },
		{ 0.175841 , 0.44129 , 0.557685 },
		{ 0.174274 , 0.445044 , 0.557792 },
		{ 0.172719 , 0.448791 , 0.557885 },
		{ 0.171176 , 0.45253 , 0.557965 },
		{ 0.169646 , 0.456262 , 0.55803 },
		{ 0.168126 , 0.459988 , 0.558082 },
		{ 0.166617 , 0.463708 , 0.558119 },
		{ 0.165117 , 0.467423 , 0.558141 },
		{ 0.163625 , 0.471133 , 0.558148 },
		{ 0.162142 , 0.474838 , 0.55814 },
		{ 0.160665 , 0.47854 , 0.558115 },
		{ 0.159194 , 0.482237 , 0.558073 },
		{ 0.157729 , 0.485932 , 0.558013 },
		{ 0.15627 , 0.489624 , 0.557936 },
		{ 0.154815 , 0.493313 , 0.55784 },
		{ 0.153364 , 0.497 , 0.557724 },
		{ 0.151918 , 0.500685 , 0.557587 },
		{ 0.150476 , 0.504369 , 0.55743 },
		{ 0.149039 , 0.508051 , 0.55725 },
		{ 0.147607 , 0.511733 , 0.557049 },
		{ 0.14618 , 0.515413 , 0.556823 },
		{ 0.144759 , 0.519093 , 0.556572 },
		{ 0.143343 , 0.522773 , 0.556295 },
		{ 0.141935 , 0.526453 , 0.555991 },
		{ 0.140536 , 0.530132 , 0.555659 },
		{ 0.139147 , 0.533812 , 0.555298 },
		{ 0.13777 , 0.537492 , 0.554906 },
		{ 0.136408 , 0.541173 , 0.554483 },
		{ 0.135066 , 0.544853 , 0.554029 },
		{ 0.133743 , 0.548535 , 0.553541 },
		{ 0.132444 , 0.552216 , 0.553018 },
		{ 0.131172 , 0.555899 , 0.552459 },
		{ 0.129933 , 0.559582 , 0.551864 },
		{ 0.128729 , 0.563265 , 0.551229 },
		{ 0.127568 , 0.566949 , 0.550556 },
		{ 0.126453 , 0.570633 , 0.549841 },
		{ 0.125394 , 0.574318 , 0.549086 },
		{ 0.124395 , 0.578002 , 0.548287 },
		{ 0.123463 , 0.581687 , 0.547445 },
		{ 0.122606 , 0.585371 , 0.546557 },
		{ 0.121831 , 0.589055 , 0.545623 },
		{ 0.121148 , 0.592739 , 0.544641 },
		{ 0.120565 , 0.596422 , 0.543611 },
		{ 0.120092 , 0.600104 , 0.54253 },
		{ 0.119738 , 0.603785 , 0.5414 },
		{ 0.119512 , 0.607464 , 0.540218 },
		{ 0.119423 , 0.611141 , 0.538982 },
		{ 0.119483 , 0.614817 , 0.537692 },
		{ 0.119699 , 0.61849 , 0.536347 },
		{ 0.120081 , 0.622161 , 0.534946 },
		{ 0.120638 , 0.625828 , 0.533488 },
		{ 0.12138 , 0.629492 , 0.531973 },
		{ 0.122312 , 0.633153 , 0.530398 },
		{ 0.123444 , 0.636809 , 0.528763 },
		{ 0.12478 , 0.640461 , 0.527068 },
		{ 0.126326 , 0.644107 , 0.525311 },
		{ 0.128087 , 0.647749 , 0.523491 },
		{ 0.130067 , 0.651384 , 0.521608 },
		{ 0.132268 , 0.655014 , 0.519661 },
		{ 0.134692 , 0.658636 , 0.517649 },
		{ 0.137339 , 0.662252 , 0.515571 },
		{ 0.14021 , 0.665859 , 0.513427 },
		{ 0.143303 , 0.669459 , 0.511215 },
		{ 0.146616 , 0.67305 , 0.508936 },
		{ 0.150148 , 0.676631 , 0.506589 },
		{ 0.153894 , 0.680203 , 0.504172 },
		{ 0.157851 , 0.683765 , 0.501686 },
		{ 0.162016 , 0.687316 , 0.499129 },
		{ 0.166383 , 0.690856 , 0.496502 },
		{ 0.170948 , 0.694384 , 0.493803 },
		{ 0.175707 , 0.6979 , 0.491033 },
		{ 0.180653 , 0.701402 , 0.488189 },
		{ 0.185783 , 0.704891 , 0.485273 },
		{ 0.19109 , 0.708366 , 0.482284 },
		{ 0.196571 , 0.711827 , 0.479221 },
		{ 0.202219 , 0.715272 , 0.476084 },
		{ 0.20803 , 0.718701 , 0.472873 },
		{ 0.214 , 0.722114 , 0.469588 },
		{ 0.220124 , 0.725509 , 0.466226 },
		{ 0.226397 , 0.728888 , 0.462789 },
		{ 0.232815 , 0.732247 , 0.459277 },
		{ 0.239374 , 0.735588 , 0.455688 },
		{ 0.24607 , 0.73891 , 0.452024 },
		{ 0.252899 , 0.742211 , 0.448284 },
		{ 0.259857 , 0.745492 , 0.444467 },
		{ 0.266941 , 0.748751 , 0.440573 },
		{ 0.274149 , 0.751988 , 0.436601 },
		{ 0.281477 , 0.755203 , 0.432552 },
		{ 0.288921 , 0.758394 , 0.428426 },
		{ 0.296479 , 0.761561 , 0.424223 },
		{ 0.304148 , 0.764704 , 0.419943 },
		{ 0.311925 , 0.767822 , 0.415586 },
		{ 0.319809 , 0.770914 , 0.411152 },
		{ 0.327796 , 0.77398 , 0.40664 },
		{ 0.335885 , 0.777018 , 0.402049 },
		{ 0.344074 , 0.780029 , 0.397381 },
		{ 0.35236 , 0.783011 , 0.392636 },
		{ 0.360741 , 0.785964 , 0.387814 },
		{ 0.369214 , 0.788888 , 0.382914 },
		{ 0.377779 , 0.791781 , 0.377939 },
		{ 0.386433 , 0.794644 , 0.372886 },
		{ 0.395174 , 0.797475 , 0.367757 },
		{ 0.404001 , 0.800275 , 0.362552 },
		{ 0.412913 , 0.803041 , 0.357269 },
		{ 0.421908 , 0.805774 , 0.35191 },
		{ 0.430983 , 0.808473 , 0.346476 },
		{ 0.440137 , 0.811138 , 0.340967 },
		{ 0.449368 , 0.813768 , 0.335384 },
		{ 0.458674 , 0.816363 , 0.329727 },
		{ 0.468053 , 0.818921 , 0.323998 },
		{ 0.477504 , 0.821444 , 0.318195 },
		{ 0.487026 , 0.823929 , 0.312321 },
		{ 0.496615 , 0.826376 , 0.306377 },
		{ 0.506271 , 0.828786 , 0.300362 },
		{ 0.515992 , 0.831158 , 0.294279 },
		{ 0.525776 , 0.833491 , 0.288127 },
		{ 0.535621 , 0.835785 , 0.281908 },
		{ 0.545524 , 0.838039 , 0.275626 },
		{ 0.555484 , 0.840254 , 0.269281 },
		{ 0.565498 , 0.84243 , 0.262877 },
		{ 0.575563 , 0.844566 , 0.256415 },
		{ 0.585678 , 0.846661 , 0.249897 },
		{ 0.595839 , 0.848717 , 0.243329 },
		{ 0.606045 , 0.850733 , 0.236712 },
		{ 0.616293 , 0.852709 , 0.230052 },
		{ 0.626579 , 0.854645 , 0.223353 },
		{ 0.636902 , 0.856542 , 0.21662 },
		{ 0.647257 , 0.8584 , 0.209861 },
		{ 0.657642 , 0.860219 , 0.203082 },
		{ 0.668054 , 0.861999 , 0.196293 },
		{ 0.678489 , 0.863742 , 0.189503 },
		{ 0.688944 , 0.865448 , 0.182725 },
		{ 0.699415 , 0.867117 , 0.175971 },
		{ 0.709898 , 0.868751 , 0.169257 },
		{ 0.720391 , 0.87035 , 0.162603 },
		{ 0.730889 , 0.871916 , 0.156029 },
		{ 0.741388 , 0.873449 , 0.149561 },
		{ 0.751884 , 0.874951 , 0.143228 },
		{ 0.762373 , 0.876424 , 0.137064 },
		{ 0.772852 , 0.877868 , 0.131109 },
		{ 0.783315 , 0.879285 , 0.125405 },
		{ 0.79376 , 0.880678 , 0.120005 },
		{ 0.804182 , 0.882046 , 0.114965 },
		{ 0.814576 , 0.883393 , 0.110347 },
		{ 0.82494 , 0.88472 , 0.106217 },
		{ 0.83527 , 0.886029 , 0.102646 },
		{ 0.845561 , 0.887322 , 0.099702 },
		{ 0.85581 , 0.888601 , 0.097452 },
		{ 0.866013 , 0.889868 , 0.095953 },
		{ 0.876168 , 0.891125 , 0.09525 },
		{ 0.886271 , 0.892374 , 0.095374 },
		{ 0.89632 , 0.893616 , 0.096335 },
		{ 0.906311 , 0.894855 , 0.098125 },
		{ 0.916242 , 0.896091 , 0.100717 },
		{ 0.926106 , 0.89733 , 0.104071 },
		{ 0.935904 , 0.89857 , 0.108131 },
		{ 0.945636 , 0.899815 , 0.112838 },
		{ 0.9553 , 0.901065 , 0.118128 },
		{ 0.964894 , 0.902323 , 0.123941 },
		{ 0.974417 , 0.90359 , 0.130215 },
		{ 0.983868 , 0.904867 , 0.136897 },
		// special value
		{ 1 , 0 , 0 },
		//
	};

	UCHAR r, g, b;
	for (UINT paletteInc = 0; paletteInc < PICTURE_PALETTE_SIZE; paletteInc++)
	{
		// scaling it to make it a bit darker near the bottom.
		r = UCHAR(virida[paletteInc][0] * (255.0-1) * (1.0/4 + 3.0*paletteInc / (4*255.0)));
		g = UCHAR(virida[paletteInc][1] * (255.0 - 1) * (1.0 / 4 + 3.0*paletteInc / (4 * 255.0)));
		b = UCHAR(virida[paletteInc][2] * (255.0 - 1) * (1.0 / 4 + 3.0*paletteInc / (4 * 255.0)));
		Palette.aEntries[paletteInc].peRed = LOBYTE( r );
		Palette.aEntries[paletteInc].peGreen = LOBYTE( g );
		Palette.aEntries[paletteInc].peBlue = LOBYTE( b );
		Palette.aEntries[paletteInc].peFlags = PC_RESERVED;
	}
	palettes[0] = CreatePalette((LOGPALETTE *)&Palette );
	
	double inferno[256][3] = 
	{
		// special value
		{ 0 , 0 , 1 },
		// 
		{ 0.002267 , 0.00127 , 0.01857 },
		{ 0.003299 , 0.002249 , 0.024239 },
		{ 0.004547 , 0.003392 , 0.030909 },
		{ 0.006006 , 0.004692 , 0.038558 },
		{ 0.007676 , 0.006136 , 0.046836 },
		{ 0.009561 , 0.007713 , 0.055143 },
		{ 0.011663 , 0.009417 , 0.06346 },
		{ 0.013995 , 0.011225 , 0.071862 },
		{ 0.016561 , 0.013136 , 0.080282 },
		{ 0.019373 , 0.015133 , 0.088767 },
		{ 0.022447 , 0.017199 , 0.097327 },
		{ 0.025793 , 0.019331 , 0.10593 },
		{ 0.029432 , 0.021503 , 0.114621 },
		{ 0.033385 , 0.023702 , 0.123397 },
		{ 0.037668 , 0.025921 , 0.132232 },
		{ 0.042253 , 0.028139 , 0.141141 },
		{ 0.046915 , 0.030324 , 0.150164 },
		{ 0.051644 , 0.032474 , 0.159254 },
		{ 0.056449 , 0.034569 , 0.168414 },
		{ 0.06134 , 0.03659 , 0.177642 },
		{ 0.066331 , 0.038504 , 0.186962 },
		{ 0.071429 , 0.040294 , 0.196354 },
		{ 0.076637 , 0.041905 , 0.205799 },
		{ 0.081962 , 0.043328 , 0.215289 },
		{ 0.087411 , 0.044556 , 0.224813 },
		{ 0.09299 , 0.045583 , 0.234358 },
		{ 0.098702 , 0.046402 , 0.243904 },
		{ 0.104551 , 0.047008 , 0.25343 },
		{ 0.110536 , 0.047399 , 0.262912 },
		{ 0.116656 , 0.047574 , 0.272321 },
		{ 0.122908 , 0.047536 , 0.281624 },
		{ 0.129285 , 0.047293 , 0.290788 },
		{ 0.135778 , 0.046856 , 0.299776 },
		{ 0.142378 , 0.046242 , 0.308553 },
		{ 0.149073 , 0.045468 , 0.317085 },
		{ 0.15585 , 0.044559 , 0.325338 },
		{ 0.162689 , 0.043554 , 0.333277 },
		{ 0.169575 , 0.042489 , 0.340874 },
		{ 0.176493 , 0.041402 , 0.348111 },
		{ 0.183429 , 0.040329 , 0.354971 },
		{ 0.190367 , 0.039309 , 0.361447 },
		{ 0.197297 , 0.0384 , 0.367535 },
		{ 0.204209 , 0.037632 , 0.373238 },
		{ 0.211095 , 0.03703 , 0.378563 },
		{ 0.217949 , 0.036615 , 0.383522 },
		{ 0.224763 , 0.036405 , 0.388129 },
		{ 0.231538 , 0.036405 , 0.3924 },
		{ 0.238273 , 0.036621 , 0.396353 },
		{ 0.244967 , 0.037055 , 0.400007 },
		{ 0.25162 , 0.037705 , 0.403378 },
		{ 0.258234 , 0.038571 , 0.406485 },
		{ 0.26481 , 0.039647 , 0.409345 },
		{ 0.271347 , 0.040922 , 0.411976 },
		{ 0.27785 , 0.042353 , 0.414392 },
		{ 0.284321 , 0.043933 , 0.416608 },
		{ 0.290763 , 0.045644 , 0.418637 },
		{ 0.297178 , 0.04747 , 0.420491 },
		{ 0.303568 , 0.049396 , 0.422182 },
		{ 0.309935 , 0.051407 , 0.423721 },
		{ 0.316282 , 0.05349 , 0.425116 },
		{ 0.32261 , 0.055634 , 0.426377 },
		{ 0.328921 , 0.057827 , 0.427511 },
		{ 0.335217 , 0.06006 , 0.428524 },
		{ 0.3415 , 0.062325 , 0.429425 },
		{ 0.347771 , 0.064616 , 0.430217 },
		{ 0.354032 , 0.066925 , 0.430906 },
		{ 0.360284 , 0.069247 , 0.431497 },
		{ 0.366529 , 0.071579 , 0.431994 },
		{ 0.372768 , 0.073915 , 0.4324 },
		{ 0.379001 , 0.076253 , 0.432719 },
		{ 0.385228 , 0.078591 , 0.432955 },
		{ 0.391453 , 0.080927 , 0.433109 },
		{ 0.397674 , 0.083257 , 0.433183 },
		{ 0.403894 , 0.08558 , 0.433179 },
		{ 0.410113 , 0.087896 , 0.433098 },
		{ 0.416331 , 0.090203 , 0.432943 },
		{ 0.422549 , 0.092501 , 0.432714 },
		{ 0.428768 , 0.09479 , 0.432412 },
		{ 0.434987 , 0.097069 , 0.432039 },
		{ 0.441207 , 0.099338 , 0.431594 },
		{ 0.447428 , 0.101597 , 0.43108 },
		{ 0.453651 , 0.103848 , 0.430498 },
		{ 0.459875 , 0.106089 , 0.429846 },
		{ 0.4661 , 0.108322 , 0.429125 },
		{ 0.472328 , 0.110547 , 0.428334 },
		{ 0.478558 , 0.112764 , 0.427475 },
		{ 0.484789 , 0.114974 , 0.426548 },
		{ 0.491022 , 0.117179 , 0.425552 },
		{ 0.497257 , 0.119379 , 0.424488 },
		{ 0.503493 , 0.121575 , 0.423356 },
		{ 0.50973 , 0.123769 , 0.422156 },
		{ 0.515967 , 0.12596 , 0.420887 },
		{ 0.522206 , 0.12815 , 0.419549 },
		{ 0.528444 , 0.130341 , 0.418142 },
		{ 0.534683 , 0.132534 , 0.416667 },
		{ 0.54092 , 0.134729 , 0.415123 },
		{ 0.547157 , 0.136929 , 0.413511 },
		{ 0.553392 , 0.139134 , 0.411829 },
		{ 0.559624 , 0.141346 , 0.410078 },
		{ 0.565854 , 0.143567 , 0.408258 },
		{ 0.572081 , 0.145797 , 0.406369 },
		{ 0.578304 , 0.148039 , 0.404411 },
		{ 0.584521 , 0.150294 , 0.402385 },
		{ 0.590734 , 0.152563 , 0.40029 },
		{ 0.59694 , 0.154848 , 0.398125 },
		{ 0.603139 , 0.157151 , 0.395891 },
		{ 0.60933 , 0.159474 , 0.393589 },
		{ 0.615513 , 0.161817 , 0.391219 },
		{ 0.621685 , 0.164184 , 0.388781 },
		{ 0.627847 , 0.166575 , 0.386276 },
		{ 0.633998 , 0.168992 , 0.383704 },
		{ 0.640135 , 0.171438 , 0.381065 },
		{ 0.64626 , 0.173914 , 0.378359 },
		{ 0.652369 , 0.176421 , 0.375586 },
		{ 0.658463 , 0.178962 , 0.372748 },
		{ 0.66454 , 0.181539 , 0.369846 },
		{ 0.670599 , 0.184153 , 0.366879 },
		{ 0.676638 , 0.186807 , 0.363849 },
		{ 0.682656 , 0.189501 , 0.360757 },
		{ 0.688653 , 0.192239 , 0.357603 },
		{ 0.694627 , 0.195021 , 0.354388 },
		{ 0.700576 , 0.197851 , 0.351113 },
		{ 0.7065 , 0.200728 , 0.347777 },
		{ 0.712396 , 0.203656 , 0.344383 },
		{ 0.718264 , 0.206636 , 0.340931 },
		{ 0.724103 , 0.20967 , 0.337424 },
		{ 0.729909 , 0.212759 , 0.333861 },
		{ 0.735683 , 0.215906 , 0.330245 },
		{ 0.741423 , 0.219112 , 0.326576 },
		{ 0.747127 , 0.222378 , 0.322856 },
		{ 0.752794 , 0.225706 , 0.319085 },
		{ 0.758422 , 0.229097 , 0.315266 },
		{ 0.76401 , 0.232554 , 0.311399 },
		{ 0.769556 , 0.236077 , 0.307485 },
		{ 0.775059 , 0.239667 , 0.303526 },
		{ 0.780517 , 0.243327 , 0.299523 },
		{ 0.785929 , 0.247056 , 0.295477 },
		{ 0.791293 , 0.250856 , 0.29139 },
		{ 0.796607 , 0.254728 , 0.287264 },
		{ 0.801871 , 0.258674 , 0.283099 },
		{ 0.807082 , 0.262692 , 0.278898 },
		{ 0.812239 , 0.266786 , 0.274661 },
		{ 0.817341 , 0.270954 , 0.27039 },
		{ 0.822386 , 0.275197 , 0.266085 },
		{ 0.827372 , 0.279517 , 0.26175 },
		{ 0.832299 , 0.283913 , 0.257383 },
		{ 0.837165 , 0.288385 , 0.252988 },
		{ 0.841969 , 0.292933 , 0.248564 },
		{ 0.846709 , 0.297559 , 0.244113 },
		{ 0.851384 , 0.30226 , 0.239636 },
		{ 0.855992 , 0.307038 , 0.235133 },
		{ 0.860533 , 0.311892 , 0.230606 },
		{ 0.865006 , 0.316822 , 0.226055 },
		{ 0.869409 , 0.321827 , 0.221482 },
		{ 0.873741 , 0.326906 , 0.216886 },
		{ 0.878001 , 0.33206 , 0.212268 },
		{ 0.882188 , 0.337287 , 0.207628 },
		{ 0.886302 , 0.342586 , 0.202968 },
		{ 0.890341 , 0.347957 , 0.198286 },
		{ 0.894305 , 0.353399 , 0.193584 },
		{ 0.898192 , 0.358911 , 0.18886 },
		{ 0.902003 , 0.364492 , 0.184116 },
		{ 0.905735 , 0.37014 , 0.17935 },
		{ 0.90939 , 0.375856 , 0.174563 },
		{ 0.912966 , 0.381636 , 0.169755 },
		{ 0.916462 , 0.387481 , 0.164924 },
		{ 0.919879 , 0.393389 , 0.16007 },
		{ 0.923215 , 0.399359 , 0.155193 },
		{ 0.92647 , 0.405389 , 0.150292 },
		{ 0.929644 , 0.411479 , 0.145367 },
		{ 0.932737 , 0.417627 , 0.140417 },
		{ 0.935747 , 0.423831 , 0.13544 },
		{ 0.938675 , 0.430091 , 0.130438 },
		{ 0.941521 , 0.436405 , 0.125409 },
		{ 0.944285 , 0.442772 , 0.120354 },
		{ 0.946965 , 0.449191 , 0.115272 },
		{ 0.949562 , 0.45566 , 0.110164 },
		{ 0.952075 , 0.462178 , 0.105031 },
		{ 0.954506 , 0.468744 , 0.099874 },
		{ 0.956852 , 0.475356 , 0.094695 },
		{ 0.959114 , 0.482014 , 0.089499 },
		{ 0.961293 , 0.488716 , 0.084289 },
		{ 0.963387 , 0.495462 , 0.079073 },
		{ 0.965397 , 0.502249 , 0.073859 },
		{ 0.967322 , 0.509078 , 0.068659 },
		{ 0.969163 , 0.515946 , 0.063488 },
		{ 0.970919 , 0.522853 , 0.058367 },
		{ 0.97259 , 0.529798 , 0.053324 },
		{ 0.974176 , 0.53678 , 0.048392 },
		{ 0.975677 , 0.543798 , 0.043618 },
		{ 0.977092 , 0.55085 , 0.03905 },
		{ 0.978422 , 0.557937 , 0.034931 },
		{ 0.979666 , 0.565057 , 0.031409 },
		{ 0.980824 , 0.572209 , 0.028508 },
		{ 0.981895 , 0.579392 , 0.02625 },
		{ 0.982881 , 0.586606 , 0.024661 },
		{ 0.983779 , 0.593849 , 0.02377 },
		{ 0.984591 , 0.601122 , 0.023606 },
		{ 0.985315 , 0.608422 , 0.024202 },
		{ 0.985952 , 0.61575 , 0.025592 },
		{ 0.986502 , 0.623105 , 0.027814 },
		{ 0.986964 , 0.630485 , 0.030908 },
		{ 0.987337 , 0.63789 , 0.034916 },
		{ 0.987622 , 0.64532 , 0.039886 },
		{ 0.987819 , 0.652773 , 0.045581 },
		{ 0.987926 , 0.66025 , 0.05175 },
		{ 0.987945 , 0.667748 , 0.058329 },
		{ 0.987874 , 0.675267 , 0.065257 },
		{ 0.987714 , 0.682807 , 0.072489 },
		{ 0.987464 , 0.690366 , 0.07999 },
		{ 0.987124 , 0.697944 , 0.087731 },
		{ 0.986694 , 0.70554 , 0.095694 },
		{ 0.986175 , 0.713153 , 0.103863 },
		{ 0.985566 , 0.720782 , 0.112229 },
		{ 0.984865 , 0.728427 , 0.120785 },
		{ 0.984075 , 0.736087 , 0.129527 },
		{ 0.983196 , 0.743758 , 0.138453 },
		{ 0.982228 , 0.751442 , 0.147565 },
		{ 0.981173 , 0.759135 , 0.156863 },
		{ 0.980032 , 0.766837 , 0.166353 },
		{ 0.978806 , 0.774545 , 0.176037 },
		{ 0.977497 , 0.782258 , 0.185923 },
		{ 0.976108 , 0.789974 , 0.196018 },
		{ 0.974638 , 0.797692 , 0.206332 },
		{ 0.973088 , 0.805409 , 0.216877 },
		{ 0.971468 , 0.813122 , 0.227658 },
		{ 0.969783 , 0.820825 , 0.238686 },
		{ 0.968041 , 0.828515 , 0.249972 },
		{ 0.966243 , 0.836191 , 0.261534 },
		{ 0.964394 , 0.843848 , 0.273391 },
		{ 0.962517 , 0.851476 , 0.285546 },
		{ 0.960626 , 0.859069 , 0.29801 },
		{ 0.95872 , 0.866624 , 0.31082 },
		{ 0.956834 , 0.874129 , 0.323974 },
		{ 0.954997 , 0.881569 , 0.337475 },
		{ 0.953215 , 0.888942 , 0.351369 },
		{ 0.951546 , 0.896226 , 0.365627 },
		{ 0.950018 , 0.903409 , 0.380271 },
		{ 0.948683 , 0.910473 , 0.395289 },
		{ 0.947594 , 0.917399 , 0.410665 },
		{ 0.946809 , 0.924168 , 0.426373 },
		{ 0.946392 , 0.930761 , 0.442367 },
		{ 0.946403 , 0.937159 , 0.458592 },
		{ 0.946903 , 0.943348 , 0.47497 },
		{ 0.947937 , 0.949318 , 0.491426 },
		{ 0.949545 , 0.955063 , 0.50786 },
		{ 0.95174 , 0.960587 , 0.524203 },
		{ 0.954529 , 0.965896 , 0.540361 },
		{ 0.957896 , 0.971003 , 0.556275 },
		{ 0.961812 , 0.975924 , 0.571925 },
		{ 0.966249 , 0.980678 , 0.587206 },
		{ 0.971162 , 0.985282 , 0.602154 },
		{ 0.976511 , 0.989753 , 0.61676 },
		{ 0.982257 , 0.994109 , 0.631017 },
		// special value
		{ 1 , 0 , 0 }
		//
	};

	for (int paletteValueInc = 0; paletteValueInc < PICTURE_PALETTE_SIZE; paletteValueInc++)
	{
		// scaling it to make it a bit darker near the bottom.
		r = int( inferno[paletteValueInc][0] * 255.0 );
		g = int( inferno[paletteValueInc][1] * 255.0 );
		b = int( inferno[paletteValueInc][2] * 255.0 );
		Palette.aEntries[paletteValueInc].peRed = LOBYTE( r );
		Palette.aEntries[paletteValueInc].peGreen = LOBYTE( g );
		Palette.aEntries[paletteValueInc].peBlue = LOBYTE( b );
		Palette.aEntries[paletteValueInc].peFlags = PC_RESERVED;
	}

	palettes[1] = CreatePalette( (LOGPALETTE *)&Palette );
	///
	double blackToWhite[256][3];
	for (int paletteInc = 0; paletteInc < 256; paletteInc++)
	{
		blackToWhite[paletteInc][0] = paletteInc / 256.0;
		blackToWhite[paletteInc][1] = paletteInc / 256.0;
		blackToWhite[paletteInc][2] = paletteInc / 256.0;
	}
	// special values
	blackToWhite[0][0] = 0;
	blackToWhite[0][1] = 0;
	blackToWhite[0][2] = 1;
	// 	
	blackToWhite[255][0] = 1;
	blackToWhite[255][1] = 0;
	blackToWhite[255][2] = 0;
	for (int paletteValueInc = 0; paletteValueInc < PICTURE_PALETTE_SIZE; paletteValueInc++)
	{
		// scaling it to make it a bit darker near the bottom.
		r = int( blackToWhite[paletteValueInc][0] * 255.0 );
		g = int( blackToWhite[paletteValueInc][1] * 255.0 );
		b = int( blackToWhite[paletteValueInc][2] * 255.0 );
		Palette.aEntries[paletteValueInc].peRed = LOBYTE( r );
		Palette.aEntries[paletteValueInc].peGreen = LOBYTE( g );
		Palette.aEntries[paletteValueInc].peBlue = LOBYTE( b );
		Palette.aEntries[paletteValueInc].peFlags = PC_RESERVED;
	}

	palettes[2] = CreatePalette( (LOGPALETTE *)&Palette );
}

