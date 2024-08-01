#include <stdio.h>
#include <Windows.h>
#include <psapi.h>
#include <string.h>


int main (void){
    int id = GetProcIdByName("Spotify.exe");
    return 0;
}

void GetExecutableName(const char* fullPath, char* exeName, size_t exeNameSize) {
    const char* lastSlash = strrchr(fullPath, '\\');
    if (lastSlash != NULL) {
        strncpy(exeName, lastSlash + 1, exeNameSize - 1);
    } else {
        strncpy(exeName, fullPath, exeNameSize - 1);
    }
    exeName[exeNameSize - 1] = '\0';
}


int GetProcIdByName(const char name[]){           

    DWORD aProcesses[1024], cbNeeded, cProcesses;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        return 0;
    }

    cProcesses = cbNeeded / sizeof(DWORD);

    for (int i = 0; i < cProcesses; i++)
    {
        HANDLE handle = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
            FALSE,
            aProcesses[i]
        );

        if (!handle){
            continue;
        }


        char path[MAX_PATH];
        DWORD pathLen = MAX_PATH;

        if (QueryFullProcessImageNameA(handle, 0, path, &pathLen)) {
            CloseHandle(handle);

            char exeName[MAX_PATH];
            GetExecutableName(path, exeName, sizeof(exeName));

            if (strcmp(name, exeName) == 0) {
                printf("Process found: %s (PID: %d)\n", name, aProcesses[i]);
                return aProcesses[i];
            }
        }
    }

    return 0;
}