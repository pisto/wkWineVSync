#include <windows.h>

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD reason, LPVOID reserved){
	switch(reason){
	case DLL_PROCESS_ATTACH: break;
	case DLL_PROCESS_DETACH: break;
	}
	return TRUE;
}

