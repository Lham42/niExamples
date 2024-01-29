/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: PanelCallback */
#define  PANEL_AVAILABLESETS              2       /* control type: table, callback function: (none) */
#define  PANEL_TARGET                     3       /* control type: string, callback function: (none) */
#define  PANEL_USERNAME                   4       /* control type: string, callback function: (none) */
#define  PANEL_PASSWORD                   5       /* control type: string, callback function: (none) */
#define  PANEL_STATUS                     6       /* control type: string, callback function: (none) */
#define  PANEL_INSTALL                    7       /* control type: command, callback function: InstallCallback */
#define  PANEL_QUIT                       8       /* control type: command, callback function: QuitCallback */
#define  PANEL_GO                         9       /* control type: command, callback function: GoCallback */
#define  PANEL_UNINSTALLALL               10      /* control type: command, callback function: UninstallAllCallback */
#define  PANEL_INSTALLEDSET               11      /* control type: textBox, callback function: (none) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK GoCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK InstallCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK UninstallAllCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
