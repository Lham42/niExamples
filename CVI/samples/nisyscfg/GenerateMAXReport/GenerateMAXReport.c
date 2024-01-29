//=========================================================================================
//
// Title:       GenerateMAXReport.c
// Purpose:     Generates a MAX report
//				This report can be in either xml, html, or .zip (technical report) format
//
//=========================================================================================

#include <ansi_c.h>
#include "nisyscfg.h"

int main (void)
{
	NISysCfgStatus status = NISysCfg_OK;
	NISysCfgSessionHandle session = NULL;
	
	char target[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char filePath[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char* fileExtension = NULL;
	int reportType = -1;
	int validFilePath = 0;
	char* detailedDescription = NULL;
	
	printf("Enter the Hostname, IP Address, or MAC Address of your target system\n"
		   ">> ");
	scanf("%s", target);
	
	printf("\nEnter report type\n"
		   "(0) XML Report | (1) HTML Report | (2) Technical Support Report\n"
		   ">> ");
	while(!((reportType >= 0) && (reportType <= 2)))
	{
		scanf("%d", &reportType);
		if (!((reportType >= 0) && (reportType <= 2)))
			printf("\nInvalid choice. Please enter the report type\n"
				   ">> ");
	}
	
	while (!validFilePath)
	{
		printf("\nEnter file path for MAX report\n"
			   "Note: File path MUST end with one of the following:\n"
			   ".xml for XML Report\n"
			   ".html for HTML Report\n"
			   ".zip for Technical Support Report\n"
			   ">> ");
		scanf("%s", filePath);
	
		// Verify file extension is valid
		fileExtension = strstr(filePath, ".");
		
		switch (reportType)
			{
				case NISysCfgReportXml:
					if (strcmp(fileExtension, ".xml") == 0 ) validFilePath = 1;
					break;
				case NISysCfgReportHtml:
					if (strcmp(fileExtension, ".html") == 0) validFilePath = 1;
					break;
				case NISysCfgReportTechnicalSupportZip:
					if (strcmp(fileExtension, ".zip") == 0) validFilePath = 1;
					break;
				default:
					break;
			}
		
		
		if (!validFilePath)
			printf("Invalid file path extension\n");
	}
	
	printf("Generating report. This may take a few minutes\n");
	
	if ((status = NISysCfgInitializeSession(target, NULL, NULL, NISysCfgLocaleDefault, NISysCfgBoolFalse, 10000, NULL, &session)) == NISysCfg_OK)
	{
		//	By default, NISysCfgGenerateMAXReport will not overwrite an existing file
		//	To overwrite an existing file, set OverwriteIfExists to NISysCfgBoolTrue
		status = NISysCfgGenerateMAXReport(session, filePath, reportType, NISysCfgBoolFalse);
	}
	
	if (NISysCfg_Failed(status))
	{
		NISysCfgGetStatusDescription(session, status, &detailedDescription);
		printf("Error: %s\n", detailedDescription);
		NISysCfgFreeDetailedString(detailedDescription);
	}
	else
		printf("Report generated\n");
	printf("Press Enter to exit");
	fflush(stdin);
	getchar();
	status = NISysCfgCloseHandle(session);
	
	return 0;
}
