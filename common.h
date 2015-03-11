#ifndef _common_h
#define _common_h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#define API extern "C" __declspec(dllexport)


#define HM_QUIT				(WM_USER+1)
#define HM_ADD_HOTKEY		(WM_USER+2)


enum Error {
	E_OK=0,
	E_NULL_ARG,
	E_OUT_OF_MEMORY,
	E_EVENT_CREATE,
	E_EVENT_USAGE,
	E_NOT_RUNNING,
	E_STILL_RUNNING,
	E_ALREADY_RUNNING,
	E_START_TIMEOUT,
	E_STOP_TIMEOUT,
	E_ATOM_CREATE,
	E_HOTKEY_NOT_INIT,
	E_TIMEOUT,
	E_HOTKEY_REGISTER,
	E_HOTKEY_UNREGISTER
};


#endif //_common_h
