#pragma once
#include <qtextbrowser.h>
#include <qmessagebox.h>
#include <Windows.h>

#define SCRIPTS_PATH "./AvZ/"

namespace global {
extern QTextBrowser *log_browser;
void ErrorBox(QWidget* widget, const QString& str);
void Utf8ToGbk(std::string& strUTF8);
}