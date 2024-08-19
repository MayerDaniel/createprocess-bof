#include <windows.h>
#include "base\helpers.h"
#include <WbemCli.h>
#include <oleauto.h>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")


/**
 * For the debug build we want:
 *   a) Include the mock-up layer
 *   b) Undefine DECLSPEC_IMPORT since the mocked Beacon API
 *      is linked against the the debug build.
 */
#ifdef _DEBUG
#include "base\mock.h"
#undef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT
#endif


extern "C" {
#include "beacon.h"
#if !defined(_DEBUG) && !defined(_GTEST)
   /* DFR(OLE32, CoInitializeEx);
    DFR(OLE32, CoCreateInstance);
    DFR(OLE32, CoUninitialize);
    DFR(OLE32, CoSetProxyBlanket);
    DFR(OLEAUT32, SysAllocString);
    DFR(OLEAUT32, SysFreeString);
    DFR(OLEAUT32, VariantInit);
    DFR(OLEAUT32, SafeArrayPutElement);
    DFR(OLEAUT32, SafeArrayCreateVector);
    DFR(OLE32, CLSIDFromString);
    DFR(OLE32, IIDFromString);
    # define CoInitializeEx OLE32$CoInitializeEx
    # define CoCreateInstance OLE32$CoCreateInstance
    # define CoUninitialize OLE32$CoUninitialize
    # define CoSetProxyBlanket OLE32$CoSetProxyBlanket
    # define SysAllocString OLEAUT32$SysAllocString
    # define SysFreeString OLEAUT32$SysFreeString
    # define VariantInit OLEAUT32$VariantInit
    # define SafeArrayPutElement OLEAUT32$SafeArrayPutElement
    # define SafeArrayCreateVector OLEAUT32$SafeArrayCreateVector
    # define CLSIDFromString OLE32$CLSIDFromString
    # define IIDFromString OLE32$IIDFromString*/
#endif

    void go(char* args, int len) {
        
        DFR_LOCAL(KERNEL32, CreateProcessW);
        DFR_LOCAL(KERNEL32, CloseHandle);
        DFR_LOCAL(KERNEL32, WaitForSingleObject);
        //BeaconPrintf(CALLBACK_OUTPUT, "Executing...");
        
        datap parser;
        wchar_t* path = NULL;
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;
       

        BeaconDataParse(&parser, args, len);
        path = (wchar_t*)BeaconDataExtract(&parser, NULL);
      

        CreateProcessW(NULL, &path[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

        BeaconPrintf(CALLBACK_OUTPUT, "Success\n");
    }
}

// Define a main function for the bebug build
#if defined(_DEBUG) && !defined(_GTEST)

int main(int argc, char* argv[]) {
    // Run BOF's entrypoint
    // To pack arguments for the bof use e.g.: bof::runMocked<int, short, const char*>(go, 6502, 42, "foobar");
    bof::runMocked<wchar_t*, wchar_t*>(go, L"notepad.exe", L"\\\\127.0.0.1\\ROOT\\CIMV2");
    return 0;
}

// Define unit tests
#elif defined(_GTEST)
#include <gtest\gtest.h>

TEST(BofTest, Test1) {
    std::vector<bof::output::OutputEntry> got =
        bof::runMocked<>(go);
    std::vector<bof::output::OutputEntry> expected = {
        {CALLBACK_OUTPUT, "System Directory: C:\\Windows\\system32"}
    };
    // It is possible to compare the OutputEntry vectors, like directly
    // ASSERT_EQ(expected, got);
    // However, in this case, we want to compare the output, ignoring the case.
    ASSERT_EQ(expected.size(), got.size());
    ASSERT_STRCASEEQ(expected[0].output.c_str(), got[0].output.c_str());
}
#endif