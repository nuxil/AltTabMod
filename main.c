
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <ole2.h>
#include <psapi.h>
#include <TlHelp32.h>
#include <initguid.h>
#include <shlobj.h>
#include <dwmapi.h>
#include <VersionHelpers.h>

#include "main.h"


#define Debug(n) (printf("NR:%i\n",n))


/**
 * =================================================================
 * @name Global Variables
 * Global variables used throughout the code
 */
///@{
WCHAR g_taskswitcherName[256]   = {0};   /**< global variable, Buffer to hold the loaded string of Task Switcher in the locale language. */
BYTE g_CurrentMonitor           = 0;     /**< Global variable with index of current monitors. */
INT g_num_monitors              = 0;     /**< Global variable with the number of monitors. */
sMonitor g_monitors[127]        = {0};   /**< Global struct with info about monitor(s). */
HWND g_hwnd                     = NULL;  /**< Global handler for the main window. */
HWND g_hwnd_taskswitcher        = NULL;  /**< Global handler for the taskswitcher window. */
HWINEVENTHOOK g_eventHook       = NULL;  /**< Global handle for the eventhook used on explorer. */
HHOOK g_keyboardHook            = NULL;  /**< Global handle for the keyboard hook. */
BOOL EcsKey                     = FALSE; /**< Global state of the Esc key, used to see if if was pressed while TaskSwitcher was open. */
OsCheck WinVersion              = UNKNOWN;
prgErr exitCode                 = ERR0;
///@}


 
//=================================================================
// Mostly Useless Stuff not in use for now or maybe ever.
static inline BOOL IsDirectXModule(const wchar_t *moduleName) {
    return (wcsstr(moduleName, L"d3d9.dll") != NULL ||
            wcsstr(moduleName, L"d3d10.dll") != NULL ||
            wcsstr(moduleName, L"d3d11.dll") != NULL ||
            wcsstr(moduleName, L"d3d12.dll") != NULL);
}
static inline BOOL IsOpenGLModule(const wchar_t *moduleName) {
    return (wcsstr(moduleName, L"opengl32.dll") != NULL);
}
static inline BOOL IsVulkanModule(const wchar_t *moduleName) {
    return (wcsstr(moduleName, L"vulkan-1.dll") != NULL);

}
BOOL IsApp3D(HWND hwnd) {
    
    // Retrieves the process ID associated with the given window handle (hwnd). 
    // We need the process ID to later identify the process and check if it's using 3D graphics libraries.
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    
    // Open a handle to the process with the necessary access rights.
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess) {
        HMODULE modules[1024];
        DWORD needed;

        // Retrieve a list of loaded modules in the target process.
        if (EnumProcessModules(hProcess, modules, sizeof(modules), &needed)) {
            for (DWORD i = 0; i < (needed / sizeof(HMODULE)); i++) {
                wchar_t moduleName[MAX_PATH];
                
                // Get the base name of the module (DLL) from its handle.
                if (GetModuleBaseNameW(hProcess, modules[i], moduleName, sizeof(moduleName) / sizeof(wchar_t))) {
                    
                     // Check if the module is a DirectX, OpenGL, or Vulkan module.
                    if (IsDirectXModule(moduleName) || IsOpenGLModule(moduleName) || IsVulkanModule(moduleName)) {
                        // Close the handle to the process and return true if a 3D module is found.
                        CloseHandle(hProcess);
                        return 1;
                    }
                }
            }
        }
        // Close the handle to the process if no 3D module is found.
        CloseHandle(hProcess);
    }
    // Return false if the process handle cannot be opened or no 3D module is found.
    return 0;
}
BOOL GetDwmAttribute(HWND hwnd) {

    HRESULT hr;
    BOOL bres;
    DWORD dres;
    // =======================================================
    // 1 : DWMWA_NCRENDERING_ENABLED attributes.
    // =======================================================
    hr = DwmGetWindowAttribute(hwnd, 1, &bres, sizeof(BOOL));
    if (SUCCEEDED(hr)) {
        printf("DWMWA_NCRENDERING_ENABLED value: %d\n", bres);
    } else {
        fprintf(stderr, "DWMWA_NCRENDERING_ENABLED failed with error 0x%lx\n", hr);
        printf("GetLastError: %08lX\n", GetLastError());
    }
    printf("\n");
    

    // =======================================================
    // 14 : DWMWA_CLOAKED attributes.
    // =======================================================
    hr = DwmGetWindowAttribute(hwnd, 14, &dres, sizeof(DWORD));
    if (SUCCEEDED(hr)) {
        printf("DWMWA_CLOAK value: %ld\n", dres);
        switch(dres) {
            
            // 0 is not listed in the docs. but i guess its visible.
            case 0: {
                printf("The window is visible\n");
            } break;
            
            //DWM_CLOAKED_APP 
            case 1: {printf("The window was cloaked by its owner application.\n");} break;
            // DWM_CLOAKED_SHELL 
            case 2: {printf("The window was cloaked by the Shell.\n");} break;
            // DWM_CLOAKED_INHERITED 
            case 4: {printf("The cloak value was inherited from its owner window.\n");} break;
        }
    } else {
        fprintf(stderr, "DWMWA_CLOAKED failed with error 0x%lx\n", hr);
        printf("GetLastError: %08lX\n", GetLastError());
    }    
    printf("\n");  

    return 1;
}



