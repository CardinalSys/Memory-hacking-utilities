#include <stdio.h>
#include <Windows.h>
#include <psapi.h>
#include <string.h>

struct Process {
    int pid;
    HANDLE handle;
    HMODULE hMods[1024];
    TCHAR szModName[MAX_PATH];
};

void GetExecutableName(const char* fullPath, char* exeName, size_t exeNameSize) {
    const char* lastSlash = strrchr(fullPath, '\\');
    if (lastSlash != NULL) {
        strncpy(exeName, lastSlash + 1, exeNameSize - 1);
    } else {
        strncpy(exeName, fullPath, exeNameSize - 1);
    }
    exeName[exeNameSize - 1] = '\0';
}

struct Process GetProcessByName(const char name[]) {
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    struct Process proc = {0, NULL, {0}, TEXT("")};

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return proc;
    }

    cProcesses = cbNeeded / sizeof(DWORD);

    for (DWORD i = 0; i < cProcesses; i++) {
        HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);

        if (!handle) {
            continue;
        }

        char path[MAX_PATH];
        DWORD pathLen = MAX_PATH;

        if (QueryFullProcessImageNameA(handle, 0, path, &pathLen)) {
            char exeName[MAX_PATH];
            GetExecutableName(path, exeName, sizeof(exeName));

            if (strcmp(name, exeName) == 0) {
                printf("Process found: %s (PID: %d)\n", name, aProcesses[i]);

                if (EnumProcessModules(handle, proc.hMods, sizeof(proc.hMods), &cbNeeded)) {
                    for (DWORD j = 0; j < (cbNeeded / sizeof(HMODULE)); j++) {
                        if (GetModuleFileNameEx(handle, proc.hMods[j], proc.szModName, sizeof(proc.szModName) / sizeof(TCHAR))) {
                            printf(TEXT("\t%s (0x%p)\n"), proc.szModName, (void*)proc.hMods[j]);
                        }
                    }
                }
                
                proc.pid = aProcesses[i];
                proc.handle = handle;

                return proc;
            }
        }
        CloseHandle(handle);
    }
    return proc;
}

int main(void) {
    struct Process proc = GetProcessByName("Spotify.exe");

    if (proc.pid != 0) {
        CloseHandle(proc.handle);
    } else {
        printf("Process not found.\n");
    }
    return 0;
}
