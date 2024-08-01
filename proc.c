#include <stdio.h>
#include <Windows.h>
#include <psapi.h>

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
        HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, aProcesses[i]);

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


BOOL DataCompare(const BYTE* data, const BYTE* pattern, const char* mask) {
    for (; *mask; ++mask, ++data, ++pattern) {
        if (*mask == 'x' && *data != *pattern) {
            return FALSE;
        }
    }
    return (*mask) == NULL;
}


DWORD_PTR FindPattern(HANDLE hProcess, BYTE* pattern, char* mask, DWORD_PTR start, DWORD_PTR end) {
    SIZE_T bytesRead;
    BYTE buffer[4096];

    for (DWORD_PTR i = start; i < end; i += sizeof(buffer)) {
        ReadProcessMemory(hProcess, (LPCVOID)i, buffer, sizeof(buffer), &bytesRead);
        for (size_t j = 0; j < bytesRead; j++) {
            if (DataCompare(buffer + j, pattern, mask)) {
                return i + j;
            }
        }
    }
    return 0;
}

int main(void) {
    struct Process proc = GetProcessByName("Mesen.exe");

    if (proc.pid != 0) {

        //Read process memory example
        byte value;
        SIZE_T bytesRead;
        if (ReadProcessMemory(proc.handle, (LPCVOID)0x2725BE8D1E0, &value, sizeof(value), &bytesRead)) {
            printf("Value read from address 0x2725BE8D1E0: %d\n", value);
        } else {
            printf("Error reading address: %lu\n", GetLastError());
        }

        //Write process memory example
        byte newValue = 10;
        SIZE_T bytesWrite;
        if (WriteProcessMemory(proc.handle, (LPCVOID)0x2725BE8D1E0, &newValue, sizeof(newValue), &bytesWrite)){
            printf("Value writed from address 0x2725BE8D1E0: %d\n", newValue);
        }else{
            printf("Error writing address: %lu\n", GetLastError());
        }


        //AoB scan example
        BYTE pattern[] = {0x1F, 0x14, 0x05, 0x56, 0x30, 0x68, 0xAC, 0x44, 0xAC, 0x44, 0xAC, 0x44};
        char mask[] = "xx?x";

        DWORD_PTR start = proc.hMods[0];
        DWORD_PTR end = proc.hMods[1];

        DWORD_PTR address = FindPattern(proc.handle, pattern, mask, start, end);
        if (address) {
            printf("Pattern found at address: 0x%p\n", (void*)address);
        } else {
            printf("Pattern not found.\n");
        }

        CloseHandle(proc.handle);
    } else {
        printf("Process not found.\n");
    }
    return 0;
}
