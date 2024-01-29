//==============================================================================
//
// Title:       CalibrationAudit.c
// Purpose:     Obtains calibration information for hardware on selected target
//
//==============================================================================

#include <utility.h>
#include <ansi_c.h>
#include "nisyscfg.h"
#include "CalibrationAudit.h"

int panelHandle = 0;

int main(void)
{
	panelHandle = LoadPanel(0, "CalibrationAudit.uir", PANEL);
	DisplayPanel(panelHandle);
	RunUserInterface();
	DiscardPanel(panelHandle);
	return 0;
}

int AuditCalInfo(void)
{
	NISysCfgStatus status = NISysCfg_OK;
	NISysCfgSessionHandle session = NULL;
	NISysCfgEnumResourceHandle resourcesHandle = NULL;
	NISysCfgResourceHandle resource = NULL;
	NISysCfgFilterHandle filter = NULL;
	NISysCfgTimestampUTC intCalLastTimestamp = {0};
	NISysCfgTimestampUTC extCalLastTimestamp = {0};
	NISysCfgTimestampUTC extCalNextTimestamp = {0};

	char target[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char resourceName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char alias[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char productName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char serialNumber[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char calNotes[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char intCalLastDateStr[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char extCalLastDateStr[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char extCalNextDateStr[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char intCalLastTempStr[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char extCalLastTempStr[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char currentTempStr[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	CVIAbsoluteTime currentTimestamp = {0};
	double intCalLastTempNum = 0;
	double extCalLastTempNum = 0;
	double currentTempNum = 0;
	int year = 0;
	int month = 0;
	int day = 0;
	int col = 1;
	int row = 1;
	char* detailedDescription = NULL;

	SetWaitCursor(1);

	GetCtrlVal(panelHandle, PANEL_TARGET, target);

	if (NISysCfg_Succeeded(status = NISysCfgInitializeSession(target, NULL, NULL, NISysCfgLocaleDefault, NISysCfgBoolTrue, 10000, NULL, &session)))
	{
		if (NISysCfg_Succeeded(status = NISysCfgCreateFilter(session, &filter)))
		{
			NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyIsDevice, NISysCfgBoolTrue);
			NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertySupportsCalibration, NISysCfgBoolTrue);
			NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyIsPresent, NISysCfgIsPresentTypePresent);
			NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyIsSimulated, NISysCfgBoolFalse);
			status = NISysCfgFindHardware(session, NISysCfgFilterModeAll, filter, NULL, &resourcesHandle);
		}
	}

	if (status == NISysCfg_OK)
	{
		if (resourcesHandle)
		{
			//	Iterate through all found resources and obtain calibration information
			//	Disable run-time checking for properties that may not be valid
			SetBreakOnLibraryErrors(0);

			GetCurrentCVIAbsoluteTime(&currentTimestamp);

			while (NISysCfg_Succeeded(status) && (status = NISysCfgNextResource(session, resourcesHandle, &resource)) == NISysCfg_OK)
			{
				col = 1;
				InsertTableRows(panelHandle, PANEL_CALIBRATION_TABLE, row, 1, VAL_CELL_STRING);

				status = NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertResourceName, 0, resourceName);
				status = NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertUserAlias, 0, alias);
				if (strlen(alias))
					SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, alias);
				else
					SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col,row), ATTR_CTRL_VAL, resourceName);

				col = 2;
				status = NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyProductName, productName);
				SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col, row), ATTR_CTRL_VAL, productName);

				col = 3;
				status = NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertySerialNumber, serialNumber);
				SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col, row), ATTR_CTRL_VAL, serialNumber);

				col = 4;
				status = NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyInternalCalibrationLastTime, 0, &intCalLastTimestamp);
				CVIAbsoluteTimeToLocalCalendar(intCalLastTimestamp, &year, &month, &day, NULL, NULL, NULL, NULL, NULL);
				sprintf(intCalLastDateStr, "%d/%d/%d", month, day, year);
				SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col, row), ATTR_CTRL_VAL, intCalLastDateStr);

				col = 5;
				status = NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyInternalCalibrationLastTemp, 0, &intCalLastTempNum);
				sprintf(intCalLastTempStr, "%.2f", intCalLastTempNum);
				SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col, row), ATTR_CTRL_VAL, intCalLastTempStr);

				col = 6;
				status = NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyExternalCalibrationLastTime, &extCalLastTimestamp);
				CVIAbsoluteTimeToLocalCalendar(extCalLastTimestamp, &year, &month, &day, NULL, NULL, NULL, NULL, NULL);
				sprintf(extCalLastDateStr, "%d/%d/%d", month, day, year);
				SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col, row), ATTR_CTRL_VAL, extCalLastDateStr);

				col = 7;
				status = NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyExternalCalibrationLastTemp, &extCalLastTempNum);
				sprintf(extCalLastTempStr, "%.2f", extCalLastTempNum);
				SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col, row), ATTR_CTRL_VAL, extCalLastTempStr);

				col = 8;
				status = NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyRecommendedNextCalibrationTime, &extCalNextTimestamp);
				CVIAbsoluteTimeToLocalCalendar(extCalNextTimestamp, &year, &month, &day, NULL, NULL, NULL, NULL, NULL);
				if (currentTimestamp.cviTime.msb > extCalNextTimestamp.cviTime.msb)
					SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col, row), ATTR_TEXT_COLOR, VAL_RED);
				else
					SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col, row), ATTR_TEXT_COLOR, VAL_BLACK);
				sprintf(extCalNextDateStr, "%d/%d/%d", month, day, year);
				SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col, row), ATTR_CTRL_VAL, extCalNextDateStr);

				col = 9;
				status = NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyCurrentTemp, &currentTempNum);
				sprintf(currentTempStr, "%.2f", currentTempNum);
				SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col, row), ATTR_CTRL_VAL, currentTempStr);

				col = 10;
				status = NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyCalibrationComments, calNotes);
				SetTableCellAttribute(panelHandle, PANEL_CALIBRATION_TABLE, MakePoint(col, row), ATTR_CTRL_VAL, calNotes);

				row++;
				status = NISysCfgCloseHandle(resource);
			}

			SetBreakOnLibraryErrors(1);
		}
		else
			MessagePopup("No Devices Found", "No devices were found with calibration information");
	}

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
		GetNumTableRows(panelHandle, PANEL_CALIBRATION_TABLE, &numRows);
		if (numRows > 0)
			DeleteTableRows(panelHandle, PANEL_CALIBRATION_TABLE, 1, numRows);
		AuditCalInfo();
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
