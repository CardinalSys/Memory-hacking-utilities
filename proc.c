#include <stdio.h>
#include <Windows.h>
#include <psapi.h>

void GetExecutableName(const char* fullPath, char* exeName, size_t exeNameSize) {
    const char* lastSlash = strrchr(fullPath, '\\');
    if (lastSlash != NULL) {
        strncpy(exeName, lastSlash + 1, exeNameSize - 1);
    } else {
        strncpy(exeName, fullPath, exeNameSize - 1);
    }
    exeName[exeNameSize - 1] = '\0';
}



HANDLE OpenProcessByName(const char *name, char* outPath) {
    DWORD aProcesses[1024], cbNeeded, cProcesses;

    if(!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)){
        return -1;
    }   

    cProcesses = cbNeeded / sizeof(DWORD);

    for(DWORD i = 0 ; i < cProcesses; i++){
        HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);

        if (!handle) {
            continue;
        }
        char path[MAX_PATH];
        DWORD pathLen = MAX_PATH;

        if (QueryFullProcessImageNameA(handle, 0, path, &pathLen)) {
            strcpy(outPath, path);
            char exeName[MAX_PATH];
            GetExecutableName(path, exeName, sizeof(exeName));
            if(strcmp(exeName, name) == 0){
                return handle;
            }
        }

        CloseHandle(handle);

    }

    return -1;
}

uintptr_t GetModuleBaseAddress(const char* name, HANDLE handle){
    HMODULE hMods[1024];
    DWORD cbNeeded;


    if( EnumProcessModules(handle, hMods, sizeof(hMods), &cbNeeded))
    {
        for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
        {
            TCHAR szModName[MAX_PATH];

            if ( GetModuleFileNameEx( handle, hMods[i], szModName,
                                      sizeof(szModName) / sizeof(TCHAR)))
            {
                char exeName[MAX_PATH];
                GetExecutableName(szModName, exeName, sizeof(exeName));
                if(strcmp(name, exeName) == 0){
                    return (uintptr_t)hMods[i];
                }
            }
        }
    }

    return -1;
}


uintptr_t GetFinalAddress(HANDLE hProc, uintptr_t baseAddress, unsigned int* offsets, size_t offsetsSize) {
    uintptr_t addr = baseAddress;
    for (size_t i = 0; i < offsetsSize; i++) {
        if (!ReadProcessMemory(hProc, (LPCVOID)addr, &addr, sizeof(addr), NULL)) {
            return 0;
        }
        addr += offsets[i];
    }
    return addr;
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



int main(void){
    char path[MAX_PATH];
    HANDLE hProcess = OpenProcessByName("Mesen.exe", path);
    uintptr_t baseAddress = GetModuleBaseAddress("MesenCore.dll", hProcess) + 0x045ECED0;

    const int pattern[] = {0x18, 0x40, 0x28, 0x0};
    uintptr_t finalAddress = GetFinalAddress(hProcess, baseAddress, pattern, sizeof(pattern) / sizeof(pattern[0]));
    printf("Final address is: %llx\n", finalAddress);
    if(hProcess == NULL){
        printf("Process not found");
    }


}