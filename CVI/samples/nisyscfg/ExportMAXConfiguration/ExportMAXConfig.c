//==============================================================================
//
// Title:       ExportMAXConfig.c
// Purpose:     Exports a MAX Configuration File to the specified location
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
	char* detailedDescription = NULL;
	
	printf("Enter the Hostname, IP Address, or MAC Address of your target system\n"
		   ">> ");
	scanf("%s", target);
	
	if ((status = NISysCfgInitializeSession(target, NULL, NULL, NISysCfgLocaleDefault, NISysCfgBoolFalse, 10000, NULL, &session)) == NISysCfg_OK)
	{
		printf("Enter file path for configuration file to be stored\n"
			   "Note: The standard configuration file format uses a .nce extension\n"
			   ">> ");
		scanf("%s", filePath);
		printf("Exporting data. This may take a few minutes\n");
		//	By default, NISysCfgExportConfiguration will not overwrite an existing file
		//	To overwrite an existing file, set OverwriteIfExists to NISysCfgBoolTrue
		status = NISysCfgExportConfiguration(session, filePath, NULL, NISysCfgBoolFalse);
	}
	
	if (NISysCfg_Failed(status))
	{
		NISysCfgGetStatusDescription(session, status, &detailedDescription);
		printf("Error: %s\n", detailedDescription);
		NISysCfgFreeDetailedString(detailedDescription);
	}
	else
		printf("Configuration file exported\n");
	printf("Press Enter to exit");
	fflush(stdin);
	getchar();
	status = NISysCfgCloseHandle(session);
	
	return 0;
}