//=================================================================
BYTE GetMouseMonitor(sMonitor *monitor, INT sz) {

    POINT p;
    GetCursorPos(&p);
	INT x = p.x;
    INT y = p.y;
   
    for (INT i = 0; i < sz; i++) {
		if ( ((x >= monitor[i].rect.left) && (x <= monitor[i].rect.right)) && 
             ((y >= monitor[i].rect.top) && (y <= monitor[i].rect.bottom))
        ) {
            return i;
		} 
	}
	return 0;
}

BOOL CollectMonitorInfo(sMonitor *monitor)  {

    g_num_monitors = GetSystemMetrics(SM_CMONITORS);
    if (g_num_monitors == 0) { return 0;  }

    // loop until EnumDisplayDevices fails.
    int m=-1;
    int i=0;
    DISPLAY_DEVICE dd = {0};
    dd.cb = sizeof(DISPLAY_DEVICE);
    
    while(EnumDisplayDevices(NULL, ++m, &dd, 0) != 0) {
  
        // Get the display settings.
        BOOL res;
        DEVMODE dm = {0};
        dm.dmSize = sizeof(DEVMODE);
        res = EnumDisplaySettings(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm);
        if (res != 0) {
                
            // Get the monitor.
            POINT pt = { dm.dmPosition.x, dm.dmPosition.y };
            HMONITOR hmon = MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
            if (hmon) {

                // Collect monitor info.
                MONITORINFO mi = {0};
                mi.cbSize = sizeof(MONITORINFO);
                res = GetMonitorInfo( hmon, &mi);
                if (res != 0) {
                        
                    monitor[i].rect.left = mi.rcMonitor.left;
                    monitor[i].rect.top = mi.rcMonitor.top;
                    monitor[i].rect.right = mi.rcMonitor.right;
                    monitor[i].rect.bottom = mi.rcMonitor.bottom;
                    monitor[i].isPrimary = (mi.rcMonitor.left==0 && mi.rcMonitor.top == 0) ? TRUE : FALSE;
                            
                 /* printf("Monitor %d:\n", i + 1);
                    printf("    dm.dmPosition.x=%li, dm.dmPosition.y=%li\n", dm.dmPosition.x, dm.dmPosition.y);
                    printf("    Device Name: %s\n", dd.DeviceName);
                    printf("    Device String: %s\n", dd.DeviceString);
                    printf("    Resolution: %ldx%ld\n", dm.dmPelsWidth, dm.dmPelsHeight);
                    printf("    Refresh Rate: %ld Hz\n", dm.dmDisplayFrequency);
                    printf("    Color Depth: %ld bits per pixel\n", dm.dmBitsPerPel);
                    printf("    Desktop Location: L=%ld, T=%ld R=%ld B=%ld\n", mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom);
                    printf("    IsPrimary Desktop:%i\n", monitor[i].isPrimary );
                    printf("\n"); */
                    i++;
                }
            }
        }
    }
    return 1;
}


