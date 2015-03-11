#include <stdlib.h>
#include "hotkey.h"
#include "debug.h"


template<size_t N> void MakeRandomString(char buf[N], LPCSTR alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZ") {
	if(!N || !alphabet || !*alphabet) return;
	const size_t range = strlen(alphabet);
	for(int i=0; i<N; ++i) {
		const int n = rand()%range;
		buf[i] = alphabet[n];
	}
	buf[N-1] = '\0';
}


int hkCreate(PHOTKEY hk, UINT vk, UINT mod, Callback callback, LPVOID param) {
	if(!hk) return E_NULL_ARG;
	
	hkSet(hk, vk, mod, callback, param);
	
	char name[ID_SIZE];
	MakeRandomString<ID_SIZE>(name);
	
	hk->id = GlobalAddAtom(name);
	if(!hk->id) {
		debugf("! failed to register hotkey atom");
		return E_ATOM_CREATE;
	}
	else debugf("* hotkey atom registered");
	
	return E_OK;
}


int hkDelete(PHOTKEY hk) {
	if(!hk) return E_NULL_ARG;
	
	if(hk->id) {
		if(!DeleteAtom(hk->id))
			debugf("* hotkey atom unregistered");
		else debugf("! failed to unregister hotkey atom");
	}
	
	return E_OK;
}


int hkSet(PHOTKEY hk, UINT vk, UINT mod, Callback callback, LPVOID param) {
	if(!hk) return E_NULL_ARG;
	
	hk->vk = vk;
	hk->mod = mod;
	hk->callback = callback;
	hk->param = param;
	
	return E_OK;	
}