#include "session.h"
#include "_cgo_export.h"

ULONG CreateSession(TRACEHANDLE* hSession, char* sessionName) {
    PEVENT_TRACE_PROPERTIES pSessionProperties = NULL;
    HANDLE hToken = NULL;
    HANDLE hProcess = NULL;

    hProcess = GetCurrentProcess();
    if (OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken) == FALSE)  {
        printf("Error: Couldn't open the process token\n");
        return 132;
    }

    const size_t buffSize = sizeof(EVENT_TRACE_PROPERTIES) + strlen(sessionName) + 1;
    pSessionProperties = malloc(buffSize);
    ZeroMemory(pSessionProperties, buffSize);
    pSessionProperties->Wnode.BufferSize = buffSize;
    pSessionProperties->Wnode.ClientContext = 1;
    pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    pSessionProperties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    pSessionProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

    return StartTrace(hSession, sessionName, pSessionProperties);
};

ULONG StartSession(char* sessionName, PVOID context) {
    ULONG status = ERROR_SUCCESS;
    EVENT_TRACE_LOGFILE trace;
    TRACEHANDLE hTrace = 0;

    ZeroMemory(&trace, sizeof(EVENT_TRACE_LOGFILE));
    trace.LogFileName = NULL;
    trace.LoggerName = sessionName;
    trace.CurrentTime = 0;
    trace.BuffersRead = 0;
    trace.BufferSize = 0;
    trace.Filled = 0;
    trace.EventsLost = 0;
    trace.Context = context;
    trace.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
    trace.EventRecordCallback = (PEVENT_RECORD_CALLBACK)(handleEvent);

    hTrace = OpenTrace(&trace);

    if (INVALID_PROCESSTRACE_HANDLE == hTrace) {
        return GetLastError();
    }

    status = ProcessTrace(&hTrace, 1, 0, 0);
    if (status != ERROR_SUCCESS && status != ERROR_CANCELLED) {
        return status;
    }
}

ULONGLONG GetPropertyName(PTRACE_EVENT_INFO info , int i) {
    return (ULONGLONG)((PBYTE)(info) + info->EventPropertyInfoArray[i].NameOffset);
}

ULONG GetPropertyCount(PTRACE_EVENT_INFO info, int i) {
    return info->EventPropertyInfoArray[i].count;
}

USHORT GetInType(PTRACE_EVENT_INFO info, int i) {
    return info->EventPropertyInfoArray[i].nonStructType.InType;
}

USHORT GetOutType(PTRACE_EVENT_INFO info, int i) {
    return info->EventPropertyInfoArray[i].nonStructType.OutType;
}


ULONG GetMapName(PTRACE_EVENT_INFO info, int i) {
    return (ULONGLONG)((PBYTE)(info) + info->EventPropertyInfoArray[i].nonStructType.MapNameOffset);
}
