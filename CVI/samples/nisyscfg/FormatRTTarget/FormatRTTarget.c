//==============================================================================
//
// Title:       FormatRTTarget.c
// Purpose:     Formats the specified RT Target, and allows the user to change
//				the Hostname and IP Address
//
//==============================================================================

#include <stdio.h>
#include <windows.h>
#include "nisyscfg.h"

NISysCfgStatus formatTarget(NISysCfgSessionHandle session, int IPRequestMode, const char* newHostname, char* newIP, NISysCfgBool forceSafeMode);

int main(void)
{
	NISysCfgStatus status = NISysCfg_OK;
	NISysCfgSessionHandle session = NULL;
	char target[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char newHostname[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char newIP[25] = "";
	char* detailedDescription = NULL;
	char forceSafeMode = 'n';
	int IPRequestMode = 0;

	//	Query user for device and format information

	printf("Enter the Hostname, IP Address, or MAC Address of the target system you wish to format\n"
		   ">> ");
	scanf("%s", target);

	printf("\n** Your target must be booted into User-Directed Safe Mode before proceeding **\n"
		   "Would you like your target to automatically reboot into Safe Mode before\n"
		   "formatting? (y/n)\n"
		   ">> ");
	scanf(" %c", &forceSafeMode);

	fflush(stdin);
	printf("\nEnter the new Hostname for the target\n"
		   "Hostname must contain only alphanumeric characters and hyphens\n"
		   ">> ");
	scanf("%s", newHostname);

	printf("\nSelect the IP Address request mode\n"
		   "This specifies whether the remote target receives its\n"
		   "IP address statically or dynamically\n\n"
		   "Static IP: 1 \nDHCP/Link-Local: 2 \n\n");
	printf("Enter the IP Address Request Mode\n"
		   ">> ");

	//	Check for valid IPRequestMode input
	while(!((IPRequestMode >= 1) && (IPRequestMode <= 2)))
	{
		scanf("%d", &IPRequestMode);
		if (!((IPRequestMode >= 1) && (IPRequestMode <= 2)))
			printf("\nInvalid choice. Please Enter the IP Address Request Mode\n"
				   ">> ");
	}

	//	If the new IP Address is on a different subnet, the Gateway will need to change
	//	This example does not change the NISysCfgSystemPropertyGateway property
	if (IPRequestMode == NISysCfgIpAddressModeStatic)
	{
		printf("\nEnter the new static IP Address of the target\n"
			   ">> ");
		scanf("%s", newIP);
	}

	if (NISysCfg_Succeeded(status = NISysCfgInitializeSession (target, NULL, NULL, NISysCfgLocaleDefault, NISysCfgBoolTrue, 10000, NULL, &session)))
	{
		if (forceSafeMode == 'y' || forceSafeMode == 'Y')
			status = formatTarget(session, IPRequestMode, newHostname, newIP, NISysCfgBoolTrue);
		else
			status = formatTarget(session, IPRequestMode, newHostname, newIP, NISysCfgBoolFalse);
	}

	if (NISysCfg_Failed(status))
	{
		NISysCfgGetStatusDescription(session, status, &detailedDescription);
		printf("Error formatting target: %s\n", detailedDescription);
		printf("Press Enter to close session and exit");
		NISysCfgFreeDetailedString(detailedDescription);
	}
	else
	{
		printf("Done Formatting, press Enter to close session and exit\n");
	}

	fflush(stdin);
	getchar();
	NISysCfgCloseHandle(session);

	return 0;
}

NISysCfgStatus formatTarget(NISysCfgSessionHandle session, int IPRequestMode, const char* newHostname, char* newIP, NISysCfgBool forceSafeMode)
{
	NISysCfgStatus status = NISysCfg_OK;
	NISysCfgBool requireRestart = NISysCfgBoolFalse;
	char* detailedResult = NULL;

	printf("\nFormatting...\n");

	if (NISysCfg_Failed(status = NISysCfgFormat (session, forceSafeMode, NISysCfgBoolTrue, NISysCfgFileSystemDefault, NISysCfgPreservePrimaryResetOthers, 300000)))
		return status;

	if (NISysCfg_Failed(status = NISysCfgSetSystemProperty(session, NISysCfgSystemPropertyHostname, newHostname)))
		return status;

	if (NISysCfg_Failed(status = NISysCfgSetSystemProperty(session, NISysCfgSystemPropertyIpAddressMode, IPRequestMode)))
		return status;

	if (IPRequestMode == NISysCfgIpAddressModeStatic)
	{
		if (NISysCfg_Failed(status = NISysCfgSetSystemProperty(session, NISysCfgSystemPropertyIpAddress, newIP)))
			return status;
	}

	if (NISysCfg_Failed(status = NISysCfgSaveSystemChanges(session, &requireRestart, &detailedResult)))
	{
		printf("Save Error: %s\n", detailedResult);
		NISysCfgFreeDetailedString(detailedResult);
		return status;
	}

	if (requireRestart)
	{
		status = NISysCfgRestart(session, NISysCfgBoolTrue, NISysCfgBoolFalse, NISysCfgBoolFalse, 90000, newIP);
	}

	return status;
}
