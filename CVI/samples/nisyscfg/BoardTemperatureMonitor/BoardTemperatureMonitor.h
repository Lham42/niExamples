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
#define  PANEL_TEMPERATURE                3       /* control type: scale, callback function: TemperatureCallback */
#define  PANEL_QUIT                       4       /* control type: command, callback function: QuitCallback */
#define  PANEL_STOP                       5       /* control type: command, callback function: StopCallback */
#define  PANEL_GO                         6       /* control type: command, callback function: GoCallback */
#define  PANEL_TEMPERATURE_CHART          7       /* control type: strip, callback function: (none) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK GoCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK StopCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TemperatureCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
