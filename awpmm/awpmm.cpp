#include <iostream>
#include <windows.h>
#include <cwctype>
#include <shlobj.h>
#include <atlbase.h>
#include <atlalloc.h>

// Thanks Raymond Chen 
	// https://devblogs.microsoft.com/oldnewthing/?p=833
	// https://devblogs.microsoft.com/oldnewthing/20040520-00/?p=39243


class CCoInitialize {
public:
	CCoInitialize() : m_hr(CoInitialize(NULL)) { }
	~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
	operator HRESULT() const { return m_hr; }
	HRESULT m_hr;
};

int save() {
	
	CCoInitialize init;
	// Create the DesktopWallpaper object
	CComPtr<IDesktopWallpaper> spdw;
	CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL,
		IID_PPV_ARGS(&spdw));
	// See if there is a single wallpaper on all monitors.
	CComHeapPtr<wchar_t> spszCommonWallpaper;
	HRESULT hr = spdw->GetWallpaper(nullptr, &spszCommonWallpaper);
	switch (hr) {
	case S_OK:
		printf("Same wallpaper on all monitors: %ls\n",
			static_cast<wchar_t *>(spszCommonWallpaper));
		break;
	case S_FALSE:
		printf("Different wallpaper on each monitor\n");
		break;
	default:
		printf("Mysterious error: 0x%08x\n", hr);
		break;
	}
	// Get the number of monitors,
	UINT count;
	spdw->GetMonitorDevicePathCount(&count);
	printf("There are %d monitors\n", count);
	// Print information about each monitor to console and to file

	FILE *out;
	if (fopen_s(&out, "wallpapers.dat", "w") == 0) {
		for (UINT i = 0; i < count; i++) {
			// Get the device path for the monitor.
			CComHeapPtr<wchar_t> spszId;
			spdw->GetMonitorDevicePathAt(i, &spszId);
			printf("path[%d] = %ls\n",
				i, static_cast<wchar_t *>(spszId));
		
			// Get the wallpaper on that monitor.
			CComHeapPtr<wchar_t> spszWallpaper;
			hr = spdw->GetWallpaper(spszId, &spszWallpaper);
			printf("image = %ls\n",
				static_cast<wchar_t *>(spszWallpaper));

			fprintf(out, "%ls,%ls\n", static_cast<wchar_t *>(spszId), static_cast<wchar_t *>(spszWallpaper));
		}

		fclose(out);
	} else {
		printf("Could not open file.\n");
		return 1;
	}

	return 0;
}

int restore() {

	CCoInitialize init;
	// Create the DesktopWallpaper object
	CComPtr<IDesktopWallpaper> spdw;
	CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL,
		IID_PPV_ARGS(&spdw));

	
	FILE *in;
	if (fopen_s(&in, "wallpapers.dat", "r") == 0) {
		wchar_t line[4096];
		while (fgetws(line, 4096, in)) {
			wcschr(line, L'\n')[0] = 0;
			wchar_t* comma = wcschr(line, L',');
			comma[0] = 0;


			wchar_t* id = line;
			wchar_t* wallpaper = comma + 1;

			if (wcslen(id) != 0 && wcslen(wallpaper) != 0) {
				printf("%ls,%ls\n", id, wallpaper);
				HRESULT hr = spdw->SetWallpaper(id, wallpaper);
				printf("HR: %d\n", hr);
			} else {
				printf("Borked line.\n");
			}
		}

		fclose(in);
	} else {
		printf("Could not open file.\n");
		return 1;
	}

	return 0;
}

int error() {
	printf("Must specify [s]ave or [r]estore.\n");
	return 1;
}

int wmain(int argc, wchar_t *argv[], wchar_t *envp[]) 
{
	if (argc != 2) {
		return error();
	} else {
		wchar_t lowerfirst = static_cast<wchar_t>(std::towlower(argv[1][0]));
		if (lowerfirst == 's') {
			return save();
		} else if (lowerfirst == 'r') {
			return restore();
		} else {
			return error();
		}
	}
}


