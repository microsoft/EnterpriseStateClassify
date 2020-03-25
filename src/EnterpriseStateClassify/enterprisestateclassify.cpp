// Copyright (C) Microsoft Corporation
//
// EnterpriseStateClassify Usage:
//   EnterpriseStateClassify Connect <FLAGS> <TARGET> - Get enterprise state of target
//   EnterpriseStateClassify ConnectLoop <FLAGS> <TARGET> - Get enterprise state of target 20 times every 1.5 seconds
//   EnterpriseStateClassify ConnectLoopFast <FLAGS> <TARGET> - Get enterprise state of target 20 times
//

#include <netfw.h>
#include <stdio.h>
#include <sddl.h>

//ERROR Handling
#define FAIL_IF_DWERR(X) \
        if (X != NO_ERROR) goto CLEAN;

typedef enum _OP_TYPE
{
    OP_TYPE_INVALID = 0,
    OP_TYPE_CONNECT,
    OP_TYPE_LISTEN,
    OP_TYPE_SOCKETCREATION,
    OP_TYPE_CONNECTLOOP,
    OP_TYPE_CONNECTLOOPFAST
} OP_TYPE;

typedef enum _ARG_TYPE
{
    ARG_TYPE_OPERATION = 1,
    ARG_TYPE_FLAGS,
    ARG_TYPE_TARGET,
    ARG_TYPE_MAX,
    ARG_TYPE_HELP = 1
} ARG_TYPE;

//
// Function pointer definition for NetworkIsolationGetEnterpriseIdAsync
//
typedef DWORD(NTAPI *PNetworkIsolationGetEnterpriseIdAsync) (
    _In_z_ LPCWSTR,
    _In_   DWORD dwFlags,
    _In_opt_ void * context,
    _In_   PNETISO_EDP_ID_CALLBACK_FN callback,
    _Out_  HANDLE *hOperation
    );

//
// Function pointer definition for NetworkIsolationGetEnterpriseIdClose
//
typedef DWORD(NTAPI *PNetworkIsolationGetEnterpriseIdClose) (
    _In_  HANDLE hOperation,
          BOOL   bWaitForOperation
    );

void CALLBACK
NetworkIsolationGetEnterpriseIdSyncCallback(
    _Inout_opt_ void *context,
    _In_opt_z_ const LPCWSTR wszEnterpriseId,
    _In_ DWORD dwErr
    )
{
    size_t copyLength = 0;
    if (context == NULL)
    {
        return;
    }

    if (wszEnterpriseId == NULL)
    {
        return;
    }

    *((LPWSTR *)context) = NULL;
    copyLength = wcslen(wszEnterpriseId) + 1;
    (*(LPWSTR *)context) = new wchar_t[copyLength];

    dwErr = wcscpy_s((*(LPWSTR *)context), copyLength, wszEnterpriseId);
    if (dwErr != NO_ERROR)
    {
        printf("ERROR occured in callback %d - %X\n", dwErr, dwErr);
    }
}

DWORD
FwDiagnoseConnectTarget(
    _In_ LPWSTR wszTarget,
    _In_ DWORD  dwFlags
    )
{
    DWORD dwErr               = NO_ERROR;
    LPWSTR wszEnterpriseId    = NULL;
    HANDLE hOperation         = NULL;
    HMODULE hModule           = NULL;
    PNetworkIsolationGetEnterpriseIdAsync pFuncNetworkIsolationGetEnterpriseIdAsync = NULL;
    PNetworkIsolationGetEnterpriseIdClose pFuncNetworkIsolationGetEnterpriseIdClose = NULL;
    
    hModule = LoadLibrary(L"firewallapi.dll");
    if (hModule != NULL)
    {
        pFuncNetworkIsolationGetEnterpriseIdAsync = (PNetworkIsolationGetEnterpriseIdAsync) GetProcAddress(
            hModule,
            "NetworkIsolationGetEnterpriseIdAsync"
        );
        pFuncNetworkIsolationGetEnterpriseIdClose = (PNetworkIsolationGetEnterpriseIdClose) GetProcAddress(
            hModule,
            "NetworkIsolationGetEnterpriseIdClose"
        );
    }

    if (pFuncNetworkIsolationGetEnterpriseIdAsync != NULL &&
        pFuncNetworkIsolationGetEnterpriseIdClose != NULL)
    {
        dwErr = pFuncNetworkIsolationGetEnterpriseIdAsync(
                wszTarget,
                dwFlags,
                &wszEnterpriseId,
                (PNETISO_EDP_ID_CALLBACK_FN)
                    NetworkIsolationGetEnterpriseIdSyncCallback,
                &hOperation
                );
        FAIL_IF_DWERR(dwErr);

        dwErr = pFuncNetworkIsolationGetEnterpriseIdClose(
                hOperation,
                TRUE
                );
        FAIL_IF_DWERR(dwErr);
    }

    if (wszEnterpriseId != NULL)
    {
        wprintf(L"\
\nThe target: %s\n Enterprise state: EnterpriseId=%s\n", wszTarget, wszEnterpriseId );
    }
    else
    {
        wprintf(L"\
\nThe target %s\n Enterprise state: PERSONAL \n", wszTarget );
    }

CLEAN:
    if (hModule != NULL)
    {
        FreeLibrary(hModule);
    }
    if (dwErr != NO_ERROR)
    {
        wprintf(L"\
\nERROR %s couldn't be diagnosed\n", wszTarget );
    }
    return dwErr;
}



