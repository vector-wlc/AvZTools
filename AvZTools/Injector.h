#pragma once
#pragma execution_character_set("utf-8")

#include <Windows.h>
#include <stdint.h>
#include <tlhelp32.h>
#include <qwidget.h>
#include "global.h"
#include <qdir.h>

#define OUTPUT_AVZDLL_PATH "AvZ/bin/libavz.dll"
#define INJECT_AVZDLL_PATH L"C:/ProgramData/libavz.dll"

// 脚本注入器
class Injector {

private:
    HWND hwnd = nullptr;
    DWORD pid = 0;
    HANDLE handle = nullptr;
    QWidget* widget;

public:
    Injector(QWidget* _widget)
        : widget(_widget)
    {
    }
    std::wstring strToWstr(const char* str);
    bool isValid();
    void write(uintptr_t addr, size_t len, uint8_t* data);
    bool openByWindow();
    void manageDLL();
    DWORD ejectDLL();
    
    DWORD injectDLL(LPCWSTR pszLibFile);
    template <typename T, typename... Args>
    T ReadMemory(Args... args)
    {
        std::initializer_list<uintptr_t> lst = { static_cast<uintptr_t>(args)... };
        uintptr_t buff = 0;
        T result = T();
        for (auto it = lst.begin(); it != lst.end(); ++it)
            if (it != lst.end() - 1)
                ReadProcessMemory(handle, (const void*)(buff + *it), &buff, sizeof(buff), nullptr);
            else
                ReadProcessMemory(handle, (const void*)(buff + *it), &result, sizeof(result), nullptr);
        return result;
    }
};
