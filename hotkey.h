#ifndef _hotkey_h
#define _hotkey_h

#include "common.h"


#define ID_SIZE 32


typedef void (__cdecl *Callback)(LPVOID);


typedef struct _HOTKEY {
	ATOM id;
	UINT vk;
	UINT mod;
	Callback callback;
	LPVOID param;
} HOTKEY, *PHOTKEY;


int hkCreate(PHOTKEY hk, UINT vk, UINT mod=0, Callback callback=NULL, LPVOID param=NULL);
int hkDelete(PHOTKEY hk);

int hkSet(PHOTKEY hk, UINT vk, UINT mod=0, Callback callback=NULL, LPVOID param=NULL);

#endif //_hotkey_h