//+----------------------------------------------------
//
// Main Function of our tool.
// This function parses the input, preparse the data, and performs the 
// instructed management operation on the specified target.
//
//+---------------------------------------------------

int __cdecl wmain(int argc, wchar_t **argv)
{
    DWORD dwStatus = NO_ERROR;
    OP_TYPE OpType = OP_TYPE_INVALID;
    WCHAR *wszTarget = NULL;
    DWORD dwFlags = 0;
    DWORD i = 0;

    if (lstrcmpiW( L"/?", argv[ARG_TYPE_HELP]) == 0)
    {
        wprintf(L"\
\nUsage:\
\n   EnterpriseStateClassify <OPERATION> <FLAGS> <TARGET>\
\n      <OPERATION> .- Connect/ConnectLoop/ConnectLoopFast \
\n      <FLAGS> .-  flags to pass \
\n      <TARGET> .- Target to verify as a http proxy\n" );

        return 0;
    }

    if (argc < ARG_TYPE_MAX-1)
    {
        wprintf(L"\
\nUsage:\
\n   EnterpriseStateClassify <OPERATION> <FLAGS> <TARGET>\
\n      <OPERATION> .- Connect/ConnectLoop/ConnectLoopFast \
\n      <FLAGS> .-  flags to pass \
\n      <TARGET> .- Target to verify as a http proxy\n" );
        dwStatus = ERROR_INVALID_PARAMETER;
    }
    FAIL_IF_DWERR(dwStatus);

    if (lstrcmpiW( L"Connect", argv[ARG_TYPE_OPERATION] ) == 0)
    {
        OpType = OP_TYPE_CONNECT;
    }
    else if (lstrcmpiW( L"ConnectLoop", argv[ARG_TYPE_OPERATION] ) == 0)
    {
        OpType = OP_TYPE_CONNECTLOOP;
    }
    else if (lstrcmpiW(L"ConnectLoopFast", argv[ARG_TYPE_OPERATION]) == 0)
    {
        OpType = OP_TYPE_CONNECTLOOPFAST;
    }
    else
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        FAIL_IF_DWERR(dwStatus);
    }

    //
    // Read the flags
    //
    swscanf_s(argv[ARG_TYPE_FLAGS], L"%d", &dwFlags);

    //
    // Prepare data for doing the work
    //
    if (OpType == OP_TYPE_CONNECT ||
        OpType == OP_TYPE_CONNECTLOOP ||
        OpType == OP_TYPE_CONNECTLOOPFAST)
    {
        wszTarget = argv[ARG_TYPE_TARGET];
    }

    //
    // Perform the requested work
    //
    if (OpType == OP_TYPE_CONNECT)
    {
        dwStatus = FwDiagnoseConnectTarget(wszTarget, dwFlags);
        FAIL_IF_DWERR(dwStatus);
    }
    else if (OpType == OP_TYPE_CONNECTLOOP ||
             OpType == OP_TYPE_CONNECTLOOPFAST)
    {
        for (i = 0; i < 20; i++)
        {
            dwStatus = FwDiagnoseConnectTarget(wszTarget, dwFlags);
            if (OpType == OP_TYPE_CONNECTLOOP)
            {
                Sleep(1500);
            }
        }
    }

CLEAN:

    //
    // Handle Errors
    //
    if (dwStatus != NO_ERROR)
    {
        printf("ERROR occured %d - %X\n", dwStatus, dwStatus);
    }

    return 0;
}
