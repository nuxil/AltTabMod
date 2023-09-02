#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED


#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

/**
 * @def IDI_APPICON
 * Defines a id number for the ICON resource
*/
#define IDI_APPICON 1001

/**
 * @def WM_EVENT_TASKSWITCHER
 * Defines custom message.\n
 * This message will be sent by the WinEventProc_Win10/7 callback when the TaskSwitcher window closes.
*/
#define WM_EVENT_TASKSWITCHER (WM_USER + 1) 

/**
 * @def mmon
 * Defines a short name for: g_monitors[g_CurrentMonitor].rect
*/
#define mmon    (g_monitors[g_CurrentMonitor].rect)

/**
 * @def def RECT_INSIDE
 * Defines a check to see if rect B is within rect A
*/
#define RECT_INSIDE(A,B)    (((B.left>=A.left)&&(B.top>=A.top)&&(B.right<=A.right)&&(B.bottom<=A.bottom))?1:0)

/**
 * ==============================================================================================
 * @struct sMonitor
 * @brief A structure with general monitor info, general Info about the size, id, primary etc.\n
 * @var sMonitor::rect
 * The screen size/position of the monitor.
 * @var sMonitor::isPrimary
 * Indicates if the monitor is the primary monitor.
 */
typedef struct sMonitor {
    RECT rect;
    BOOL isPrimary;
} sMonitor;


/**
 * ==============================================================================================
 * @brief Enumeration representing different program error codes.
 * This enumeration defines various error codes that the program can return.
 * Each error code is associated with a specific error condition.
 */
typedef enum prgErr {
    ERR0 = 0, /**< No error, successful execution. */
    ERR1,     /**< Error code 1: The program is already running. */
    ERR2,     /**< Error code 2: Failed loading twinui.pcshell.dll. */
    ERR3,     /**< Error code 3: Failed loading alttab.dll. */
    ERR4,     /**< Error code 4: Too old Windows version. */
    ERR5,     /**< Error code 5: Failed collecting monitor info. */
    ERR6,     /**< Error code 6: Failed setting keyboard hook. */
    ERR7,     /**< Error code 7: Failed to get snapshot of processes. */
    ERR8      /**< Error code 8: Failed setting hook on explorer.exe. */
} prgErr;


/**
 * ==============================================================================================
 * @brief Enumeration representing different Windows operating system versions.
 */
typedef enum OsCheck {
    UNKNOWN = 0, /**< Unknown operating system version. */
    WIN10,       /**< Windows 10. */
    WIN7,        /**< Windows 7. */
} OsCheck;

/** 
 * =================================================================
 * @name Windows event CallBack functions
 * Callback functions that it attached to **explorer.exe**.\n
 * Windows 10 and Windows 7 has its own callback functions to keep them short and snappy.\n
 * This is because they do things a bit differently.
 */
///@{
 /**
 * @brief WinEventProc_Win10 callback function.\n
 * This function is a callback used with the WinAPI's SetWinEventHook function\n
 * to monitor and respond to various window-related events on Windows 10.\n
 * @param hWinEventHook The handle to the WinEvent hook.\n
 * @param event The event that occurred.
 * @param hwnd The handle to the window associated with the event.
 * @param idObject The identifier of the object associated with the event.
 * @param idChild The identifier of the child object associated with the event.
 * @param idEventThread The identifier of the thread that generated the event.
 * @param dwmsEventTime The timestamp of the event in milliseconds.
 * @return No return value.
 */
VOID CALLBACK WinEventProc_Win10(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime);

 /**
 * @brief WinEventProc_Win7 callback function.\n
 * This function is a callback used with the WinAPI's SetWinEventHook function\n
 * to monitor and respond to various window-related events on Windows 7.\n
 * @param hWinEventHook The handle to the WinEvent hook.
 * @param event The event that occurred.
 * @param hwnd The handle to the window associated with the event.
 * @param idObject The identifier of the object associated with the event.
 * @param idChild The identifier of the child object associated with the event.
 * @param idEventThread The identifier of the thread that generated the event.
 * @param dwmsEventTime The timestamp of the event in milliseconds.
 * @return No return value.
 */
VOID CALLBACK WinEventProc_Win7(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime);
///@}


/** 
 * =================================================================
 * @name Other CallBack functions
 * Other Callback functions the program uses.\n
 */
///@{
/**
 * @brief Window procedure (WinProc) callback function.\n
 * This function serves as the window procedure for a Windows application.\n
 * It handles and processes various window messages to provide the desired behavior,\n
 * and functionality for the application's main window.\n
 * @param hwnd The handle to the window that received the message.
 * @param uMsg The message identifier.
 * @param wParam Additional message-specific information.
 * @param lParam Additional message-specific information.
 * @return The return value is message-specific and typically depends on the\n
 * message being processed. Refer to the specific message documentation for details.
 */
LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); 

/**
 * @brief Low-level keyboard input procedure (WinKeyboardProc) callback function.\n
 * This function serves as a low-level keyboard input procedure and is used to\n
 * monitor and respond to keyboard input events at a low level. It is typically\n
 * employed for tasks such as global keyboard shortcuts or keylogging detection.\n
 * @param nCode An integer value that indicates the action that triggered the callback.\n
 * If less than zero, the callback should pass control to the next hook procedure in the hook chain.
 * @param wParam Specifies the type of keyboard message (WM_KEYDOWN, WM_KEYUP, etc.).
 * @param lParam A pointer to a KBDLLHOOKSTRUCT structure that contains information about the keyboard event.
 * @return Depending on the specific message and the desired behavior, this function\n
 * may return a value to allow or block further processing of the keyboard event.\n
 * If nCode is less than zero or the function wants to allow the event to\n
 * proceed, it should return the result of calling the CallNextHookEx function.\n
 * Otherwise, it may return a non-zero value to block the event.
 */
LRESULT CALLBACK WinKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
///@}

/**
 * @brief The main entry point for a Windows application (WinMain).\n
 * This function is the entry point for a Windows application and is typically\n
 * used to initialize the application, create the main window, and enter the\n
 * application's message loop for handling user input and events.\n
 * @param hInstance The handle to the current instance of the application.
 * @param prev (Unused) The handle to the previous instance of the application.
 * @param pCmdLine A pointer to the command-line arguments passed to the application.
 * @param nCmdShow Controls how the window is to be shown (e.g., minimized, maximized).
 * @return The exit code of the application upon termination.
 */
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prev, LPSTR pCmdLine, int nCmdShow );

/**
 * @brief Collects information about a monitor (CollectMonitorInfo).\n
 * This function collects information about a monitor and stores it in a provided
 * sMonitor structure.\n
 * @param monitor A pointer to an sMonitor structure where the collected information\n
 * will be stored. The structure should be initialized by the caller.\n
 * @return TRUE if the monitor information was successfully collected and stored in\n
 * the provided structure; otherwise, FALSE.
 */
BOOL CollectMonitorInfo(sMonitor *monitor);


/**
 * @brief Retrieves the monitor associated with the mouse cursor (GetMouseMonitor).\n
 * This function determines the monitor on which the mouse cursor is currently\n
 * located and provides information about that monitor in the provided, sMonitor structure.\n
 * @param monitor A pointer to an sMonitor structure where the monitor information will be stored.
 * @param sz The sjumber of monitors
 * @return The result is a BYTE value indicating the success or failure of the operation.\n
 * - 0: The operation was successful, and monitor information is stored in the provided structure.\n
 * - 1: An error occurred during the operation.
 */
BYTE GetMouseMonitor(sMonitor *monitor, INT sz);







#endif