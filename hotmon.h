#ifndef _hotmon_h
#define _hotmon_h

#include "common.h"
#include "hotkey.h"


typedef struct _HOTMON {
	HWND hwnd;
	HANDLE event;
	DWORD tid;
} HOTMON, *PHOTMON, **PPHOTMON;


API int hmCreate(PHOTMON* hm);
API int hmDelete(PHOTMON* hm);
API int hmStart(PHOTMON* hm, DWORD msec=3000);
API int hmStop(PHOTMON* hm, DWORD msec=3000);

API int hmCreateHotkey(PPHOTMON hm, PHOTKEY hk, UINT vk, UINT mod=0, Callback callback=NULL, LPVOID param=NULL);
API int hmAddHotkey(PHOTMON* hm, PHOTKEY hk);

API int hmRegister(PHOTMON* hm, PHOTKEY hk);
API int hmUnregister(PHOTMON* hm, PHOTKEY hk);

#endif //_hotmon_h
