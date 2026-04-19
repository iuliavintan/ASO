#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "psapi.lib")

typedef struct {
    WCHAR name[256];
    WCHAR displayName[256];
    DWORD pid;
    DWORD state;
} ServiceInfo;

typedef struct {
    WCHAR dllName[MAX_PATH];
    DWORD pids[512];
    int pidCount;
} DllEntry;

int getServices(SC_HANDLE scm, ServiceInfo** out)
{
    DWORD needed = 0, count = 0, resume = 0;
    *out = NULL;

    EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &needed, &count, &resume, NULL);

    BYTE* buf = (BYTE*)malloc(needed);
    if (!buf) return -1;

    resume = 0;
    if (!EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL, buf, needed, &needed, &count, &resume, NULL)) {
        free(buf);
        return -1;
    }

    LPENUM_SERVICE_STATUS_PROCESSW entries = (LPENUM_SERVICE_STATUS_PROCESSW)(void*)buf;
    ServiceInfo* arr = (ServiceInfo*)malloc(count * sizeof(ServiceInfo));
    if (!arr) { 
        free(buf); 
        return -1; 
    }

    for (DWORD i = 0; i < count; i++) {
        wcsncpy_s(arr[i].name, 256, entries[i].lpServiceName, _TRUNCATE);
        wcsncpy_s(arr[i].displayName, 256, entries[i].lpDisplayName, _TRUNCATE);
        arr[i].pid = entries[i].ServiceStatusProcess.dwProcessId;
        arr[i].state = entries[i].ServiceStatusProcess.dwCurrentState;
    }

    free(buf);
    *out = arr;
    return (int)count;
}

void afisareServicii(ServiceInfo* services, int count)
{
    int idx = 1;
    for (int i = 0; i < count; i++) {
        if (services[i].state != SERVICE_RUNNING) continue;
        wprintf(L"%d. %ls [%ls] pid=%lu\n",
            idx++,
            services[i].displayName,
            services[i].name,
            (unsigned long)services[i].pid);
    }
}

void afisareDLLuri(ServiceInfo* services, int count)
{
    int tableSize = 0;
    DllEntry* table = (DllEntry*)calloc(2048, sizeof(DllEntry));
    if (!table) return;

    for (int i = 0; i < count; i++) {
        if (services[i].state != SERVICE_RUNNING || services[i].pid == 0)
            continue;

        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, services[i].pid);
        if (!hProc) continue;

        HMODULE mods[512];
        DWORD cbNeeded = 0;

        if (EnumProcessModules(hProc, mods, sizeof(mods), &cbNeeded)) {
            DWORD modCount = cbNeeded / sizeof(HMODULE);
            for (DWORD m = 0; m < modCount; m++) {
                WCHAR path[MAX_PATH] = { 0 };
                if (!GetModuleFileNameExW(hProc, mods[m], path, MAX_PATH))
                    continue;

                WCHAR* base = wcsrchr(path, L'\\');
                WCHAR* dllName = base ? base + 1 : path;

                int found = -1;
                for (int j = 0; j < tableSize; j++) {
                    if (_wcsicmp(table[j].dllName, dllName) == 0) {
                        found = j;
                        break;
                    }
                }

                if (found == -1) {
                    wcsncpy_s(table[tableSize].dllName, MAX_PATH, dllName, _TRUNCATE);
                    found = tableSize++;
                }

                int alreadyAdded = 0;
                for (int p = 0; p < table[found].pidCount; p++)
                    if (table[found].pids[p] == services[i].pid) { alreadyAdded = 1; break; }

                if (!alreadyAdded && table[found].pidCount < 512)
                    table[found].pids[table[found].pidCount++] = services[i].pid;
            }
        }
        CloseHandle(hProc);
    }

    for (int i = 0; i < tableSize; i++) {
        if (table[i].pidCount < 2) continue;
        wprintf(L"%ls -> %d procese: ", table[i].dllName, table[i].pidCount);
        for (int p = 0; p < table[i].pidCount; p++) {
            if (p) printf(", ");
            printf("%lu", (unsigned long)table[i].pids[p]);
        }
        printf("\n");
    }

    free(table);
}

int main(void)
{
    SetConsoleOutputCP(CP_UTF8);

    SC_HANDLE scm = OpenSCManagerW(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm) {
        printf("Eroare la deschiderea Service Manager (rulati ca admin)\n");
        return 1;
    }

    ServiceInfo* services = NULL;
    int count = getServices(scm, &services);
    CloseServiceHandle(scm);

    if (count <= 0) {
        printf("Nu s-au putut enumera serviciile\n");
        return 1;
    }

    printf("=== Servicii care ruleaza ===\n");
    afisareServicii(services, count);

    printf("\n=== DLL-uri incarcate de mai multe servicii ===\n");
    afisareDLLuri(services, count);

    free(services);
    system("pause");
    return 0;
}