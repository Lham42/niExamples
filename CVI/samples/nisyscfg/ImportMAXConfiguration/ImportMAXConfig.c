//==============================================================================
//
// Title:       ImportMAXConfig.c
// Purpose:     Imports a MAX Configuration File from the specified location
//
//==============================================================================

#include <stdio.h>
#include "nisyscfg.h"

int main(void)
{
	NISysCfgStatus status = NISysCfg_OK;
	NISysCfgSessionHandle session = NULL;
	
	char target[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char filePath[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char* detailedResults = NULL;
	char* detailedDescription = NULL;
	
	printf("Enter the Hostname, IP Address, or MAC Address of your target system\n"
		   ">> ");
	scanf("%s", target);
	
	if ((status = NISysCfgInitializeSession(target, NULL, NULL, NISysCfgLocaleDefault, NISysCfgBoolFalse, 10000, NULL, &session)) == NISysCfg_OK)
	{
		printf("Enter file path of existing configuration file\n"
			   "Note: The standard configuration file format uses a .nce extension\n"
			   ">> ");
		scanf("%s", filePath);
		printf("Importing data. This may take a few minutes\n");
		//	By default, NISysCfgImportConfiguration will merge new items with existing ones
		// 	To change this behavior, modify ImportMode
		status = NISysCfgImportConfiguration(session, filePath, NULL, NISysCfgImportMergeItems, &detailedResults);
		if (NISysCfg_Failed(status))
		{
			printf("NISysCfgImportConfiguration Failed!\n Detailed Results: \n %s", detailedResults);
			NISysCfgFreeDetailedString(detailedResults);
		}
	}
	
	if (NISysCfg_Failed(status))
	{
		NISysCfgGetStatusDescription(session, status, &detailedDescription);
		printf("Error: %s\n", detailedDescription);
		NISysCfgFreeDetailedString(detailedDescription);
	}
	else
		printf("Configuration file imported\n");
	printf("Press Enter to exit");
	fflush(stdin);
	getchar();
	status = NISysCfgCloseHandle(session);
	
	return 0;
}

