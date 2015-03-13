#include "hotmon.h"
#include "hotkey.h"
#include "debug.h"

#include <stdio.h>
#include <process.h>
#include <stdexcept>
#include <vector>
#include <time.h>
#include <stdlib.h>


BOOL WINAPI DllMain(HINSTANCE, DWORD r, LPVOID) {
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
			debugf("! hotmon window class already registered");
			return;
		}
		
		if(!RegisterClassEx(&wc)) {
			debugf("! unable to register hotmon window class");
			throw std::runtime_error("couldn't register hotmon window class");
		}
		else debugf("* hotmon window class registered");
	}
	virtual ~HotmonWindowClass() {
		if(!owned) return;
		UnregisterClass(wc.lpszClassName, wc.hInstance);
		debugf("* hotmon window class unregistered");
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
			debugf("! unable to create hotmon window");
			throw std::runtime_error("couldn't create hotmon window");
		}
	}
	virtual ~HotmonWindow() {
	}
	operator HWND() { return handle; }
public:
	void associate(PHOTMON* hm) const {
		if(!hm || !*hm)
			throw std::runtime_error("HotmonWindow::associate needs a valid PPHOTMON");
		(*hm)->hwnd = handle;
		SetWindowLong(handle, GWL_USERDATA, (LONG)hm);
	}
};


//NOTE: We could put this functionality into HotmonWindow, but i didn't
//	like to bloat the class.
//FIXME: If we wanted to signal hmStop with this, say, from within the
//	end of the RunMainLoop, the IsWindow(handle) check would need to be
//	dropped.
class HotmonContext {
private:
	HWND handle;
	PHOTMON* hm;
public:
	HotmonContext(HWND handle) : handle(handle) {
		if(!handle || !IsWindow(handle))
			throw std::runtime_error("HotmonContext needs a valid HWND");
		hm = (PHOTMON*)GetWindowLong(handle, GWL_USERDATA);
		if(!hm || !*hm)
			throw std::runtime_error("HotmonContext failed to retrieve associated PHOTMON*");
		handle = (*hm)->hwnd;
	}
	HotmonContext(PHOTMON* hm) : hm(hm) {
		if(!hm || !*hm)
			throw std::runtime_error("HotmonContext needs a valid PHOTMON*");
		handle = (*hm)->hwnd;
		if(!handle || !IsWindow(handle))
			throw std::runtime_error("HotmonContext failed to retrieve associated HWND");
	}
public:
	HWND getWindowHandle() const {
		return handle;
	}
	
	PHOTMON* getHotmonHandle() const {
		return hm;
	}
private:
	void signal() const {
		SetEvent((*hm)->event);
	}
public:
	// This is supposed to be called when the main thread starts.
	void sendStartSignal() const {
		signal();
	}
	
	// We intend to call this from the WM_DESTROY handler shortly
	//	before the WM_QUIT will break the main thread's loop.
	void sendStopSignal() const {
		// This is how the API functions tell whether the main thread
		//	is running. We set it to 0 to indicate the thread is done,
		//	even though the thread might still be running for a second
		//	longer.
		(*hm)->tid = 0;
		
		signal();
	}
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
			debugf("WM_CREATE");
			break;
		}
		case WM_CLOSE: {
			debugf("WM_CLOSE");
			DestroyWindow(h);
			break;
		}
		case WM_DESTROY: {
			debugf("WM_DESTROY");
			
			HotmonContext(h).sendStopSignal();
			
			PostQuitMessage(0);
			break;
		}
		case WM_HOTKEY: {
			const ATOM id = (ATOM)w;
			//const UINT mod = (UINT)LOWORD(l);
			//const UINT vk = (UINT)HIWORD(l);
			debugf("WM_HOTKEY");
			debugf("# looking for hotkey with id %08X", id);
			for(size_t i=0; i<kk.size(); ++i) {
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
			debugf("HM_QUIT");
			
			for(size_t i=0; i<kk.size(); ++i) {
				hmUnregister(hm, &kk[i]);
				hkDelete(&kk[i]);
			}
			
			SendCloseMessage(h);
			return 0;
		}
		case HM_ADD_HOTKEY: {
			PHOTMON* hm = (PHOTMON*)w;
			PHOTKEY hk = (PHOTKEY)l;
			debugf("HM_ADD_HOTKEY");
			
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
	wnd.associate(hm);
	
	HotmonContext(hm).sendStartSignal();
	RunMainLoop();
}


API int hmCreate(PHOTMON* hm) {
	if(!hm) return E_NULL_ARG;
	
	srand(time(NULL));
	
	*hm = new HOTMON();
	if(!*hm) return E_OUT_OF_MEMORY;
	
	memset(*hm, 0, sizeof(HOTMON));
	
	(*hm)->event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(!(*hm)->event) {
		delete hm;
		return E_EVENT_CREATE;
	}
	
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
	
	(*hm)->tid = _beginthread(hmThread, 0, (LPVOID)hm);
	
	if(WAIT_OBJECT_0 != WaitForSingleObject((*hm)->event, msec))
		return E_START_TIMEOUT;
		
	return E_OK;
}


API int hmStop(PHOTMON* hm, DWORD msec) {
	if(!hm) return E_NULL_ARG;
	if(!(*hm)->tid) return E_NOT_RUNNING;
	
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
	if(!hk->id) return E_HOTKEY_NOT_INIT;
	
	if(!RegisterHotKey((*hm)->hwnd, hk->id, hk->mod, hk->vk)) {
		debugf("! failed to register hotmon hotkey");
		return E_HOTKEY_REGISTER;
	}
	
	debugf("* hotmon hotkey registered");
	return E_OK;
}


API int hmUnregister(PHOTMON* hm, PHOTKEY hk) {
	if(!hm) return E_NULL_ARG;
	if(!hk) return E_NULL_ARG;
	if(!hk->id) return E_HOTKEY_NOT_INIT;

	if(!UnregisterHotKey((*hm)->hwnd, hk->id)) {
		debugf("! failed to unregister hotmon hotkey");
		return E_HOTKEY_UNREGISTER;
	}
	
	debugf("* hotmon hotkey unregistered");
	return E_OK;
}

