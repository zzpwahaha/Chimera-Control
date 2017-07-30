#include "stdafx.h"
#include "SMSTextingControl.h"
#include "externals.h"
#include "commctrl.h"
#include "constants.h"
//#include "dialogProcedures.h"
#include <algorithm>
#include "reorganizeControl.h"
#include <Algorithm>
#include "textPromptDialogProcedure.h"

void SMSTextingControl::promptForEmailAddressAndPassword()
{
	/// TODO
	TextPromptDialog dialog(&emailAddress, "Please enter an email address:");
	dialog.DoModal();	
	TextPromptDialog dialog2(&password, "Please enter a password:");
	dialog2.DoModal();
}


void SMSTextingControl::rearrange(int width, int height, fontMap fonts)
{
	title.rearrange("", "", width, height, fonts);
	peopleListView.rearrange("", "", width, height, fonts);	
}


void SMSTextingControl::initializeControls(POINT& pos, CWnd* parent, bool isTriggerModeSensitive, int& id, 
										   fontMap fonts, std::vector<CToolTipCtrl*>& tooltips)
{
	title.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 25 };
	title.ID = id++;
	title.Create("TEXT ME", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY | WS_BORDER, title.sPos, parent, title.ID);
	title.fontType = Heading;
	title.SetFont(fonts["Normal Font"]);

	peopleListView.sPos = { pos.x, pos.y + 25, pos.x + 480, pos.y + 120 };
	peopleListView.ID = id++;
	idVerify(peopleListView.ID, IDC_SMS_TEXTING_LISTVIEW);
	peopleListView.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS, peopleListView.sPos, parent, peopleListView.ID);
	peopleListView.fontType = Small;
	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
	// Type of mask
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	// width between each coloum
	listViewDefaultCollumn.cx = 0x42;
	listViewDefaultCollumn.pszText = "Person";
	// Inserting Couloms as much as we want
	peopleListView.InsertColumn(0, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Phone #";
	peopleListView.InsertColumn(1, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Carrier";
	peopleListView.InsertColumn(2, &listViewDefaultCollumn);
	listViewDefaultCollumn.cx = 0x62;
	listViewDefaultCollumn.pszText = "At Finish?";
	peopleListView.InsertColumn(3, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "If No Loading?";
	peopleListView.InsertColumn(4, &listViewDefaultCollumn);
	// Make First Blank row.
	LVITEM listViewDefaultItem;
	memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
	listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
	listViewDefaultItem.cchTextMax = 256; // Max size of test
	listViewDefaultItem.pszText = "___";
	listViewDefaultItem.iItem = 0;          // choose item  
	listViewDefaultItem.iSubItem = 0;       // Put in first coluom
	peopleListView.InsertItem(&listViewDefaultItem);
	for (int itemInc = 1; itemInc < 3; itemInc++) // Add SubItems in a loop
	{
		listViewDefaultItem.iSubItem = itemInc;
		peopleListView.SetItem(&listViewDefaultItem);
	}
	listViewDefaultItem.iSubItem = 3;
	listViewDefaultItem.pszText = "No";
	peopleListView.SetItem(&listViewDefaultItem);
	listViewDefaultItem.iSubItem = 4;
	peopleListView.SetItem(&listViewDefaultItem);
	pos.y += 120;
}


void SMSTextingControl::updatePersonInfo()
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	peopleListView.ScreenToClient(&cursorPos);

	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator;
	int subitemIndicator;
	peopleListView.SubItemHitTest(&myItemInfo);
	itemIndicator = myItemInfo.iItem;
	subitemIndicator = myItemInfo.iSubItem;
	if (itemIndicator == -1)
	{
		// user didn't click in an item.
		return;
	}
	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	listViewItem.mask = LVIF_TEXT;   // Text Style
	listViewItem.cchTextMax = 256; // Max size of test
	
	listViewItem.iItem = itemIndicator;          // choose item  
	/// check if adding new person
	if (itemIndicator == peopleToText.size())
	{
		// add a person
		peopleToText.resize(peopleToText.size() + 1);
		peopleToText.back().textWhenComplete = false;
		// add an item to the control
		listViewItem.pszText = "___";
		listViewItem.iItem = itemIndicator;          // choose item  
		listViewItem.iSubItem = 0;       // Put in first coluom
		peopleListView.InsertItem(&listViewItem);
		for (int itemInc = 1; itemInc < 3; itemInc++) // Add SubItems in a loop
		{
			listViewItem.iSubItem = itemInc;
			// Enter text to SubItems
			peopleListView.SetItem(&listViewItem);
		}
		// default texting options is no!
		listViewItem.iSubItem = 3;
		listViewItem.pszText = "No";
		peopleListView.SetItem(&listViewItem);
		listViewItem.iSubItem = 4;
		peopleListView.SetItem(&listViewItem);
	}
	/// Handle different subitem clicks
	switch (subitemIndicator)
	{
		case 0:
		{
			/// person name
			// prompt for a name
			std::string newName;
			TextPromptDialog dialog(&newName, "Please enter a name:");
			dialog.DoModal();

			// update the info inside
			peopleToText[itemIndicator].name = newName;
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			listViewItem.pszText = (LPSTR)newName.c_str();
			peopleListView.SetItem(&listViewItem);
			break;
		}
		case 1:
		{
			/// number
			// Prompt for a phone number			
			std::string phoneNumber;
			TextPromptDialog dialog(&phoneNumber, "Please enter a phone number (numbers only!):");
			dialog.DoModal();

			// test to see if it is a number like it should be (note: not sure if this catches a single decimal or not...)
			try
			{
				long long test = std::stoll(phoneNumber);
			}
			catch (std::invalid_argument&)
			{
				thrower("Numbers only, please!");
			}
			catch (std::out_of_range&)
			{
				thrower("number was too long...");
			}
			peopleToText[itemIndicator].number = phoneNumber;
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			listViewItem.pszText = (LPSTR)phoneNumber.c_str();
			peopleListView.SetItem(&listViewItem);
			break;
		}
		case 2:
		{
			/// provider
			// Prompt for provider
			std::string newProvider;
			TextPromptDialog dialog(&newProvider, 
									"Please enter provider (either \"verizon\", \"tmobile\", \"at&t\", or "
									"\"googlefi\"):");
			dialog.DoModal();
			std::transform(newProvider.begin(), newProvider.end(), newProvider.begin(), ::tolower);
			if (newProvider != "verizon" && newProvider != "tmobile" && newProvider != "at&t" && newProvider != "googlefi")
			{
				MessageBox(0, "Please enter either \"verizon\", \"tmobile\", \"at&t\", or \"googlefi\" (not case sensitive)", 0, 0);
				break;
			}
			peopleToText[itemIndicator].provider = newProvider;
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			listViewItem.pszText = (LPSTR)newProvider.c_str();
			peopleListView.SetItem(&listViewItem);
			break;
		}
		case 3:
		{
			/// at finish?
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			// this is just a binary switch.
			if (peopleToText[itemIndicator].textWhenComplete)
			{
				peopleToText[itemIndicator].textWhenComplete = false;
				listViewItem.pszText = "No";
			}
			else
			{
				peopleToText[itemIndicator].textWhenComplete = true;
				listViewItem.pszText = "Yes";
			}
			peopleListView.SetItem(&listViewItem);
			break;
		}	
		case 4:
		{
			/// at finish?
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			// this is just a binary switch.
			if (peopleToText[itemIndicator].textIfLoadingStops)
			{
				peopleToText[itemIndicator].textIfLoadingStops = false;
				listViewItem.pszText = "No";
			}
			else
			{
				peopleToText[itemIndicator].textIfLoadingStops = true;
				listViewItem.pszText = "Yes";
			}
			peopleListView.SetItem(&listViewItem);
			break;
		}
	}
}


void SMSTextingControl::deletePersonInfo()
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	peopleListView.ScreenToClient(&cursorPos);

	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator;
	peopleListView.SubItemHitTest(&myItemInfo);
	itemIndicator = myItemInfo.iItem;
	if (itemIndicator == -1 || itemIndicator == peopleToText.size())
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer = MessageBox(0, ("Delete info for " + peopleToText[itemIndicator].name + "?").c_str(), 0, MB_YESNO);
	if (answer == IDYES)
	{
		peopleListView.DeleteItem(itemIndicator);
		peopleToText.erase(peopleToText.begin() + itemIndicator);
	}
}


void SMSTextingControl::sendMessage(std::string message, EmbeddedPythonHandler* pyHandler, std::string msgType)
{
	if (msgType == "Loading")
	{
		for (personInfo& person : this->peopleToText)
		{
			if (person.textIfLoadingStops)
			{
				// send text gives an appropriate error message.
				pyHandler->sendText( person, message, "Not Loading Atoms", this->emailAddress, this->password );
			}
		}
	}
	else if (msgType == "Finished")
	{
		for (personInfo& person : this->peopleToText)
		{
			if (person.textWhenComplete)
			{
				// send text gives an appropriate error message.
				pyHandler->sendText(person, message, "Experiment Finished", this->emailAddress, this->password);
			}
		}
	}
	else
	{
		thrower("ERROR: unrecognized text message type: " + msgType);
	}
}


void SMSTextingControl::rearrange(RECT parentRectangle, std::string mode)
{
	//reorganizeControl(title, mode, parentRectangle);
	//reorganizeControl(peopleListView, mode, parentRectangle);
}