//=================================================================
VOID CALLBACK WinEventProc_Win10(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime) {

    // Check if the task-switcher window has been closed.
    if (event == EVENT_OBJECT_DESTROY && idObject == OBJID_WINDOW) {

        if (hwnd == g_hwnd_taskswitcher) {
            g_hwnd_taskswitcher = NULL; // nullify it
            SendMessage(g_hwnd, WM_EVENT_TASKSWITCHER, 0, 0);
        }
    }
   
    // Check if the task-switcher window has been created.
    if (event == EVENT_OBJECT_CREATE && hwnd != NULL && idObject == OBJID_WINDOW && idChild == INDEXID_CONTAINER) {
        
        wchar_t className[256] = {0};
        wchar_t titleName[256]  = {0};
        GetClassNameW(hwnd, className, sizeof(className));
        GetWindowTextW(hwnd, titleName, sizeof(titleName));
        
        // Ensure that it is the task-switcher window that is open.
        // Can the titleName ever be empty here.. ? i am asking.
        // TaskSwitcherWnd
        if ( (wcscmp(className, L"MultitaskingViewFrame") == 0 ) && (wcscmp(titleName, g_taskswitcherName) == 0) ) {
            g_hwnd_taskswitcher = hwnd;

            // Move the task-switcher to the monitor the mouse is on.
            g_CurrentMonitor = GetMouseMonitor(g_monitors, g_num_monitors);
            WINDOWPLACEMENT ts = {0};
            ts.length = sizeof(WINDOWPLACEMENT);
            ts.showCmd = 1;
            ts.rcNormalPosition.left   = mmon.left;
            ts.rcNormalPosition.right  = mmon.right;
            ts.rcNormalPosition.top    = mmon.top;
            ts.rcNormalPosition.bottom = mmon.bottom;
            
            // Now set it at the target monitor!.
            SetWindowPlacement(hwnd, &ts);
            UpdateWindow(hwnd);    
        }
    }
}

