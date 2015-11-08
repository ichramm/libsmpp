// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"


// Make it global, so it can be used somewhere else
DWORD dwTlsIndex = 0; // address of shared memory
DWORD dwTlsIndexDumpBuffer = 0;
DWORD dwTlsIndexIconvBuffer = 0;

extern void smpp_logger_load();
extern void smpp_logger_unload();


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	LPVOID lpvData;
	DWORD *tlsIndexes[] = { &dwTlsIndex, &dwTlsIndexDumpBuffer, &dwTlsIndexIconvBuffer, NULL };

	switch (ul_reason_for_call)
	{
	// The DLL is loading due to process
	// initialization or a call to LoadLibrary.
	case DLL_PROCESS_ATTACH:

		for ( int i = 0; tlsIndexes[i]; i++)
		{
			// Allocate a TLS index.
			if (( *tlsIndexes[i] = TlsAlloc()) == TLS_OUT_OF_INDEXES)
			{
				for (int j = i-1; j >= 0; j--)
				{
					TlsFree(*tlsIndexes[j]);
				}

				return FALSE;
			}
		}

		// Initialize logger
		smpp_logger_load();

		break;

	// The attached process creates a new thread.
	case DLL_THREAD_ATTACH:

		// TLS index for each thread is initialized as
		// needed in smpp34.h

		break;

	// The thread of the attached process terminates.
	case DLL_THREAD_DETACH:

		// Release the allocated memory for this thread.
		for ( int i = 0; tlsIndexes[i]; i++)
		{
			lpvData = TlsGetValue(*tlsIndexes[i]);
			if (lpvData != NULL)
			{
				LocalFree((HLOCAL) lpvData);
			}
		}

		break;

	// DLL unload due to process termination or FreeLibrary.
	case DLL_PROCESS_DETACH:

		for ( int i = 0; tlsIndexes[i]; i++)
		{
			// Release the allocated memory for this thread.
			lpvData = TlsGetValue(*tlsIndexes[i]);
			if (lpvData != NULL)
			{
				LocalFree((HLOCAL) lpvData);
			}

			// Release the TLS index.
			TlsFree(*tlsIndexes[i]);
		}

		// Release logger
		smpp_logger_unload();

		break;

	default:
		break;
	}

	return TRUE;
}

