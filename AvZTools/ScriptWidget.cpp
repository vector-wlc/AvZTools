#include "ScriptWidget.h"
#include "Injector.h"
#include <Windows.h>

bool ScriptWidget::is_have_error_window = true;
bool ScriptWidget::is_have_delete_window = true;

ScriptWidget::ScriptWidget(QString label)
{
    script_name = new QLabel(label);
    run_btn = new QPushButton("运行");
    run_btn->hide();
    edit_script_btn = new QPushButton("编辑");
    edit_script_btn->hide();
    delete_btn = new QPushButton("删除");
    delete_btn->hide();

    creatUi();

    connect(delete_btn, &QPushButton::released, [=]() {
        bool is_delete = true;
        if (is_have_delete_window) {
            auto is_yes = QMessageBox::information(this,
                "删除脚本",
                QString("您是否要删除脚本 : ") + script_name->text() + " ?",
                QMessageBox::Yes | QMessageBox::No);
            is_delete = (is_yes == QMessageBox::Yes);
        }

        if (is_delete) {
            QFile file(QString(SCRIPTS_PATH) + script_name->text());
            file.remove();
            file.close();
        }

        emit deleteScript(is_delete);
    });

    connect(edit_script_btn, &QPushButton::released, [=]() {
        emit editScript(script_name->text());
    });

    connect(run_btn, &QPushButton::released, [=]() {
        emit scriptCompiling();
        compileScript();
        injectScript();
        emit scriptCompiled();
    });
}

ScriptWidget::~ScriptWidget()
{
    delete run_btn;
    delete edit_script_btn;
    delete delete_btn;
    delete script_name;
}

void ScriptWidget::compileScript()
{
    global::log_browser->clear();
    global::log_browser->insertPlainText(script_name->text() + " 正在编译中...\n");
    // 准备 g++ 命令
    QString cmd;
    cmd += QString("/c ")
        + "\""
        + QDir::currentPath()
        + "/MinGW/bin/g++.exe"
        + "\""
        + " AvZ/"
        + script_name->text()
        + " -std=c++1z"
        + " -I"
        + " AvZ/inc"
        + " -l"
        + " avz"
        + " -L"
        + " AvZ/bin"
        + " -shared"
        + " -o"
        + OUTPUT_AVZDLL_PATH
        + " 2>log.txt";

    auto cmd_std = cmd.toStdString();
    global::Utf8ToGbk(cmd_std);

    // 使用 ShellExecuteEx 阻塞调用 g++
    // 原因：QProcess 启动 g++ 一直有问题
    // 因此使用隐藏的 cmd.exe 调用 g++
    SHELLEXECUTEINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SEE_MASK_NOCLOSEPROCESS;
    si.lpVerb = "open";
    si.lpFile = "cmd.exe";
    si.lpParameters = cmd_std.c_str();
    si.nShow = SW_HIDE;

    ShellExecuteExA(&si);

    DWORD dwExitCode;
    GetExitCodeProcess(si.hProcess, &dwExitCode);
    while (dwExitCode == STILL_ACTIVE) {
        Sleep((DWORD)5);
        GetExitCodeProcess(si.hProcess, &dwExitCode);
    }

    CloseHandle(si.hProcess);
}

void ScriptWidget::injectScript()
{
    QFile file("./log.txt");
    if (file.open(QIODevice::ReadWrite)) {
        if (file.atEnd()) {
            global::log_browser->insertPlainText(script_name->text() + " 编译成功\n");
            Injector injector(this);
            if (injector.openByWindow()) {
                injector.manageDLL();
            }
        } else {
            QTextStream text_stream(&file);
            text_stream.setCodec("gbk");
            global::log_browser->insertPlainText(text_stream.readAll());
            global::log_browser->insertPlainText(script_name->text() + " 编译失败\n");
            if (is_have_error_window) {
                global::ErrorBox(this, "当前脚本 " + script_name->text() + " 中有语法错误，请查看错误提示信息");
            }
        }
    }
}

void ScriptWidget::creatUi()
{
    setFixedHeight(SCRIPT_WIDGET_HEIGHT);
    setFixedWidth(SCRIPT_WIDGET_WIDTH);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, qRgb(65, 205, 82)); //设置背景绿色
    setAutoFillBackground(true);
    setPalette(pal);

    auto h_layout = new QHBoxLayout;
    h_layout->addStretch();
    h_layout->addWidget(script_name);
    h_layout->addStretch();
    h_layout->addWidget(run_btn);
    h_layout->addStretch();
    h_layout->addWidget(edit_script_btn);
    h_layout->addStretch();
    h_layout->addWidget(delete_btn);

    setLayout(h_layout);
}

void ScriptWidget::enterEvent(QEvent* e)
{
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, qRgb(30, 160, 47)); //设置背景绿色
    setAutoFillBackground(true);
    setPalette(pal);
    run_btn->show();
    delete_btn->show();
    edit_script_btn->show();
}

void ScriptWidget::leaveEvent(QEvent* e)
{
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, qRgb(65, 205, 82)); //设置背景绿色
    setAutoFillBackground(true);
    setPalette(pal);
    run_btn->hide();
    delete_btn->hide();
    edit_script_btn->hide();
}