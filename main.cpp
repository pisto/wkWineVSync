#include <iostream>
#include <windows.h>
#include <ddraw.h>
#include <GL/glx.h>
using namespace std;

HMODULE madCHook;
BOOL WINAPI (*HookAPI)(LPCSTR, LPCSTR, PVOID, PVOID*, DWORD);
BOOL WINAPI (*UnhookAPI)(PVOID*);
BOOL WINAPI (*HookCode)(PVOID, PVOID, PVOID*, DWORD);
BOOL WINAPI (*UnhookCode)(PVOID*);
#define vtb(thing, indx) ((*(void***)(thing))[indx])

decltype(DirectDrawCreate)* nextDirectDrawCreate;
HRESULT WINAPI (*nextWaitForVerticalBlank)(LPDIRECTDRAW*, DWORD, HANDLE);

PFNGLXGETVIDEOSYNCSGIPROC m_glXGetVideoSyncSGI;
PFNGLXWAITVIDEOSYNCSGIPROC m_glXWaitVideoSyncSGI;

void cleanup(){
	if(nextWaitForVerticalBlank){
		UnhookCode((PVOID*)&nextWaitForVerticalBlank);
		nextWaitForVerticalBlank = 0;
	}
	if(nextDirectDrawCreate){
		UnhookAPI((PVOID*)&nextDirectDrawCreate);
		nextDirectDrawCreate = 0;
	}
	HookAPI = 0;
	UnhookAPI = 0;
	HookCode = 0;
	UnhookCode = 0;
	if(madCHook){
		FreeLibrary(madCHook);
		madCHook = 0;
	}
}

HRESULT WINAPI myWaitForVerticalBlank(LPDIRECTDRAW* a, DWORD b, HANDLE c){
	uint counter;
	if(!m_glXGetVideoSyncSGI(&counter)) m_glXWaitVideoSyncSGI(2, (counter+1)%2, &counter);
	else{
		cleanup();
		cerr<<"glXGetVideoSyncSGI, I suicide.\n";
	}
	return DD_OK;
}

HRESULT WINAPI myDirectDrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter){
	auto ret = nextDirectDrawCreate(lpGUID, lplpDD, pUnkOuter);
	if(ret == DD_OK && !nextWaitForVerticalBlank){
		//actual wine implementation is in IDirectDraw7, hook that one only
		LPDIRECTDRAW7 ddraw7;
		if((*lplpDD)->QueryInterface(IID_IDirectDraw7, (LPVOID*)&ddraw7) != S_OK){
			cleanup();
			cerr<<"Cannot query IDirectDraw7, I suicide.\n";
		}
		else{
			if(!HookCode(vtb(ddraw7, 22), (PVOID)myWaitForVerticalBlank, (PVOID*)&nextWaitForVerticalBlank, 0)){
				cleanup();
				cerr<<"Cannot hook WaitForVerticalBlank, I suicide.\n";
			}
			ddraw7->Release();
		}
	}
	return ret;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD reason, LPVOID reserved){
	switch(reason){
	case DLL_PROCESS_ATTACH: try{
		if(!(madCHook = LoadLibrary("madCHook.dll"))) throw "Cannot load madCHook.dll";
#define get(fn) if(!(fn = decltype(fn)(GetProcAddress(madCHook, #fn)))) throw "Cannot find " #fn
		get(HookAPI);
		get(UnhookAPI);
		get(HookCode);
		get(UnhookCode);
#undef get
		if(!HookAPI("ddraw.dll", "DirectDrawCreate", (PVOID)myDirectDrawCreate, (PVOID*)&nextDirectDrawCreate, 0));
#define get(fn) if(!(m_##fn = decltype(m_##fn)(glXGetProcAddress((const GLubyte*)#fn)))) throw "Cannot find " #fn
		get(glXGetVideoSyncSGI);
		get(glXWaitVideoSyncSGI);
#undef get
		break;
	} catch(const char* msg){
		cleanup();
		MessageBox(NULL, msg, "wkWineVSync fatal error", MB_ICONERROR);
		return FALSE;
	}
	case DLL_PROCESS_DETACH: cleanup();
	}
	return TRUE;
}

