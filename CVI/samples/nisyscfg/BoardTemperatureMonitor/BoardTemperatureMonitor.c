//============================================================================================
//
// Title:		BoardTemperatureMonitor.c
// Purpose:		Monitors temperature of National Instruments boards in system,
//				and reports a warning when temperature on a board exceeds Temperature Limit.
//
//				NOTE: Only works on boards with built-in temperature sensors.
//
//============================================================================================

#include <windows.h>
#include <utility.h>
#include "nisyscfg.h"
#include "BoardTemperatureMonitor.h"

NISysCfgSessionHandle session = NULL;
NISysCfgEnumResourceHandle resourcesHandle = NULL;

char* detailedDescription = NULL;
int panelHandle = 0;
double maxTemperature = 0;
int threadPool = 0;
int threadExitFlag = 0;
int numResources = 0; 

int main(void)
{
	panelHandle = LoadPanel(0, "BoardTemperatureMonitor.uir", PANEL);
	DisplayPanel(panelHandle);
	SetCtrlAttribute(panelHandle, PANEL_STOP, ATTR_DIMMED, 1);
	RunUserInterface();
	DiscardPanel(panelHandle);
	return 0;
}

int CVICALLBACK GatherTemperatures(void *functionData)
{
	/*	Obtain temperatures from each board and plot on chart	*/
	
	NISysCfgStatus status = NISysCfg_OK;
	NISysCfgResourceHandle resource = NULL;          
	double *temperatures = NULL;
	int rsrcIndex = 0;
	char hotBoardName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char errorMessage[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	
	temperatures = malloc(sizeof(double) * numResources);
	
	while (!threadExitFlag)
	{
		status = NISysCfgResetEnumeratorGetCount(resourcesHandle, NULL);
		while (NISysCfg_Succeeded(status) && (status = NISysCfgNextResource(session, resourcesHandle, &resource)) == NISysCfg_OK)
		{
			if (NISysCfg_Succeeded(status = NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyCurrentTemp, &temperatures[rsrcIndex])))
			{
				if (temperatures[rsrcIndex] > maxTemperature)
				{
					status = NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyProductName, hotBoardName);
					strcat(errorMessage, "WARNING! ");
					strcat(errorMessage, hotBoardName);
					strcat(errorMessage, " is above the temperature limit. Stopping scan...");
					MessagePopup("Board Overheating!", errorMessage);
					StopCallback(panelHandle, PANEL_STOP, EVENT_COMMIT, NULL, 0, 0);
				}
			}
			else
				break;
		
			rsrcIndex++;
			status = NISysCfgCloseHandle(resource);
			PlotStripChart(panelHandle, PANEL_TEMPERATURE_CHART, temperatures, numResources, 0, 0, VAL_DOUBLE);
			Sleep(50);
		}
		rsrcIndex = 0;
	}
	
	if (NISysCfg_Failed(status))
	{
		NISysCfgGetStatusDescription(session, status, &detailedDescription);
		MessagePopup("System Configuration Error", detailedDescription);
		NISysCfgFreeDetailedString(detailedDescription);
		StopCallback(panelHandle, PANEL_STOP, EVENT_COMMIT, NULL, 0, 0);
	}
	return 0;
}

int CVICALLBACK PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	/* 	Close System Configuration session and quit	*/
	
	if (event == EVENT_CLOSE)
	{
		NISysCfgStatus status = NISysCfg_OK;
		threadExitFlag = 1;   
		
		//	Wait for thread to exit
		Sleep(100);
		
		status = NISysCfgCloseHandle(resourcesHandle);
		status = NISysCfgCloseHandle(session);
		CmtDiscardThreadPool(threadPool);
		QuitUserInterface(0);
	}
	return 0;
}

int CVICALLBACK TemperatureCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	/*	Update maximum temperature limit	*/
	
	if (event == EVENT_VAL_CHANGED)
	{
		GetCtrlVal(panelHandle, PANEL_TEMPERATURE, &maxTemperature);
	}
	return 0;
}

int CVICALLBACK QuitCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	/* 	Close System Configuration session and quit	*/   
	
	if (event == EVENT_COMMIT)
	{
		NISysCfgStatus status = NISysCfg_OK; 
		threadExitFlag = 1;   
		
		//	Wait for thread to exit
		Sleep(100);

		status = NISysCfgCloseHandle(resourcesHandle);
		status = NISysCfgCloseHandle(session);
		CmtDiscardThreadPool(threadPool);
		QuitUserInterface(0);
	}
	return 0;
	
}

