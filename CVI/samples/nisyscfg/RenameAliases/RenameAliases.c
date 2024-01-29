//==============================================================================
//
// Title:       RenameAliases.c
// Purpose:     Displays all DAQmx and VISA-based hardware on a given system, 
//				then allows the user to rename the alias of specified devices.
//																
//==============================================================================

#include <ansi_c.h>
#include "nisyscfg.h"
#include "RenameAliases.h"

#define NO_INIT 0
#define INIT 1
					
NISysCfgStatus status = NISysCfg_OK;
NISysCfgSessionHandle session = NULL;					
NISysCfgEnumResourceHandle resourcesHandle = NULL;

char* detailedDescription = NULL;
int panelHandle = 0;
char target[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
char newAlias[NISYSCFG_SIMPLE_STRING_LENGTH] = "";

int main(void)
{
	panelHandle = LoadPanel(0, "RenameAliases.uir", PANEL);
	DisplayPanel(panelHandle);
	SetCtrlAttribute(panelHandle, PANEL_SUBMIT_CHANGES, ATTR_DIMMED, 1);
	RunUserInterface();
	DiscardPanel(panelHandle);
	return 0;
}

	/*  int UpdateTable(int init)																*/
	/*	Updates table in UI																		*/
	/* 	If init=1, initialize System Configuration session and populate Configuration table		*/
	/*	If init=0, update Configuration table with new aliases									*/

int UpdateTable(int init)
{		
	NISysCfgResourceHandle resource = NULL;  
	NISysCfgFilterHandle filter = NULL;  
	
	char productName[NISYSCFG_SIMPLE_STRING_LENGTH] = ""; 
	char resourceName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char alias[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	int col = 1;
	int row = 1;

	SetWaitCursor(1);
	
	if (init)
	{
		if (NISysCfg_Succeeded(status = NISysCfgInitializeSession(target, NULL, NULL, NISysCfgLocaleDefault, NISysCfgBoolTrue, 10000, NULL, &session)))
			if (NISysCfg_Succeeded(status = NISysCfgCreateFilter(session, &filter)))
			{
				NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyIsDevice, NISysCfgBoolTrue);
				if (NISysCfg_Succeeded(status = NISysCfgFindHardware(session, NISysCfgFilterModeAll, filter, "daqmx,ni-visa", &resourcesHandle)))
					status = NISysCfgCloseHandle(filter);
			}
	}
	else
		status = NISysCfgResetEnumeratorGetCount(resourcesHandle, NULL);
	
	if (resourcesHandle)
	{
		//	Obtain hardware properties and update Configuration_Table
		while (NISysCfg_Succeeded(status) && (status = NISysCfgNextResource(session, resourcesHandle, &resource)) == NISysCfg_OK)
		{
			if (init)
				InsertTableRows(panelHandle, PANEL_CONFIGURATION_TABLE, row, 1, VAL_CELL_STRING);
	
			if (NISysCfg_Failed(status = NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyProductName, productName)))
				break;
			if (NISysCfg_Failed(status = NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertResourceName, 0, resourceName)))
				break;  
			if (NISysCfg_Failed(status = NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertUserAlias, 0, alias)))
				break;
		
			col = 1;
			if (strlen(productName))
				SetTableCellAttribute(panelHandle, PANEL_CONFIGURATION_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, productName);
			else
				SetTableCellAttribute(panelHandle, PANEL_CONFIGURATION_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, resourceName);
	
			col = 2;
			if (strlen(alias))
				SetTableCellAttribute(panelHandle, PANEL_CONFIGURATION_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, alias);
			else
				SetTableCellAttribute(panelHandle, PANEL_CONFIGURATION_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, resourceName);
		
			status = NISysCfgCloseHandle(resource);
			row++;
		}		 
	}
	else
		MessagePopup("No Devices Found", "No valid devices were found on the specified system");
	
	SetWaitCursor(0);
	if (NISysCfg_Failed(status))
	{
		NISysCfgGetStatusDescription(session, status, &detailedDescription);
		MessagePopup("System Configuration Error", detailedDescription);
		NISysCfgFreeDetailedString(detailedDescription);
		return 0;
	}
	return 1;
}
							   
int CVICALLBACK PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	/* 	Close System Configuration session and quit	*/
	
	if(event == EVENT_CLOSE)
	{
		status = NISysCfgCloseHandle(resourcesHandle);
		status = NISysCfgCloseHandle(session);
		QuitUserInterface(0);
	}
	return 0;
}

int CVICALLBACK GoCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	/*	Initialize table for given target	*/
	
	if(event == EVENT_COMMIT)
	{
		//	Clear table to remove existing data
		int numRows = 0;
		GetNumTableRows(panelHandle, PANEL_CONFIGURATION_TABLE, &numRows);
		if (numRows > 0)
			DeleteTableRows(panelHandle, PANEL_CONFIGURATION_TABLE, 1, numRows);
		GetCtrlVal(panelHandle, PANEL_TARGET, target);
		if (UpdateTable(INIT))
			SetCtrlAttribute(panelHandle, PANEL_SUBMIT_CHANGES, ATTR_DIMMED, 0);
	}
	return 0;
}

int CVICALLBACK SubmitCallback(int panel, int control, int event, void *callbackData,
							int eventData1, int eventData2)
{
	/*	Submit alias changes and refresh table	*/
	
	if(event == EVENT_COMMIT)
	{
		NISysCfgResourceHandle resource = NULL;
		int col = 1;
		int row = 1;
		char existingAlias[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		char newAlias[NISYSCFG_SIMPLE_STRING_LENGTH] = "";

		SetWaitCursor(1);
		
		status = NISysCfgResetEnumeratorGetCount(resourcesHandle, NULL);
		
		while (NISysCfg_Succeeded(status) && (status = NISysCfgNextResource(session, resourcesHandle, &resource)) == NISysCfg_OK)
		{
			col = 2;
			GetTableCellAttribute(panelHandle, PANEL_CONFIGURATION_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, existingAlias);
			col = 3;
			GetTableCellAttribute(panelHandle, PANEL_CONFIGURATION_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, newAlias);
					
			//	Only update alias if table cell has text and if the new alias is different than the old alias
			if (strlen(newAlias) && strcmp(existingAlias, newAlias) != 0)
			{
				if (NISysCfg_Failed(status = NISysCfgRenameResource(resource, newAlias, NISysCfgBoolFalse, NISysCfgBoolTrue, NULL, NULL)))
					break;
			}
			status = NISysCfgCloseHandle(resource);
			row++;
		}
		
		SetWaitCursor(0);
		if (NISysCfg_Succeeded(status))
			UpdateTable(NO_INIT);
		else
		{
			NISysCfgGetStatusDescription(session, status, &detailedDescription);
			MessagePopup("System Configuration Error", detailedDescription);
			NISysCfgFreeDetailedString(detailedDescription);
		}
	}	 
	return 0;
}

int CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData,
							int eventData1, int eventData2)
{
	/* 	Close System Configuration session and quit */
	
	if(event == EVENT_COMMIT)
	{
		status = NISysCfgCloseHandle(resourcesHandle);
		status = NISysCfgCloseHandle(session);
		QuitUserInterface(0);
	}
	return 0;
}


