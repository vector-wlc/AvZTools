#include "Injector.h"

void Injector::write(uintptr_t addr, size_t len, uint8_t* data)
{
    WriteProcessMemory(handle, (void*)addr, data, len, nullptr);
}

std::wstring Injector::strToWstr(const char* str)
{
    int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);

    MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, len);
    std::wstring result(wstr);

    if (wstr)
        delete[] wstr;

    return result;
}

bool Injector::isValid()
{
    if (handle == nullptr) {
        return false;
    }

    DWORD exit_code;
    GetExitCodeProcess(handle, &exit_code);
    bool valid = (exit_code == STILL_ACTIVE);

    return valid;
}

bool Injector::openByWindow()
{
    if (isValid()) {
        CloseHandle(handle);
    }
    hwnd = FindWindowW(L"MainWindow", L"Plants vs. Zombies");

    if (hwnd == nullptr) {
        global::ErrorBox(widget, "您是否未打开游戏? (注意必须是英文原版，steam 版也是不可以的！)");
        return false;
    }

    GetWindowThreadProcessId(hwnd, &pid);
    if (pid != 0) {
        handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    }

    if (ReadMemory<uint32_t>(0x4140c5) != 0x0019b337) {
        global::ErrorBox(widget, "您使用的游戏版本不是英文原版，请点击 帮助->获取 PvZ 英文原版 下载 AvZ 所支持的英文原版");
        return false;
    }

    auto address = ReadMemory<uintptr_t>(0x6a9ec0);
    auto game_ui = ReadMemory<int>(address + 0x7fc);
    while (game_ui == 2 || game_ui == 3) {
        global::ErrorBox(widget, "检测到游戏窗口在选卡或战斗界面，这种行为可能会导致注入失败，请在游戏主界面进行注入");
        game_ui = ReadMemory<int>(address + 0x7fc);
    }

    return hwnd != nullptr;
}

void Injector::manageDLL()
{
    ejectDLL();
    ejectDLL();

    // 删除上一次注入的 DLL
    DeleteFileW(INJECT_AVZDLL_PATH);

    global::log_browser->insertPlainText("正在复制脚本...\n");
    if (!CopyFileW(strToWstr(OUTPUT_AVZDLL_PATH).c_str(), INJECT_AVZDLL_PATH, false)) {
        global::ErrorBox(widget, "libavz.dll 复制失败，请检查根路径下是否有文件 libavz.dll");
        return;
    }
    global::log_browser->insertPlainText("操作成功\n");

    // 删除本次编译完成的 DLL
    DeleteFileA(OUTPUT_AVZDLL_PATH);

    global::log_browser->insertPlainText("正在将脚本注入到游戏中...\n");
    if (!injectDLL(INJECT_AVZDLL_PATH)) {
        global::ErrorBox(widget, "脚本注入失败，请重新运行尝试");
    } else {
        global::log_browser->insertPlainText("操作成功\n");
    }
}

DWORD Injector::ejectDLL()
{
    // Copy from Internet =_=
    WCHAR* szDllName = L"libavz.dll";
    BOOL bMore = FALSE, bFound = FALSE;
    HANDLE hSnapshot, hProcess, hThread;
    HMODULE hModule = NULL;
    MODULEENTRY32 me = { sizeof(me) };
    LPTHREAD_START_ROUTINE pThreadProc;
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    bMore = Module32First(hSnapshot, &me);
    for (; bMore; bMore = Module32Next(hSnapshot, &me)) {
        if (!lstrcmpW(me.szModule, szDllName) || !lstrcmpW(me.szExePath, szDllName)) {
            bFound = TRUE;
            break;
        }
    }
    if (!bFound) {
        CloseHandle(hSnapshot);
        return FALSE;
    }
    if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid))) {
        global::log_browser->insertPlainText(QString("OpenProcess(%1) failed!!! [%2]\n,").arg(pid).arg(GetLastError()));
        return FALSE;
    }
    hModule = GetModuleHandleA("Kernel32.dll");
    pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "FreeLibrary");
    hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, me.modBaseAddr, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    CloseHandle(hSnapshot);
    return TRUE;
}

DWORD Injector::injectDLL(LPCWSTR pszLibFile)
{
    // Calculate the number of bytes needed for the DLL's pathname
    DWORD dwSize = (lstrlenW(pszLibFile) + 1) * sizeof(wchar_t);
    if (handle == NULL) {
        wprintf(L"[-] Error: Could not open Process for PID (%d).\n", pid);
        return FALSE;
    }

    // Allocate space in the remote Process for the pathname
    LPVOID pszLibFileRemote = (PWSTR)VirtualAllocEx(handle, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
    if (pszLibFileRemote == NULL) {
        global::log_browser->insertPlainText(QString("[-] Error: Could not allocate memory inside PID (%1).\n").arg(pid));
        return FALSE;
    }

    // Copy the DLL's pathname to the remote Process address space
    DWORD n = WriteProcessMemory(handle, pszLibFileRemote, (LPCVOID)pszLibFile, dwSize, NULL);
    if (n == 0) {
        global::log_browser->insertPlainText(QString("[-] Error: Could not write any bytes into the PID [%1] address space.\n").arg(pid));
        return FALSE;
    }

    // Get the real address of LoadLibraryW in Kernel32.dll
    LPTHREAD_START_ROUTINE pfnThreadRtn = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
    if (pfnThreadRtn == NULL) {
        global::log_browser->insertPlainText(QString("[-] Error: Could not find LoadLibraryA function inside kernel32.dll library.\n"));
        return FALSE;
    }

    // Create a remote thread that calls LoadLibraryW(DLLPathname)
    HANDLE hThread = CreateRemoteThread(handle, NULL, 0, pfnThreadRtn, pszLibFileRemote, 0, NULL);
    if (hThread == NULL) {
        global::log_browser->insertPlainText(QString("[-] Error: Could not create the Remote Thread.\n"));
        return FALSE;
    }

    // Wait for the remote thread to terminate
    WaitForSingleObject(hThread, INFINITE);

    // Free the remote memory that contained the DLL's pathname and close Handles
    if (pszLibFileRemote != NULL)
        VirtualFreeEx(handle, pszLibFileRemote, 0, MEM_RELEASE);

    if (hThread != NULL)
        CloseHandle(hThread);

    return TRUE;
}