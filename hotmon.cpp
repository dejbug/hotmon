#include "hotmon.h"
#include "hotkey.h"
#include "debug.h"

#include <stdio.h>
#include <process.h>
#include <stdexcept>
#include <vector>
#include <time.h>
#include <stdlib.h>


BOOL WINAPI DllMain(HINSTANCE i, DWORD r, LPVOID) {
	switch(r) {
		case DLL_PROCESS_ATTACH: break;
		case DLL_PROCESS_DETACH: break;
		case DLL_THREAD_ATTACH: break;
		case DLL_THREAD_DETACH: break;
	}
	return TRUE;
}


void SendCloseMessage(HWND h) {
	SendMessage(h, WM_CLOSE, 0, 0);
}


class HotmonWindowClass {
private:
	WNDCLASSEX wc;
	bool owned;
public:
	HotmonWindowClass(WNDPROC proc, bool owned=true) : owned(owned) {
		memset(&wc, 0, sizeof(WNDCLASSEX));
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.lpfnWndProc = proc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.lpszClassName = "HOTMON_CLASS";
		
		WNDCLASSEX info;
		memset(&info, 0, sizeof(WNDCLASSEX));
		if(GetClassInfoEx(wc.hInstance, wc.lpszClassName, &info)) {
			OutputDebugString("! hotmon window class already registered");
			return;
		}
		
		if(!RegisterClassEx(&wc)) {
			OutputDebugString("! unable to register hotmon window class");
			throw std::runtime_error("couldn't register hotmon window class");
		}
		else OutputDebugString("* hotmon window class registered");
	}
	virtual ~HotmonWindowClass() {
		if(!owned) return;
		UnregisterClass(wc.lpszClassName, wc.hInstance);
		OutputDebugString("* hotmon window class unregistered");
	}
	operator WNDCLASSEX() { return wc; }
	LPCSTR name() const { return wc.lpszClassName; }
	HINSTANCE instance() const { return wc.hInstance; }
};


class HotmonWindow {
private:
	HWND handle;
public:
	HotmonWindow(const HotmonWindowClass& wc, LPCSTR title="hotmon") {
		handle = CreateWindowEx(0, wc.name(), title, 0,
			CW_USEDEFAULT, CW_USEDEFAULT, 100, 100,
			NULL, NULL, wc.instance(), NULL);
		if(!handle) {
			OutputDebugString("! unable to create hotmon window");
			throw std::runtime_error("couldn't create hotmon window");
		}
	}
	virtual ~HotmonWindow() {
	}
	operator HWND() { return handle; }
};


int RunMainLoop() {
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}


LRESULT CALLBACK MonitorProc(HWND h, UINT m, WPARAM w, LPARAM l) {
	static std::vector<HOTKEY> kk;
	
	switch(m) {
		case WM_CREATE: {
			OutputDebugString("WM_CREATE");
			break;
		}
		case WM_CLOSE: {
			OutputDebugString("WM_CLOSE");
			DestroyWindow(h);
			break;
		}
		case WM_DESTROY: {
			OutputDebugString("WM_DESTROY");
			//hk = std::auto_ptr<Hotkey>();
			PostQuitMessage(0);
			break;
		}
		case WM_HOTKEY: {
			const ATOM id = (ATOM)w;
			const UINT mod = (UINT)LOWORD(l);
			const UINT vk = (UINT)HIWORD(l);
			OutputDebugString("WM_HOTKEY");
			debugf("# looking for hotkey with id %08X", id);
			for(int i=0; i<kk.size(); ++i) {
				if(kk[i].id == id) {
					debugf("# found %d-th hotkey with id %08X", i, id);
					if(kk[i].callback) kk[i].callback(kk[i].param);
					break;
				}
			}
			return 0;
		}
		case HM_QUIT: {
			PHOTMON* hm = (PHOTMON*)w;
			OutputDebugString("HM_QUIT");
			
			for(int i=0; i<kk.size(); ++i) {
				hmUnregister(hm, &kk[i]);
				hkDelete(&kk[i]);
				(*hm)->tid = 0;
			}
			
			//TODO: To be extra safe, we should store the PHOTMON
			//	and signal its event before the MainLoop returns.
			SetEvent((*hm)->event);
			
			SendCloseMessage(h);
			return 0;
		}
		case HM_ADD_HOTKEY: {
			PHOTMON* hm = (PHOTMON*)w;
			PHOTKEY hk = (PHOTKEY)l;
			OutputDebugString("HM_ADD_HOTKEY");
			
			hmRegister(hm, hk);
			kk.push_back(*hk);
			debugf("# tracking %d hotkey(s) now", kk.size());
			return 0;
		}
		default: break;
	}
	return DefWindowProc(h, m, w, l);
}


