//==============================================================================================
//
// Title:       InstallUninstallSoftwareComponents.c
// Purpose:     Installs or uninstalls individual software components from
//				a Real-Time target
//
//				Note: You may need to allow this example access to your network upon execution
//
//==============================================================================================

#include <ansi_c.h>
#include "nisyscfg.h"
#include "InstallUninstallSoftwareComponents.h"

#define AVAILABLE_SOFTWARE 0
#define INSTALLED_SOFTWARE 1

NISysCfgSessionHandle session = NULL;
NISysCfgStatus status = NISysCfg_OK;  
int panelHandle = 0;

int main(void)
{
	panelHandle = LoadPanel(0, "InstallUninstallSoftwareComponents.uir", PANEL);
	DisplayPanel(panelHandle);
	SetCtrlAttribute(panelHandle, PANEL_STATUS, ATTR_DIMMED, 1);
	RunUserInterface();
	DiscardPanel(panelHandle);
	return 0;
}

NISysCfgStatus UpdateSoftwareComponents(int table)
{
	/*	Update the software components tables, depending on the value of the table parameter	*/
	
	NISysCfgStatus status = NISysCfg_OK;
	NISysCfgEnumSoftwareComponentHandle componentsHandle = NULL;
	char softwareID[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char softwareVersion[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char softwareTitle[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	
	int col = 0;
	int row = 0;
	int numRows = 0;
	
	switch (table)
	{
		case AVAILABLE_SOFTWARE:
			//	Clear table to remove existing data
			GetNumTableRows(panelHandle, PANEL_AVAILABLECOMPONENTS, &numRows);
			if (numRows > 0)
				DeleteTableRows(panelHandle, PANEL_AVAILABLECOMPONENTS, 1, numRows);
			SetCtrlVal(panelHandle, PANEL_STATUS, "Getting Available Software"); 			
			if (NISysCfg_Succeeded(status = NISysCfgGetAvailableSoftwareComponents(session, NISysCfgIncludeItemsAllVisible, &componentsHandle)))
			{
				while (NISysCfg_Succeeded(status) && (status = NISysCfgNextComponentInfo(componentsHandle, softwareID, softwareVersion, softwareTitle, NULL, NULL)) == NISysCfg_OK)
				{
					row++;      
					InsertTableRows(panelHandle, PANEL_AVAILABLECOMPONENTS, row, 1, VAL_CELL_STRING);
					col = 1;
					SetTableCellAttribute(panelHandle, PANEL_AVAILABLECOMPONENTS, MakePoint(col, row), ATTR_CTRL_VAL, softwareTitle);
					col = 2;
					SetTableCellAttribute(panelHandle, PANEL_AVAILABLECOMPONENTS, MakePoint(col, row), ATTR_CTRL_VAL, softwareVersion);
					col = 3;
					SetTableCellAttribute(panelHandle, PANEL_AVAILABLECOMPONENTS, MakePoint(col, row), ATTR_CTRL_VAL, softwareID);
				}
			}
			break;
		case INSTALLED_SOFTWARE:
			//	Clear table to remove existing data
			GetNumTableRows(panelHandle, PANEL_INSTALLEDCOMPONENTS, &numRows);
			if (numRows > 0)
				DeleteTableRows(panelHandle, PANEL_INSTALLEDCOMPONENTS, 1, numRows);
			SetCtrlVal(panelHandle, PANEL_STATUS, "Getting Installed Software");   
			if (NISysCfg_Succeeded(status = NISysCfgGetInstalledSoftwareComponents(session, NISysCfgIncludeItemsAllVisible, NISysCfgBoolFalse, &componentsHandle)))
			{
				if (componentsHandle)
				{
					while (NISysCfg_Succeeded(status) && (status = NISysCfgNextComponentInfo(componentsHandle, softwareID, softwareVersion, softwareTitle, NULL, NULL)) == NISysCfg_OK)
					{
						row++;      
						InsertTableRows(panelHandle, PANEL_INSTALLEDCOMPONENTS, row, 1, VAL_CELL_STRING);
						col = 1;
						SetTableCellAttribute(panelHandle, PANEL_INSTALLEDCOMPONENTS, MakePoint(col, row), ATTR_CTRL_VAL, softwareTitle);
						col = 2;
						SetTableCellAttribute(panelHandle, PANEL_INSTALLEDCOMPONENTS, MakePoint(col, row), ATTR_CTRL_VAL, softwareVersion);
						col = 3;
						SetTableCellAttribute(panelHandle, PANEL_INSTALLEDCOMPONENTS, MakePoint(col, row), ATTR_CTRL_VAL, softwareID);
					}
				}
			}
			break;
		default:
			break;
	}				
	NISysCfgCloseHandle(componentsHandle);
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
	/*	Initialize System Configuration session and update software tables	*/
	
	if (event == EVENT_COMMIT)
	{
		char target[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char username[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char password[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char* detailedDescription = NULL;

		NISysCfgStatus status = NISysCfg_OK;

		SetCtrlAttribute(panelHandle, PANEL_STATUS, ATTR_DIMMED, 0);
		SetCtrlVal(panelHandle, PANEL_STATUS, "Initializing");
		SetWaitCursor(1);
		
		GetCtrlVal(panelHandle, PANEL_TARGET, target);
		GetCtrlVal(panelHandle, PANEL_USERNAME, username);
		GetCtrlVal(panelHandle, PANEL_PASSWORD, password);
		
		if (NISysCfg_Succeeded(status = NISysCfgInitializeSession(target, username, password, NISysCfgLocaleDefault, NISysCfgBoolTrue, 10000, NULL, &session)))
			if (NISysCfg_Succeeded(status = UpdateSoftwareComponents(AVAILABLE_SOFTWARE)))
				status = UpdateSoftwareComponents(INSTALLED_SOFTWARE);
		
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
	/*	Install specified software components and all dependencies, and update table	*/
	
	if (event == EVENT_COMMIT)
	{
		char softwareID[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char softwareVersion[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		NISysCfgEnumSoftwareComponentHandle componentsToInstall = NULL;
		char* detailedDescription = NULL; 
		char dependerTitle[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char dependerVersion[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char dependeeTitle[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char dependeeVersion[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		Point selectedCell;
		selectedCell.x = 0;
		selectedCell.y = 0;
		
		SetWaitCursor(1);
		SetCtrlVal(panelHandle, PANEL_STATUS, "Installing Software and all Dependencies");
		status = NISysCfgCreateComponentsEnum(&componentsToInstall);
		GetActiveTableCell(panelHandle, PANEL_AVAILABLECOMPONENTS, &selectedCell);
		
		//	Note: Multiple components can be added to NISysCfgEnumSoftwareComponentHandle
		//	This example only adds the component currently selected in the table and its dependencies
		GetTableCellAttribute(panelHandle, PANEL_AVAILABLECOMPONENTS, MakePoint(2, (selectedCell.y)), ATTR_CTRL_VAL, softwareVersion);
		GetTableCellAttribute(panelHandle, PANEL_AVAILABLECOMPONENTS, MakePoint(3, (selectedCell.y)), ATTR_CTRL_VAL, softwareID);
		if (NISysCfg_Succeeded(status = NISysCfgAddComponentToEnum(componentsToInstall, softwareID, softwareVersion, NISysCfgVersionSelectionExact)))
			if (NISysCfg_Succeeded(status = NISysCfgInstallUninstallComponents(session, NISysCfgBoolTrue, NISysCfgBoolTrue, componentsToInstall, 0, NULL, NULL)))
				status = UpdateSoftwareComponents(INSTALLED_SOFTWARE);
		
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

int  CVICALLBACK UninstallCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	/*	Uninstall software component and give option of uninstalling all dependencies, then update table	*/
	
	if (event == EVENT_COMMIT)
	{
		char* componentsToUninstall[100];
		char softwareID[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		int numComponentsToUninstall = 1;
		char* detailedDescription = NULL; 
		NISysCfgEnumDependencyHandle brokenDependencies = NULL;
		char dependerTitle[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char dependerVersion[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char dependerID[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char dependeeTitle[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char dependeeVersion[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char brokenDependenciesMsg[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char tempBuf[128] = "";
		Point selectedCell;
		selectedCell.x = 0;
		selectedCell.y = 0;
		
		SetCtrlVal(panelHandle, PANEL_STATUS, "Uninstalling Software");
		GetActiveTableCell(panelHandle, PANEL_INSTALLEDCOMPONENTS, &selectedCell);
		SetWaitCursor(1);

		GetTableCellAttribute(panelHandle, PANEL_INSTALLEDCOMPONENTS, MakePoint(3, (selectedCell.y)), ATTR_CTRL_VAL, softwareID);
		componentsToUninstall[0] = malloc(strlen(softwareID) * sizeof(char) + 1);
		strcpy(componentsToUninstall[0], softwareID);
		while ((status = NISysCfgInstallUninstallComponents(session, NISysCfgBoolTrue, NISysCfgBoolTrue, NULL, numComponentsToUninstall, componentsToUninstall, &brokenDependencies)) == NISysCfg_NotUninstallable)
		{
			status = NISysCfgNextDependencyInfo(brokenDependencies, NULL, NULL, NULL, NULL, NULL, dependeeVersion, dependeeTitle, NULL);
			sprintf(brokenDependenciesMsg, "The following components depend on %s %s:\n", dependeeTitle, dependeeVersion);
			status = NISysCfgResetEnumeratorGetCount(brokenDependencies, NULL);
			while ((status = NISysCfgNextDependencyInfo(brokenDependencies, dependerID, dependerVersion, dependerTitle, NULL, NULL, NULL, NULL, NULL)) == NISysCfg_OK)
			{
				componentsToUninstall[numComponentsToUninstall] = malloc(strlen(dependerID) + 1);
				strcpy(componentsToUninstall[numComponentsToUninstall], dependerID);
				numComponentsToUninstall++;
				sprintf(tempBuf, "%s %s\n", dependerTitle, dependerVersion);
				strcat(brokenDependenciesMsg, tempBuf);
			}
			sprintf(tempBuf, "Would you like to uninstall all of these components?\n");
			strcat(brokenDependenciesMsg, tempBuf);
			SetWaitCursor(0);
			if (!ConfirmPopup("Broken Dependencies", brokenDependenciesMsg))
				break;
			status = NISysCfgCloseHandle(brokenDependencies); 
			SetWaitCursor(1);
		}
			
		for (int i=0; i<numComponentsToUninstall; i++)
			free(componentsToUninstall[i]);
		
		status = UpdateSoftwareComponents(INSTALLED_SOFTWARE);
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

int  CVICALLBACK UninstallAllCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	/*	Uninstall all software components and update table	*/
	
	if (event == EVENT_COMMIT)
	{
		char* detailedDescription = NULL;
		
		SetWaitCursor(1);
		SetCtrlVal(panelHandle, PANEL_STATUS, "Uninstalling All Software");
		if (NISysCfg_Succeeded(status = NISysCfgUninstallAll(session, NISysCfgBoolTrue)))
			status = UpdateSoftwareComponents(INSTALLED_SOFTWARE);
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


