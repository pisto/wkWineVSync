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
HRESULT WINAPI (*nextCreateSurface)(LPDIRECTDRAW* a, DDSURFACEDESC2* b, LPDIRECTDRAW* c, IUnknown* UnkOuter);
HRESULT WINAPI (*nextFlip)(LPDIRECTDRAW* a, LPDIRECTDRAWSURFACE* b, DWORD c);

PFNGLXGETVIDEOSYNCSGIPROC m_glXGetVideoSyncSGI;
PFNGLXWAITVIDEOSYNCSGIPROC m_glXWaitVideoSyncSGI;

void cleanup(){
	if(nextFlip){
		UnhookCode((PVOID*)&nextFlip);
		nextFlip = 0;
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

HRESULT WINAPI myFlip(LPDIRECTDRAW* a, LPDIRECTDRAWSURFACE* b, DWORD c){
	uint counter;
	if(!m_glXGetVideoSyncSGI(&counter)) m_glXWaitVideoSyncSGI(2, (counter+1)%2, &counter);
	else{
		cleanup();
		cerr<<"glXGetVideoSyncSGI failed, I suicide.\n";
	}
	return nextFlip(a, b, c);
}

HRESULT WINAPI myCreateSurface(LPDIRECTDRAW* a, DDSURFACEDESC2* b, LPDIRECTDRAW* lplpDDSurface, IUnknown* d){
	auto ret = nextCreateSurface(a, b, lplpDDSurface, d);
	if(ret == DD_OK && !nextFlip)
		if(!HookCode(vtb((*lplpDDSurface), 11), (PVOID)myFlip, (PVOID*)&nextFlip, 0)){
			cleanup();
			cerr<<"Cannot hook IDirectDrawSurface2::Flip, I suicide.\n";
		}
	return ret;
}

HRESULT WINAPI myDirectDrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter){
	auto ret = nextDirectDrawCreate(lpGUID, lplpDD, pUnkOuter);
	if(ret == DD_OK && !nextCreateSurface){
		LPDIRECTDRAW2 ddraw2 = 0;
		try{
			if((*lplpDD)->QueryInterface(IID_IDirectDraw2, (LPVOID*)&ddraw2) != S_OK) throw "Cannot query IDirectDraw2";
			if(!HookCode(vtb(ddraw2, 6), (PVOID)myCreateSurface, (PVOID*)&nextCreateSurface, 0)) throw "Cannot hook IDirectDraw2::CreateSurface";
		} catch(const char* msg){
			cleanup();
			cerr<<msg<<", I suicide.\n";
		}
		if(ddraw2) ddraw2->Release();
	}
	return ret;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD reason, LPVOID reserved){
	if(reason == DLL_PROCESS_ATTACH) try{
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
	} catch(const char* msg){
		cleanup();
		MessageBox(NULL, msg, "wkWineVSync fatal error", MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

