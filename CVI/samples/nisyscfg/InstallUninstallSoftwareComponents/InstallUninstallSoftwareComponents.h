/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: PanelCallback */
#define  PANEL_TARGET                     2       /* control type: string, callback function: (none) */
#define  PANEL_USERNAME                   3       /* control type: string, callback function: (none) */
#define  PANEL_PASSWORD                   4       /* control type: string, callback function: (none) */
#define  PANEL_STATUS                     5       /* control type: string, callback function: (none) */
#define  PANEL_AVAILABLECOMPONENTS        6       /* control type: table, callback function: (none) */
#define  PANEL_INSTALLEDCOMPONENTS        7       /* control type: table, callback function: (none) */
#define  PANEL_GO                         8       /* control type: command, callback function: GoCallback */
#define  PANEL_INSTALL                    9       /* control type: command, callback function: InstallCallback */
#define  PANEL_UNINSTALL                  10      /* control type: command, callback function: UninstallCallback */
#define  PANEL_QUIT                       11      /* control type: command, callback function: QuitCallback */
#define  PANEL_UNINSTALLALL               12      /* control type: command, callback function: UninstallAllCallback */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK GoCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK InstallCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK UninstallAllCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK UninstallCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