void hmThread(LPVOID param) {
	PHOTMON* hm = (PHOTMON*)param;
	HotmonWindowClass wc(MonitorProc, false);
	HotmonWindow wnd(wc);
	(*hm)->hwnd = wnd;
	SetEvent((*hm)->event);
	RunMainLoop();
}


API int hmCreate(PHOTMON* hm) {
	if(!hm) return E_NULL_ARG;
	
	srand(time(NULL));
	
	*hm = new HOTMON();
	if(!*hm) return E_OUT_OF_MEMORY;
	
	memset(*hm, 0, sizeof(HOTMON));
	
	// create the notifier event. this will be
	// used to wait for the thread to start up,
	// so we don't start adding our hotkeys before
	// the window will be able to handle them.
	(*hm)->event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(!(*hm)->event) return E_EVENT_CREATE;
	
	return E_OK;
}


API int hmDelete(PHOTMON* hm) {
	if(!hm) return E_NULL_ARG;
	if((*hm)->tid) return E_STILL_RUNNING;
	
	CloseHandle((*hm)->event);
	memset(*hm, 0, sizeof(HOTMON));
	delete *hm;
	
	return E_OK;
}


API int hmStart(PHOTMON* hm, DWORD msec) {
	if(!hm) return E_NULL_ARG;
	if((*hm)->tid) return E_ALREADY_RUNNING;
	
	ResetEvent((*hm)->event);
	
	(*hm)->tid = _beginthread(hmThread, 0, (LPVOID)hm);
	
	if(WAIT_OBJECT_0 != WaitForSingleObject((*hm)->event, msec))
		return E_START_TIMEOUT;
		
	return E_OK;
}


API int hmStop(PHOTMON* hm, DWORD msec) {
	if(!hm) return E_NULL_ARG;
	if(!(*hm)->tid) return E_NOT_RUNNING;
	
	ResetEvent((*hm)->event);
	
	SendMessage((*hm)->hwnd, HM_QUIT, (WPARAM)hm, 0);
	
	if(WAIT_OBJECT_0 != WaitForSingleObject((*hm)->event, msec))
		return E_STOP_TIMEOUT;
		
	return E_OK;
}


/*
API int hmAddHotkey(PHOTMON* hm, UINT vk, UINT mod, Callback callback, LPVOID param) {
	if(!hm) return E_NULL_ARG;
	if(!(*hm)->tid) return E_NOT_RUNNING;
	HOTKEY hk;
	memset(&hk, 0, sizeof(hk));
	hkCreate(&hk, vk, mod, callback, param);
	return hmAddHotkey(hm, &hk);
}
*/


API int hmAddHotkey(PHOTMON* hm, PHOTKEY hk) {
	if(!hm) return E_NULL_ARG;
	if(!hk) return E_NULL_ARG;
	if(!(*hm)->tid) return E_NOT_RUNNING;
	SendMessage((*hm)->hwnd, HM_ADD_HOTKEY, (WPARAM)hm, (LPARAM)hk);
	return E_OK;
}


API int hmCreateHotkey(PPHOTMON hm, PHOTKEY hk, UINT vk, UINT mod, Callback callback, LPVOID param) {
	if(!hm) return E_NULL_ARG;
	if(!hk) return E_NULL_ARG;
	if(!(*hm)->tid) return E_NOT_RUNNING;
	memset(hk, 0, sizeof(HOTKEY));
	hkCreate(hk, vk, mod, callback, param);
	return E_OK;
}


API int hmRegister(PHOTMON* hm, PHOTKEY hk) {
	if(!hm) return E_NULL_ARG;
	if(!hk) return E_NULL_ARG;
	if(!hk->id) E_HOTKEY_NOT_INIT;
	if(!RegisterHotKey((*hm)->hwnd, hk->id, hk->mod, hk->vk))
			OutputDebugString("! failed to register hotmon hotkey");
		else OutputDebugString("* hotmon hotkey registered");
}


API int hmUnregister(PHOTMON* hm, PHOTKEY hk) {
	if(!hm) return E_NULL_ARG;
	if(!hk) return E_NULL_ARG;
	if(!hk->id) E_HOTKEY_NOT_INIT;

	if(!UnregisterHotKey((*hm)->hwnd, hk->id))
		OutputDebugString("! failed to unregister hotmon hotkey");
	else OutputDebugString("* hotmon hotkey unregistered");
}

