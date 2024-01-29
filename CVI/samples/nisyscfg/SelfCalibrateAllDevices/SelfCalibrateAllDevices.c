//==========================================================================
//
// Title:       SelfCalibrateAllDevices.c
// Purpose:     Self-Calibrate all hardware devices on the specified system
//																
//==========================================================================

#include <utility.h>
#include <ansi_c.h>
#include "nisyscfg.h"
#include "SelfCalibrateAllDevices.h"

int panelHandle = 0;

int main(void)
{
	panelHandle = LoadPanel(0, "SelfCalibrateAllDevices.uir", PANEL);
	DisplayPanel(panelHandle);
	RunUserInterface();
	DiscardPanel(panelHandle);
	return 0;
}

int SelfCalibrateDevices(void)
{
	NISysCfgStatus status = NISysCfg_OK;
	NISysCfgSessionHandle session = NULL;
	NISysCfgEnumResourceHandle resourcesHandle = NULL;
	NISysCfgResourceHandle resource = NULL;
	NISysCfgFilterHandle filter = NULL;
	
	char target[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char productName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char resourceName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char alias[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char passFail[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char* detailedDescription = NULL;
	char* detailedResult = NULL;
	int col = 1;
	int row = 1;
	
	SetWaitCursor(1);
	
	GetCtrlVal(panelHandle, PANEL_TARGET, target);
	
	if (NISysCfg_Succeeded(status = NISysCfgInitializeSession(target, NULL, NULL, NISysCfgLocaleDefault, NISysCfgBoolTrue, 10000, NULL, &session)))
		if (NISysCfg_Succeeded(status = NISysCfgCreateFilter(session, &filter)))
		{
			NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyIsDevice, NISysCfgBoolTrue);
			NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyIsSimulated, NISysCfgBoolFalse);
			NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertySupportsCalibration, NISysCfgBoolTrue);
			status = NISysCfgFindHardware(session, NISysCfgFilterModeAll, filter, NULL, &resourcesHandle);
		}
	
	if (resourcesHandle)
	{
		//	Iterate through all found resources and obtain properties/self-calibrate
		//	Disable run-time checking for properties that may not be valid
		SetBreakOnLibraryErrors(0);

		while (NISysCfg_Succeeded(status) && (status = NISysCfgNextResource(session, resourcesHandle, &resource)) == NISysCfg_OK)
		{				   
			col = 1;
			InsertTableRows(panelHandle, PANEL_RESULTS_TABLE, row, 1, VAL_CELL_STRING);
		
			status = NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyProductName, productName);
			status = NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertResourceName, 0, resourceName);
			status = NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertUserAlias, 0, alias);
		
			status = NISysCfgSelfCalibrateHardware(resource, &detailedResult);
		
			//	Populate table with results of self-calibration
			switch (status)
			{
				case NISysCfg_OK:
					strcpy(passFail, "Pass");
					break;
				case NISysCfg_NotImplemented:
					strcpy(passFail, "Not Supported");
					status = NISysCfg_OK;
					break;
				default:
					sprintf(passFail, "Error: %d", status);
					status = NISysCfg_OK;
					break;
			}
		
			if (strlen(alias))
				SetTableCellAttribute(panelHandle, PANEL_RESULTS_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, alias);
			else
				SetTableCellAttribute(panelHandle, PANEL_RESULTS_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, resourceName);
		
			col = 2;
			SetTableCellAttribute(panelHandle, PANEL_RESULTS_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, productName);
		
			col = 3;
			SetTableCellAttribute(panelHandle, PANEL_RESULTS_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, passFail);
			
			col = 4;
			SetTableCellAttribute(panelHandle, PANEL_RESULTS_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, detailedResult);
		
			row++;
			status = NISysCfgCloseHandle(resource);
			NISysCfgFreeDetailedString(detailedResult);
		}
	
		SetBreakOnLibraryErrors(1);
	}
	else
		MessagePopup("No Devices Found", "No devices were found that can be self-calibrated on the specified system");
	
	if (NISysCfg_Failed(status))
	{
		NISysCfgGetStatusDescription(session, status, &detailedDescription);
		MessagePopup("System Configuration Error", detailedDescription);
		NISysCfgFreeDetailedString(detailedDescription);
	}
	
	status = NISysCfgCloseHandle(filter);
	status = NISysCfgCloseHandle(resourcesHandle);
	status = NISysCfgCloseHandle(session);
	SetWaitCursor(0);
	
	return 0;
}

int CVICALLBACK PanelCallback (int panel, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event == EVENT_CLOSE)
		QuitUserInterface(0);
	return 0;
}

int CVICALLBACK GoCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event == EVENT_COMMIT)
	{
		//	Clear table to remove existing data
		int numRows = 0;
		GetNumTableRows(panelHandle, PANEL_RESULTS_TABLE, &numRows);
		if (numRows > 0)
			DeleteTableRows(panelHandle, PANEL_RESULTS_TABLE, 1, numRows);
		SelfCalibrateDevices();
	}
	return 0;
}

int CVICALLBACK QuitCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event == EVENT_COMMIT)
		QuitUserInterface(0);
	return 0;
}