int CVICALLBACK GoCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	/* 	Initialize System Configuration session and resources,		*/   
	/*	set graph properties, and spawn GatherTemperatures thread 	*/
	
	if (event == EVENT_COMMIT)
	{
		NISysCfgStatus status = NISysCfg_OK;
		NISysCfgResourceHandle resource = NULL;
		NISysCfgFilterHandle filter = NULL;
		int traceColor = 0;
		int traceIndex = 1;
		char target[NISYSCFG_SIMPLE_STRING_LENGTH] = "";  
		char productName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
		
		SetWaitCursor(1);
		
		threadExitFlag = 0;
		ClearStripChart(panelHandle, PANEL_TEMPERATURE_CHART);
		GetCtrlVal(panelHandle, PANEL_TARGET, target);
		GetCtrlVal(panelHandle, PANEL_TEMPERATURE, &maxTemperature); 
		if (NISysCfg_Succeeded(status = NISysCfgInitializeSession(target, NULL, NULL, NISysCfgLocaleDefault, NISysCfgBoolTrue, 10000, NULL, &session)))
			if (NISysCfg_Succeeded(status = NISysCfgCreateFilter(session, &filter)))
			{
				NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyIsSimulated, NISysCfgBoolFalse);
				NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyIsDevice, NISysCfgBoolTrue);
				NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertySupportsCalibration, NISysCfgBoolTrue);
				status = NISysCfgFindHardware(session, NISysCfgFilterModeAll, filter, NULL, &resourcesHandle);
			}
		SetWaitCursor(0); 
		if (resourcesHandle)
		{
			status = NISysCfgResetEnumeratorGetCount(resourcesHandle, &numResources);
			SetCtrlAttribute(panelHandle, PANEL_TEMPERATURE_CHART, ATTR_NUM_TRACES, numResources);
	
			//	Adjust graph legend to show all traces appropriately
			while (NISysCfg_Succeeded(status) && (status = NISysCfgNextResource(session, resourcesHandle, &resource)) == NISysCfg_OK)
			{
				status = NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyProductName, &productName);
				GetTraceAttribute(panelHandle, PANEL_TEMPERATURE_CHART, traceIndex, ATTR_TRACE_COLOR, &traceColor);
				SetTraceAttribute(panelHandle, PANEL_TEMPERATURE_CHART, traceIndex, ATTR_TRACE_LG_VISIBLE, 1);
				SetTraceAttribute(panelHandle, PANEL_TEMPERATURE_CHART, traceIndex, ATTR_TRACE_LG_TEXT_COLOR, traceColor);
				SetTraceAttributeEx(panelHandle, PANEL_TEMPERATURE_CHART, traceIndex, ATTR_TRACE_LG_TEXT, productName);
				status = NISysCfgCloseHandle(resource);
				traceIndex++;
			}
			
			if (NISysCfg_Failed(status))
			{
				NISysCfgGetStatusDescription(session, status, &detailedDescription);
				MessagePopup("System Configuration Error", detailedDescription);
				NISysCfgFreeDetailedString(detailedDescription);
				StopCallback(panelHandle, PANEL_STOP, EVENT_COMMIT, NULL, 0, 0);
			}
			else
			{
				CmtNewThreadPool(1, &threadPool);
				CmtPreAllocThreadPoolThreads(threadPool, 1, NULL);
				CmtScheduleThreadPoolFunction(threadPool, GatherTemperatures, NULL, NULL);
			}
		
			status = NISysCfgResetEnumeratorGetCount(resourcesHandle, NULL);
			status = NISysCfgCloseHandle(filter);
		
			SetCtrlAttribute(panelHandle, PANEL_GO, ATTR_DIMMED, 1);
			SetCtrlAttribute(panelHandle, PANEL_STOP, ATTR_DIMMED, 0);
		}
		else
			MessagePopup("No Devices Found", "No devices with temperature sensors were found on the specified system");
	}
	return 0;
}

int CVICALLBACK StopCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	/*	Halt GatherTemperatures thread	*/
	
	if (event == EVENT_COMMIT)
	{
		threadExitFlag = 1;
		Sleep(100);
		SetCtrlAttribute(panelHandle, PANEL_STOP, ATTR_DIMMED, 1);
		SetCtrlAttribute(panelHandle, PANEL_GO, ATTR_DIMMED, 0);  
	}
	return 0;
}
