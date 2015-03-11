#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include "hotmon.h"


void __cdecl f10_callback(LPVOID) {
	printf("f10\n");
}


void __cdecl f11_callback(LPVOID) {
	printf("f11\n");
}


int main() {
	PHOTMON hm = NULL;
	hmCreate(&hm);
	hmStart(&hm);
	Sleep(1000);
	
	HOTKEY hk1, hk2;
	hmCreateHotkey(&hm, &hk1, VK_F3, 0, f10_callback, NULL);
	hmCreateHotkey(&hm, &hk2, VK_F4, 0, f11_callback, NULL);
	
	hmAddHotkey(&hm, &hk1);
	hmAddHotkey(&hm, &hk2);
	
	Sleep(5000);
	
	hmStop(&hm);
	hmDelete(&hm);
	Sleep(1000);
	
	return 0;
}
