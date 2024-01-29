//==============================================================================
//
// Title:       ShowAllHardware.c
// Purpose:     Displays all hardware on a given system as well as specified
//				properties of that hardware
//
//==============================================================================

#include <string.h>
#include <stdio.h>
#include <utility.h>
#include "nisyscfg.h"

int main(void)
{
	NISysCfgStatus status = NISysCfg_OK;
	NISysCfgSessionHandle session = NULL;
	NISysCfgEnumResourceHandle resourcesHandle = NULL;
	NISysCfgResourceHandle resource = NULL;
	NISysCfgFilterHandle filter = NULL;

	char expertName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char resourceName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char alias[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char productName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char serialNumber[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char target[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
	char *detailedDescription = NULL;
	char networkShow = 'y';

	printf("Enter the Hostname, IP Address, or MAC Address of your target system\n"
		   ">> ");
	scanf("%s", target);

	printf("Do you want to show all network devices? (y/n)\n"
		   ">> ");
	scanf(" %c", &networkShow);
	fflush(stdin);

	//	Initialize System Configuration session and find all hardware on the target

	printf("\nInitializing session and finding hardware... \n");
	if (NISysCfg_Succeeded(status = NISysCfgInitializeSession(target, NULL, NULL, NISysCfgLocaleDefault, NISysCfgBoolTrue, 10000, NULL, &session)))
		if (NISysCfg_Succeeded(status = NISysCfgCreateFilter(session, &filter)))
		{
			NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyIsDevice, NISysCfgBoolTrue);
			if (NISysCfg_Succeeded(status = NISysCfgFindHardware(session, NISysCfgFilterModeAll, filter, NULL, &resourcesHandle)))
			{
				//	Iterate through all found resources and obtain properties
				//	Disable run-time checking for properties that may not be valid
				SetBreakOnLibraryErrors(0);
				while (NISysCfg_Succeeded(status) && (status = NISysCfgNextResource(session, resourcesHandle, &resource)) == NISysCfg_OK)
				{
					NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertName, 0, expertName);
					if ((strcmp(expertName, "network") != 0) || (networkShow == 'y' || networkShow == 'Y'))
					{
						NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertResourceName, 0, resourceName);
						NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertUserAlias, 0, alias);

						//	Print the Device Alias if it exists (e.g. Dev1, PXI1Slot2, etc), otherwise print the Resource Name
						if (strlen(alias))
							printf("Device Alias  : %s\n", alias);
						else
							printf("Resource Name  : %s\n", resourceName);

						NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyProductName, productName);
						printf("Product Name  : %s\n", productName);

						NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertySerialNumber, serialNumber);
						printf("Serial Number : %s\n\n", serialNumber);

						status = NISysCfgCloseHandle(resource);
					}
				}
				SetBreakOnLibraryErrors(1);
			}
		}

	if (NISysCfg_Failed(status))
	{
		NISysCfgGetStatusDescription(session, status, &detailedDescription);
		printf("Error: %s\n", detailedDescription);
		NISysCfgFreeDetailedString(detailedDescription);
	}

	printf("Press Enter to close session and exit\n");
	fflush(stdin);
	getchar();

	status = NISysCfgCloseHandle(filter);
	status = NISysCfgCloseHandle(resourcesHandle);
	status = NISysCfgCloseHandle(session);

	return 0;
}
