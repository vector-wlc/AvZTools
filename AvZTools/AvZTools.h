#pragma once
#pragma execution_character_set("utf-8")

#include <QtWidgets/QMainWindow>
#include <Windows.h>
#include <list>
#include <qdir.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qpushbutton.h>
#include <qscrollarea.h>
#include <qstatusbar.h>
#include <qtextbrowser.h>
#include <qtextstream.h>
#include <stdint.h>
#include <qmenubar.h>
#include <qdesktopservices.h>
#include <qsettings.h>

#include "global.h"
#include "NewScript.h"
#include "Process.h"
#include "ScriptWidget.h"

#define EDITOR_PATH "./configure/editor_path.txt"
#define SETTINGS_PATH "./configure/settings.txt"

class AvZTools : public QMainWindow {
    Q_OBJECT

public:
    AvZTools(QWidget* parent = Q_NULLPTR);
    ~AvZTools();

private:
    std::list<ScriptWidget*> script_list;
    QVBoxLayout* script_v_layout;
    QWidget* script_widget;
    QLabel* editor_label;
    QLineEdit* editor_line;
    QPushButton* choose_editor_btn;
    QAction* delete_script_action;
    QAction* error_script_action;
    QAction* auto_open_vscode_action;

    // 创建整体Ui
    void creatUi();

    // 创建菜单
    void creatMenu();

    // 更新脚本列表
    void updateScriptList();

    // 保存编辑器路径
    void saveEditorPath();

    // 载入设置
    void loadSettings();

    // 保存设置
    void saveSettings();

    // 初始化编辑器路径
    QString initEditorPath();
};
