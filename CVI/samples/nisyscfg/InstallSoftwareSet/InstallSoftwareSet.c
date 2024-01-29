//==============================================================================================
//
// Title:       InstallSoftwareSet.c
// Purpose:     Installs a software set to an RT Target
//
//				Note: You may need to allow this example access to your network upon execution
//
//==============================================================================================

#include <formatio.h>
#include <ansi_c.h>
#include "nisyscfg.h"
#include "InstallSoftwareSet.h"

#define AVAILABLE_SOFTWARE 0
#define INSTALLED_SOFTWARE 1

NISysCfgSessionHandle session = NULL;
NISysCfgStatus status = NISysCfg_OK;
int panelHandle = 0;

int main(void)
{
	panelHandle = LoadPanel(0, "InstallSoftwareSet.uir", PANEL);
	DisplayPanel(panelHandle);
	SetCtrlAttribute(panelHandle, PANEL_STATUS, ATTR_DIMMED, 1);
	SetCtrlAttribute(panelHandle, PANEL_INSTALLEDSET, ATTR_DIMMED, 1);
	RunUserInterface();
	DiscardPanel(panelHandle);
	return 0;
}


NISysCfgStatus UpdateSoftwareSets(int table)
{
	/*	Update the software sets table or text box, depending on the value of the table	parameter	*/

	NISysCfgStatus status = NISysCfg_OK;
	NISysCfgEnumSoftwareSetHandle softwareSetsEnum = NULL;
	NISysCfgSoftwareSetHandle softwareSet = NULL;
	NISysCfgEnumSoftwareComponentHandle softwareComponents = NULL;
	char softwareID[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char softwareTitle[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char labviewRTVersion[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char installedSetText[NISYSCFG_SIMPLE_STRING_LENGTH] = "";

	int col = 0;
	int row = 0;
	int numRows = 0;

	switch (table)
	{
		case AVAILABLE_SOFTWARE:
			//	Clear table to remove existing data
			GetNumTableRows(panelHandle, PANEL_AVAILABLESETS, &numRows);
			if (numRows > 0)
				DeleteTableRows(panelHandle, PANEL_AVAILABLESETS, 1, numRows);
			SetCtrlVal(panelHandle, PANEL_STATUS, "Getting Available Software Sets");
			if (NISysCfg_Succeeded(status = NISysCfgGetAvailableSoftwareSets(session, &softwareSetsEnum)) && softwareSetsEnum)
			{
				while (NISysCfg_Succeeded(status) && (status = NISysCfgNextSoftwareSet(softwareSetsEnum, &softwareSet)) == NISysCfg_OK)
				{
					row++;
					InsertTableRows(panelHandle, PANEL_AVAILABLESETS, row, 1, VAL_CELL_STRING);
					NISysCfgGetSoftwareSetInfo(softwareSet, NISysCfgIncludeItemsAllVisible, NISysCfgBoolFalse, softwareID, NULL, softwareTitle, NULL, NULL, NULL, &softwareComponents);
					col = 1;
					SetTableCellAttribute(panelHandle, PANEL_AVAILABLESETS, MakePoint(col, row), ATTR_CTRL_VAL, softwareTitle);
					//	Find version of LabVIEW RT installed with software set
					while (NISysCfg_Succeeded(status) && (NISysCfgNextComponentInfo(softwareComponents, NULL, labviewRTVersion, softwareTitle, NULL, NULL)) == NISysCfg_OK)
					{
						if (CompareStrings(softwareTitle, 0, "LabVIEW Real-Time", 0, 0) == 0)
							break;
					}
					col = 2;
					SetTableCellAttribute(panelHandle, PANEL_AVAILABLESETS, MakePoint(col, row), ATTR_CTRL_VAL, labviewRTVersion);
					col = 3;
					SetTableCellAttribute(panelHandle, PANEL_AVAILABLESETS, MakePoint(col, row), ATTR_CTRL_VAL, softwareID);
					NISysCfgCloseHandle(softwareComponents);
					NISysCfgCloseHandle(softwareSet);
				}
				NISysCfgCloseHandle(softwareSetsEnum);
			}
			else
				MessagePopup("Get Available Software Sets Error", "Selected target may not support installation of software sets");
			break;
		case INSTALLED_SOFTWARE:
			//	Clear text box to remove existing data
			ResetTextBox(panelHandle, PANEL_INSTALLEDSET, "");
			SetCtrlVal(panelHandle, PANEL_STATUS, "Getting Installed Software Set");
			if (NISysCfg_Succeeded(status = NISysCfgGetInstalledSoftwareSet(session, NISysCfgBoolFalse, &softwareSet)))
			{
				if (softwareSet)
				{
					status = NISysCfgGetSoftwareSetInfo(softwareSet, NISysCfgIncludeItemsAllVisible, NISysCfgBoolFalse, NULL, NULL, softwareTitle, NULL, NULL, NULL, &softwareComponents);
					sprintf(installedSetText, "%s \n", softwareTitle);
					//	Find version of LabVIEW RT installed with software set
					while (NISysCfg_Succeeded(status) && (NISysCfgNextComponentInfo(softwareComponents, NULL, labviewRTVersion, softwareTitle, NULL, NULL)) == NISysCfg_OK)
					{
						if (CompareStrings(softwareTitle, 0, "LabVIEW Real-Time", 0, 0) == 0)
							break;
					}
					strcat(installedSetText, softwareTitle);
					strcat(installedSetText, " ");
					strcat(installedSetText, labviewRTVersion);
					InsertTextBoxLine(panelHandle, PANEL_INSTALLEDSET, -1, installedSetText);
					NISysCfgCloseHandle(softwareComponents);
					NISysCfgCloseHandle(softwareSet);
				}
				else
					InsertTextBoxLine(panelHandle, PANEL_INSTALLEDSET, -1, "No Software Sets Installed");
			}
			break;
		default:
			break;
	}
	return status;
}

int  CVICALLBACK PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	/*	Close System Configuration session and quit	*/

	if (event == EVENT_CLOSE)
	{
		status = NISysCfgCloseHandle(session);
		QuitUserInterface(0);
	}
	return 0;
}

int  CVICALLBACK GoCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	/*	Initialize System Configuration session and update software table and text box	*/

	if (event == EVENT_COMMIT)
	{
		char target[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char username[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char password[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char* detailedDescription = NULL;

		NISysCfgStatus status = NISysCfg_OK;

		SetCtrlAttribute(panelHandle, PANEL_STATUS, ATTR_DIMMED, 0);
		SetCtrlAttribute(panelHandle, PANEL_INSTALLEDSET, ATTR_DIMMED, 0);
		SetCtrlVal(panelHandle, PANEL_STATUS, "Initializing");
		SetWaitCursor(1);

		GetCtrlVal(panelHandle, PANEL_TARGET, target);
		GetCtrlVal(panelHandle, PANEL_USERNAME, username);
		GetCtrlVal(panelHandle, PANEL_PASSWORD, password);

		if (NISysCfg_Succeeded(status = NISysCfgInitializeSession(target, username, password, NISysCfgLocaleDefault, NISysCfgBoolTrue, 10000, NULL, &session)))
			if (NISysCfg_Succeeded(status = UpdateSoftwareSets(AVAILABLE_SOFTWARE)))
				status = UpdateSoftwareSets(INSTALLED_SOFTWARE);

		SetWaitCursor(0);
		if (NISysCfg_Failed(status))
		{
			NISysCfgGetStatusDescription(session, status, &detailedDescription);
			MessagePopup("System Configuration Error", detailedDescription);
			NISysCfgFreeDetailedString(detailedDescription);
			//	Close session if we received some error
			status = NISysCfgCloseHandle(session);
		}
		SetCtrlVal(panelHandle, PANEL_STATUS, "Waiting on user");
	}
	return 0;
}

int  CVICALLBACK InstallCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	/*	Install specified software set and update text box	*/

	if (event == EVENT_COMMIT)
	{
		char softwareID[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char softwareVersion[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		NISysCfgEnumSoftwareComponentHandle componentsToInstall = NULL;
		char* detailedDescription = NULL;
		Point selectedCell;
		selectedCell.x = 0;
		selectedCell.y = 0;

		SetWaitCursor(1);
		SetCtrlVal(panelHandle, PANEL_STATUS, "Installing Software Set");
		status = NISysCfgCreateComponentsEnum(&componentsToInstall);
		GetActiveTableCell(panelHandle, PANEL_AVAILABLESETS, &selectedCell);
		GetTableCellAttribute(panelHandle, PANEL_AVAILABLESETS, MakePoint(2, (selectedCell.y)), ATTR_CTRL_VAL, softwareVersion);
		GetTableCellAttribute(panelHandle, PANEL_AVAILABLESETS, MakePoint(3, (selectedCell.y)), ATTR_CTRL_VAL, softwareID);
		if (NISysCfg_Succeeded(status = NISysCfgInstallSoftwareSet(session, NISysCfgBoolTrue, softwareID, NULL, NULL, NULL)))
			status = UpdateSoftwareSets(INSTALLED_SOFTWARE);

		SetWaitCursor(0);
		if (NISysCfg_Failed(status))
		{
			NISysCfgGetStatusDescription(session, status, &detailedDescription);
			MessagePopup("System Configuration Error", detailedDescription);
			NISysCfgFreeDetailedString(detailedDescription);
		}
		SetCtrlVal(panelHandle, PANEL_STATUS, "Waiting on user");
		status = NISysCfgCloseHandle(componentsToInstall);
	}
	return 0;
}

int  CVICALLBACK UninstallAllCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	/*	Uninstall all software and update text box	*/

	if (event == EVENT_COMMIT)
	{
		char* detailedDescription = NULL;

		SetWaitCursor(1);
		SetCtrlVal(panelHandle, PANEL_STATUS, "Uninstalling All Software");
		if (NISysCfg_Succeeded(status = NISysCfgUninstallAll(session, NISysCfgBoolTrue)))
			status = UpdateSoftwareSets(INSTALLED_SOFTWARE);
		SetWaitCursor(0);
		if (NISysCfg_Failed(status))
		{
			NISysCfgGetStatusDescription(session, status, &detailedDescription);
			MessagePopup("System Configuration Error", detailedDescription);
			NISysCfgFreeDetailedString(detailedDescription);
		}
		SetCtrlVal(panelHandle, PANEL_STATUS, "Waiting on user");
	}
	return 0;
}

int  CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	/*	Close System Configuration session and quit	*/

	if (event == EVENT_COMMIT)
	{
		status = NISysCfgCloseHandle(session);
		QuitUserInterface(0);
	}
	return 0;
}
