#include "stdafx.h"
#include <filesystem>
#include <Windows.h>
#include <thread>
#include <vector>

void AdjustPrivilege(const std::wstring& privilegeName) {
    if (!AdjustCurrentPrivilege(privilegeName)) {
        exit(1);
    }
}

void ModifyRegistryKey(HKEY rootKey, const std::wstring& subKey, const std::wstring& valueName, const std::wstring& newValue) {
    HKEY key;
    if (RegOpenKeyEx(rootKey, subKey.c_str(), 0, KEY_WRITE, &key) == ERROR_SUCCESS) {
        RegSetValueEx(key, valueName.c_str(), 0, REG_SZ, (const BYTE*)newValue.c_str(), (newValue.length() + 1) * sizeof(wchar_t));
        RegCloseKey(key);
    }
}

void DeleteRegistryKey(HKEY rootKey, const std::wstring& subKey) {
    RegDeleteKey(rootKey, subKey.c_str());
}

void DeleteFileIfExists(const std::wstring& filePath) {
    DeleteFile(filePath.c_str());
}

void ForceDeleteFilesInDirectory(const std::wstring& directoryPath) {
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
        const std::wstring filePath = entry.path().wstring();
        DeleteFileIfExists(filePath);
    }
}

void SpoofUnique(HKEY key, const std::wstring& subKey, const std::wstring& valueName) {
    WCHAR spoof[MAX_PATH] = { 0 };
    wcscpy(spoof, L"spoofed_value");
    ModifyRegistryKey(key, subKey, valueName, spoof);
}

void CleanUpSystemFiles() {
    ForceDeleteFilesInDirectory(L"C:\\Windows\\Temp");
    ForceDeleteFilesInDirectory(L"C:\\Users\\Public\\Libraries");
}

void DeleteUnnecessaryRegistryKeys() {
    DeleteRegistryKey(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY");
    DeleteRegistryKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\BitBucket");
    DeleteRegistryKey(HKEY_LOCAL_MACHINE, L"SYSTEM\\MountedDevices");
    DeleteRegistryKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Dfrg\\Statistics");
    DeleteRegistryKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\NVIDIA Corporation\\Global");
    DeleteRegistryKey(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\mssmbios\\Data");
}

void PerformCleanUp() {
    AdjustPrivilege(SE_TAKE_OWNERSHIP_NAME);
    ModifyRegistryKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\BitBucket", L"LastEnum", L"new_enum_value");
    ModifyRegistryKey(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY", L"EDID", L"new_edid_value");
    SpoofUnique(HKEY_LOCAL_MACHINE, L"SOFTWARE\\NVIDIA Corporation\\Global", L"ClientUUID");
    CleanUpSystemFiles();
    DeleteUnnecessaryRegistryKeys();
}

int main() {
    srand(GetTickCount());
    LoadLibrary(L"ntdll.dll");

    std::vector<std::thread> threads;

    threads.push_back(std::thread(PerformCleanUp));

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