VOID CALLBACK WinEventProc_Win7(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime) {

    /*
    EVENT_SYSTEM_FOREGROUND      0x0003 03       1st event recorded after ALT TAB PRESSED.
    EVENT_SYSTEM_CAPTURESTART    0x0008 08       A window has received mouse capture. This event is sent by the system, never by servers.
    EVENT_OBJECT_FOCUS           0x8005 32773    An object has received the keyboard focus
    EVENT_OBJECT_SHOW            0x8002 32770    A hidden object is shown.
 >> EVENT_SYSTEM_SWITCHSTART     0x0014 20       The user has pressed ALT+TAB, which activates the switch window.
    EVENT_OBJECT_FOCUS           0x8005 32773
    EVENT_OBJECT_LOCATIONCHANGE  0x800B 32779    An object has changed location, shape, or size...
    EVENT_SYSTEM_CAPTUREEND      0x0009 09       A window has lost mouse capture.
    EVENT_SYSTEM_SWITCHEND       0x0015 21
    EVENT_OBJECT_HIDE            0x8003 32771
    EVENT_OBJECT_STATECHANGE     0x800A 32778
    EVENT_OBJECT_DESTROY         0x8001 32769    An object has been destroyed. The system sends this event for the following user interface elements: caret, header control, 
    EVENT_OBJECT_CREATE          0x8001 32768
    EVENT_OBJECT_NAMECHANGE      0x800c 32780    An object's Name property has changed. The system sends this event for the following user interface elements: cursor, list-view control,...
    EVENT_OBJECT_PARENTCHANGE    0x800f 32783    An object has a new parent object. Server applications send this event for their accessible objects.
   */

    if (event == EVENT_SYSTEM_SWITCHSTART) {

        if (hwnd != NULL) {

            g_CurrentMonitor = GetMouseMonitor(g_monitors, g_num_monitors);
            
            // Get the rect size of the window.
            RECT wrect = {0};
            GetWindowRect(hwnd, &wrect);
            LONG  wwidth = wrect.right - wrect.left;
            LONG  wheight = wrect.bottom - wrect.top;
            
            LONG npX = (mmon.left + (((mmon.right)  - mmon.left)/2)) - (wwidth / 2) ;
            LONG npY = (mmon.top  + (((mmon.bottom) - mmon.top)/2))- (wheight / 2) ;            
            SetWindowPos(hwnd, HWND_TOPMOST, npX, npY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
        }
    }
    
    if (event == EVENT_SYSTEM_SWITCHEND ) {
        SendMessage(g_hwnd, WM_EVENT_TASKSWITCHER, 0, 0);
   }
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {

        case WM_EVENT_TASKSWITCHER: {

            // Check if the "Esc" key was pressed. if it was, dont move anything.
            if (EcsKey) { goto end; }

            INT retry = 0;
            retry_GetForegroundWindow:
            hwnd = GetForegroundWindow();
            if ((hwnd != NULL) && ( hwnd != g_hwnd_taskswitcher)) {
                // Need to apply some rules!
                // If the window is on the same screen as the monitor. don't move it.
                // If the window is in full-screen mode. don't move it. unless its example a browser with video in fullscreen. (todo)
                // If the window is maximized. maximize it on the target desktop as well.
                // If the window is bigger than the target monitor, set it to left,top pos and maximize it.
                // If the window is wider than the target monitor but the window area is smaller. set it to left,top pos of the monitor.
                // If the window is taller/higher than the target monitor but the window area is smaller. set it to left,top pos of the monitor.               

                // Get the rect size of the window.
                RECT wrect = {0};
                MONITORINFO monitorInfo = {0};
                monitorInfo.cbSize = sizeof(MONITORINFO);
                HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);     
                LONG  wwidth;
                LONG  wheight;
                
                lable_start:
                GetWindowRect(hwnd, &wrect);
                wwidth = wrect.right - wrect.left;
                wheight = wrect.bottom - wrect.top;                
                                
                // The MonitorFromWindow function retrieves a handle to the display monitor.
                // 1. Check the monitor the window is on.
                // if it is already on the monitor, then don't move it.
                for (INT i = 0; i < g_num_monitors; i++) {
                    if (EqualRect(&g_monitors[i].rect, &monitorInfo.rcMonitor)) {
                        if (g_CurrentMonitor == i) {
                            goto end;
                        }
                    }
                }

                // 2. Check if the window is FullScreen.
                // NEEDS MORE WORK..
               if (GetMonitorInfo(hMonitor, &monitorInfo)) {
                   // Compare the desktop rect with the program rect.
                   // If it's the same size, its fullscreen, but more checks needed. (TODO)
                    if (EqualRect(&wrect, &monitorInfo.rcMonitor)) {
                        goto end;
                    }
                }

                // Check if the window is Minimized.
                if (IsIconic(hwnd)) {
                    SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
                    Sleep(50);
                    goto lable_start;

                }

                // 3. Check if the window is Maximized.
                else if (IsZoomed(hwnd)) {
                    SetWindowPos(hwnd, HWND_TOP, mmon.left, mmon.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                    SendMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
                    goto end;
                
                } // (4,5,6) More check if the window is a "Normal" window.
                else {
                    // Calculate the area of the monitor and window.
                    LONG monSz = ((mmon.right)  - mmon.left) * ((mmon.bottom) - mmon.top);
                    LONG windowSz = (wrect.right - wrect.left) * (wrect.bottom - wrect.top);

                    // 4. Check if the window area is bigger than the target monitor.
                    // If it is bigger. maximize it.
                    if (monSz < windowSz) {
                        SetWindowPos(hwnd, HWND_TOP, mmon.left, mmon.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                        SendMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
                        goto end;
                   // 7. 
                   } else {

                        LONG npX = mmon.left + ( wrect.left - monitorInfo.rcMonitor.left);
                        LONG npY = mmon.top + (wrect.top - monitorInfo.rcMonitor.top );
                        
                        // Check if the area of rectangle b is inside rectangle a
                        RECT testpos;
                        testpos.left = npX;
                        testpos.right = npX + (wrect.right - wrect.left) ;
                        testpos.top = npY;
                        testpos.bottom = npY + (wrect.bottom - wrect.top);
                        
                        if (RECT_INSIDE( mmon, testpos)) {
                            //SetWindowPos(hwnd, HWND_TOPMOST, npX, npY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                            SetWindowPos(hwnd, HWND_TOPMOST, npX, npY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
                            goto end;
                        }              
                    }
                    
                    // 5. Check the width, check if the window is wider than the monitor
                    if (wwidth > (mmon.right - mmon.left)) {
                        SetWindowPos(hwnd, HWND_TOP, mmon.left, mmon.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                        goto end;
                    }                
                    
                    // 6. Check the height
                    if (wheight > (mmon.bottom - mmon.top)) {
                        SetWindowPos(hwnd, HWND_TOP, mmon.left, mmon.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                        goto end;
                    }

                    // oki then.. just place the window in the center of the monitor.
                    LONG npX = (mmon.left + (((mmon.right)  - mmon.left)/2)) - (wwidth / 2) ;
                    LONG npY = (mmon.top  + (((mmon.bottom) - mmon.top)/2))- (wheight / 2) ;
                    SetWindowPos(hwnd, HWND_TOPMOST, npX, npY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);

                }
            } else 
            {
                // This can happen if a monitor is disconnected and the window is on it.
                // while the desktop is blinking and whatnot after its been connected/disconnected.
                // jump back up and retry to get the GetForegroundWindow
                // try a couple of times. else ignore failure.
                retry++;
                if (retry < 3) {
                    Sleep(20);
                    goto retry_GetForegroundWindow;
                }
            }
        } break;
        
        case WM_DISPLAYCHANGE: {
            // A monitor that is turned off does not trigger this but,
            // monitors connecting/disconnecting triggers this.
            // Also games running in exclusive fullscreen mode.
            // Nneed to updated info about monitors.
            if (CollectMonitorInfo(g_monitors) == FALSE) {
                 exitCode=ERR5; PostQuitMessage(0);
            }
        } break;
        
        case WM_CREATE: {}
        case WM_CLOSE: {} break;
        case WM_DESTROY: { PostQuitMessage(0); } break;
        case WM_TIMER: {} break;        
    }
    
    end:
    EcsKey = FALSE;
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WinKeyboardProc(INT nCode, WPARAM wParam, LPARAM lParam) {
    static BOOL altKeyDown = FALSE;

    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *pKeyStruct = (KBDLLHOOKSTRUCT *)lParam;

        // Check for "Alt" AND "Tab" key down press.
        if (( wParam == WM_SYSKEYDOWN) && (pKeyStruct->vkCode == VK_TAB) && (altKeyDown==FALSE))  {
            // Alt key was pressed down
            altKeyDown = TRUE;
        }
        
        // Check if Left Alt was released,
        if (wParam == WM_KEYUP) {
            if (pKeyStruct->vkCode == VK_LMENU) {
                altKeyDown = FALSE;
            }
        }
        
        // Check for "Alt" AND "esc" key down press. 
        // Used as a check because we dont want to move a window if esc was pressed in the Task Switcher window..
        if (( wParam == WM_SYSKEYDOWN) && (pKeyStruct->vkCode == VK_ESCAPE) && (altKeyDown==TRUE))  {
            // Alt key was pressed down
            EcsKey = TRUE;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


//=================================================================
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prev, LPSTR pCmdLine, int nCmdShow ) {

    // ====================================================
    // We only want to run one instance of this program.
    CreateMutexW( 0, FALSE, L"AltTabMod");
    if(GetLastError() == ERROR_ALREADY_EXISTS) { 
        return ERR1;
    }

    // ====================================================
    // Load the resource stuff so multiple languages are supported.
    // Get the OS so we can load the correct file that has all the suff we need.
    // Thanks to Mysoft for figuring out the dll files the locales where in and the ID's :D 
    
    // Win 10 or newer
    if (IsWindows10OrGreater()) {
        
        HMODULE hMod = LoadLibraryEx("twinui.pcshell.dll",NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (hMod != NULL) {
            
            // load the string with the correct language.
            INT res = LoadStringW(hMod, 1513, g_taskswitcherName, sizeof(g_taskswitcherName) / sizeof(g_taskswitcherName[0]) );
            if (res == 0) {
                wcscat(g_taskswitcherName, L"Task Switching"); // set to default.
            }
            WinVersion=WIN10;
        }else{
            // exit. need the dll stuff working for languages..
            return ERR2;
        } 
    }
    // Win Vista or newer.
    else if (IsWindowsVistaOrGreater()) {
        
        HMODULE hMod = LoadLibraryEx("alttab.dll" ,NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (hMod != NULL) {
            
            // load the string with the correct language.
            INT res = LoadStringW(hMod, 1000, g_taskswitcherName, sizeof(g_taskswitcherName) / sizeof(g_taskswitcherName[0]) );
            if (res == 0) {
                wcscat(g_taskswitcherName, L"Task Switching"); // if failed set to default to load the string. unknow language ?.
            }
            WinVersion=WIN7;
        }else{
            // exit. need the dll stuff working for languages..
            return ERR3;
        }
    } 
    // too old.. not supported.
    else {
        // too old.. not supported.
        return ERR4;
    }
 
    // ====================================================
    // Collect number of monitors and their rects.
    if (CollectMonitorInfo(g_monitors) == FALSE) { return ERR5; };
    
    // ====================================================
    // Set the keyboard hook
    g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, WinKeyboardProc, NULL, 0);
    if (g_keyboardHook == NULL) { return ERR6; }

    // ====================================================
    // Register the window class
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WinProc;  // Set the window procedure to our custom function
    wc.hInstance = hInstance;
    wc.lpszClassName = L"AltTabModWindowClass";
    RegisterClassW(&wc);  // Register the window class with the system
    
    // Create a invisible window. will be used for messages.
    g_hwnd = CreateWindowW(wc.lpszClassName, L"AltTabMod", 0, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
    
    
    // ====================================================
    // This function creates a snapshot of the current processes in the system.
    // The snapshot can be used to gather information about running processes.
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) { return ERR7; }    
    
    // ====================================================
    // Find the explorer.exe process!
    // Need to hook into it. as it does create the "ALT TAB" window sort of. 
    // But only if explorer is running, there is a basic version of the task switcher that idk how is controlled if explorer is not running.
    PROCESSENTRY32 processEntry = {0};
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(snapshot, &processEntry)) {
        do {
            if (strcmp(processEntry.szExeFile, "explorer.exe") == 0) {
                
                // Set up a event hook for the process, we can probably tweak this a bit.
                switch(WinVersion) {
                    case (WIN10): {
                        g_eventHook = SetWinEventHook(EVENT_MIN, EVENT_MAX, NULL, WinEventProc_Win10, processEntry.th32ProcessID, 0, WINEVENT_OUTOFCONTEXT);
                        if (g_eventHook == 0) { return ERR8; }
                    
                    } break;
                    
                    case (WIN7): {
                        g_eventHook = SetWinEventHook(EVENT_MIN, EVENT_MAX, NULL, WinEventProc_Win7, processEntry.th32ProcessID, 0, WINEVENT_OUTOFCONTEXT);
                        if (g_eventHook == 0) { return ERR8; }
                    } break;
                    
                    case (UNKNOWN): {
                        return ERR4;
                    } break;
                }
                
                // exit loop
                break;
            }
        } while (Process32Next(snapshot, &processEntry));
    }
    CloseHandle(snapshot);

    // ====================================================
    // Main message loop. Not used.
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }    

    UnhookWinEvent(g_eventHook);
    UnhookWindowsHookEx(g_keyboardHook);
        
    ExitProcess(exitCode);
    return exitCode;
}

/**
Exit codes:

0 ERR0, Termination ok
1 ERR1, already running
2 ERR2, loading twinui.pcshell.dll failed
3 ERR3, loading alttab.dll failed
4 ERR4, Too old version of windows..
5 ERR5, Collecting Monitor Info failed
6 ERR6, Setting the keyboard hook failed
7 ERR7, failed to get a snapshot of processes
8 ERR8, failed to set a hook on explorer.exe

*/


   
