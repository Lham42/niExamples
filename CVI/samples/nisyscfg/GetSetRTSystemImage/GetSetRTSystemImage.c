//==============================================================================================
//
// Title:       GetSetRTSystemImage.c
// Purpose:     Obtains an image from a Real-Time target, or re-images a Real-Time target
//				from a stored image
//
//				Note: You may need to allow this example access to your network upon execution
//
//==============================================================================================

#include <ansi_c.h>
#include "nisyscfg.h"
#include "pwctrl.h"
#include "GetSetRTSystemImage.h"

NISysCfgSessionHandle session = NULL;
NISysCfgStatus status = NISysCfg_OK;
int panelHandle = 0;

int main(void)
{
	panelHandle = LoadPanel(0, "GetSetRTSystemImage.uir", PANEL);
	PasswordCtrl_ConvertFromString (panelHandle, PANEL_PASSWORD);
	DisplayPanel(panelHandle);
	SetCtrlAttribute(panelHandle, PANEL_STATUS, ATTR_DIMMED, 1);
	RunUserInterface();
	DiscardPanel(panelHandle);
	return 0;
}

int ClearTable(void)
{
	//	Clear table to remove existing data
	int numRows = 0;
	GetNumTableRows(panelHandle, PANEL_INSTALLEDCOMPONENTS, &numRows);
	if (numRows > 0)
		DeleteTableRows(panelHandle, PANEL_INSTALLEDCOMPONENTS, 1, numRows);
	return 0;
}
			 
int  CVICALLBACK GoCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	if (event == EVENT_COMMIT)
	{
		char target[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char username[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char password[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char filePath[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char softwareVersion[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char softwareTitle[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char encryptionPassphrase[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char* detailedDescription = NULL;
		int imageMode = -1;
		int overwrite = -1;
		int col = 0;
		int row = 0;
		NISysCfgEnumSoftwareComponentHandle componentsHandle = NULL;
		
		GetCtrlVal(panelHandle, PANEL_TARGET, target);
		GetCtrlVal(panelHandle, PANEL_USERNAME, username);
		GetCtrlVal(panelHandle, PANEL_PASSWORD, password);
		GetCtrlVal(panelHandle, PANEL_FILEPATH, filePath);
		GetCtrlVal(panelHandle, PANEL_ENCRYPTION, encryptionPassphrase);
		GetCtrlVal(panelHandle, PANEL_GETSET, &imageMode);
		GetCtrlVal(panelHandle, PANEL_OVERWRITE, &overwrite);
		
		SetCtrlAttribute(panelHandle, PANEL_STATUS, ATTR_DIMMED, 0);
		
		SetWaitCursor(1);
		SetCtrlVal(panelHandle, PANEL_STATUS, "Initializing");
		if (NISysCfg_Succeeded(status = NISysCfgInitializeSession(target, username, password, NISysCfgLocaleDefault, NISysCfgBoolTrue, 10000, NULL, &session)))
			switch (imageMode)
			{
				case 0: //	Get Image
					SetCtrlVal(panelHandle, PANEL_STATUS, "Obtaining Image From Target");
					status = NISysCfgGetSystemImageAsFolder2(session, NISysCfgBoolTrue, filePath, encryptionPassphrase, 0, NULL, overwrite, NISysCfgBoolFalse);
					ClearTable();
					SetCtrlVal(panelHandle, PANEL_STATUS, "Getting Installed Software");
					if (NISysCfg_Succeeded(status) && (status = NISysCfgGetInstalledSoftwareComponents(session, NISysCfgIncludeItemsAllVisible, NISysCfgBoolFalse, &componentsHandle)) == NISysCfg_OK)
					{
						while ((status = NISysCfgNextComponentInfo(componentsHandle, NULL, softwareVersion, softwareTitle, NULL, NULL)) == NISysCfg_OK)
						{
							row++;
							InsertTableRows(panelHandle, PANEL_INSTALLEDCOMPONENTS, row, 1, VAL_CELL_STRING);
							col = 1;
							SetTableCellAttribute(panelHandle, PANEL_INSTALLEDCOMPONENTS, MakePoint(col, row), ATTR_CTRL_VAL, softwareTitle);
							col = 2;
							SetTableCellAttribute(panelHandle, PANEL_INSTALLEDCOMPONENTS, MakePoint(col, row), ATTR_CTRL_VAL, softwareVersion);
						}
					}
					break;
				case 1: //	Set Image
					SetCtrlVal(panelHandle, PANEL_STATUS, "Reimaging Target");
					status = NISysCfgSetSystemImageFromFolder2 (session, NISysCfgBoolTrue, filePath, encryptionPassphrase, 0, NULL, NISysCfgBoolFalse, NISysCfgApplyPrimaryApplyOthers);
					ClearTable();
					SetCtrlVal(panelHandle, PANEL_STATUS, "Getting Installed Software");
					if (NISysCfg_Succeeded(status) && (status = NISysCfgGetInstalledSoftwareComponents(session, NISysCfgIncludeItemsAllVisible, NISysCfgBoolFalse, &componentsHandle)) == NISysCfg_OK)
					{
						while ((status = NISysCfgNextComponentInfo(componentsHandle, NULL, softwareVersion, softwareTitle, NULL, NULL)) == NISysCfg_OK)
						{
							row++;
							InsertTableRows(panelHandle, PANEL_INSTALLEDCOMPONENTS, row, 1, VAL_CELL_STRING);
							col = 1;
							SetTableCellAttribute(panelHandle, PANEL_INSTALLEDCOMPONENTS, MakePoint(col, row), ATTR_CTRL_VAL, softwareTitle);
							col = 2;
							SetTableCellAttribute(panelHandle, PANEL_INSTALLEDCOMPONENTS, MakePoint(col, row), ATTR_CTRL_VAL, softwareVersion);
						}
					}
					break;
				default:
					break;
			}
		SetWaitCursor(0);
		if (NISysCfg_Failed(status))
		{
			NISysCfgGetStatusDescription(session, status, &detailedDescription);
			MessagePopup("System Configuration Error", detailedDescription);
			NISysCfgFreeDetailedString(detailedDescription);
		}
		NISysCfgCloseHandle(componentsHandle);
		SetCtrlVal(panelHandle, PANEL_STATUS, "Waiting on User");
	}
	return 0;
}
	
int  CVICALLBACK PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	if (event == EVENT_CLOSE)
	{
		status = NISysCfgCloseHandle(session);
		QuitUserInterface(0);
	}
	return 0;
}

int  CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	if (event == EVENT_COMMIT)
	{
		status = NISysCfgCloseHandle(session);
		QuitUserInterface(0);
	}
	return 0;
}
