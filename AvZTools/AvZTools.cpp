#include "AvZTools.h"
#include "qscrollarea.h"

AvZTools::AvZTools(QWidget *parent)
    : QMainWindow(parent)
{
    global::log_browser = new QTextBrowser;
    global::log_browser->setFont(QFont("Consolas"));

    editor_label = new QLabel("编辑器路径");
    editor_line = new QLineEdit;
    choose_editor_btn = new QPushButton("选择");
    creatMenu();
    creatUi();
    loadSettings();
    editor_line->setReadOnly(true);
    editor_line->setText(initEditorPath());

    ScriptWidget::setIsHaveDeleteWindow(delete_script_action->isChecked());
    ScriptWidget::setIsHaveErrorWindow(error_script_action->isChecked());

    connect(choose_editor_btn, &QPushButton::released, [=]() {
        auto editor_path = QFileDialog::getOpenFileName(this, "选择编辑器", "", "*.exe");

        if (editor_path.isEmpty())
        {
            global::ErrorBox(this, "指定编辑器路径出错");
        }
        else
        {
            editor_line->setText(editor_path);
            saveEditorPath();
        }
    });
}

AvZTools::~AvZTools()
{
    saveEditorPath();
    saveSettings();
    delete global::log_browser;
}

void AvZTools::updateScriptList()
{
    for (auto each : script_list)
    {
        disconnect(each);
        delete each;
    }

    script_list.clear();

    QDir dir(SCRIPTS_PATH);
    dir.setFilter(QDir::Files);

    QStringList filters;
    filters << "*.cpp";
    dir.setNameFilters(filters);
    QFileInfoList list = dir.entryInfoList();
    script_widget->resize(SCRIPT_WIDGET_WIDTH + 10, (SCRIPT_WIDGET_HEIGHT + 10) * list.size());
    script_widget->setFixedHeight((SCRIPT_WIDGET_HEIGHT + 10) * list.size());
    for (auto &each : list)
    {
        script_list.push_back(new ScriptWidget(each.fileName()));
    }

    for (auto each : script_list)
    {
        connect(each, &ScriptWidget::deleteScript, [=](bool is_delete) {
            if (is_delete)
            {
                updateScriptList();
            }
        });

        connect(each, &ScriptWidget::scriptCompiling, [=]() {
            for (auto each : script_list)
            {
                each->setRunDisabled(true);
            }
        });

        connect(each, &ScriptWidget::scriptCompiled, [=]() {
            for (auto each : script_list)
            {
                each->setRunDisabled(false);
            }
        });

        connect(each, &ScriptWidget::editScript, [=](QString name) {
            if (editor_line->text().lastIndexOf("Code.exe") != -1)
            { // VSCode 打开文件夹
                QProcess::startDetached(editor_line->text(), QString(SCRIPTS_PATH).split(" "));
            }
            else
            { // 其他打开单个文件
                QProcess::startDetached(editor_line->text(), QString(SCRIPTS_PATH + name).split(" "));
            }
        });
        script_v_layout->addWidget(each);
    }
}

void AvZTools::creatUi()
{
    resize(1000, 600);
    setWindowIcon(QIcon("iconengines/avz.ico"));
    setWindowTitle("AvZ Tools");
    setFont(QFont("Microsoft YaHei"));
    script_v_layout = new QVBoxLayout;
    script_widget = new QWidget;
    updateScriptList();
    script_widget->setLayout(script_v_layout);

    auto script_scroll = new QScrollArea;
    script_scroll->setWidget(script_widget);
    script_scroll->setMaximumWidth(SCRIPT_WIDGET_WIDTH + 20);

    auto h_editor_layout = new QHBoxLayout;
    editor_label->setFixedWidth(90);
    h_editor_layout->addWidget(editor_label);
    h_editor_layout->addWidget(editor_line);
    choose_editor_btn->setFixedWidth(100);
    h_editor_layout->addWidget(choose_editor_btn);

    auto v_layout = new QVBoxLayout;
    v_layout->addLayout(h_editor_layout);
    v_layout->addWidget(global::log_browser);

    auto h_layout = new QHBoxLayout;
    h_layout->addWidget(script_scroll);
    h_layout->addLayout(v_layout);

    auto temp = new QWidget;
    temp->setLayout(h_layout);
    setCentralWidget(temp);
}

void AvZTools::creatMenu()
{
    auto menu_file = menuBar()->addMenu("文件");
    auto new_script = menu_file->addAction("新建脚本");
    auto add_script = menu_file->addAction("添加脚本");
    auto update_script_lst = menu_file->addAction("更新脚本列表");

    auto menu_settings = menuBar()->addMenu("设置");
    delete_script_action = menu_settings->addAction("删除脚本提醒");
    delete_script_action->setCheckable(true);
    error_script_action = menu_settings->addAction("脚本出错提醒");
    error_script_action->setCheckable(true);
    auto_open_vscode_action = menu_settings->addAction("启动时自动打开 VSCode (此选项仅当编辑器为 VSCode 时生效)");
    auto_open_vscode_action->setCheckable(true);

    auto menu_help = menuBar()->addMenu("帮助");
    auto avz_qq_group = menu_help->addAction("AvZ QQ群");
    auto avz_tutor = menu_help->addAction("AvZ 图文教程");
    auto avz_update_tip = menu_help->addAction("AvZ 更新说明");
    auto avz_update = menu_help->addAction("AvZ 版本获取 密码：37zu");
    auto avz_locate = menu_help->addAction("AvZ 工作目录");
    auto avz_example = menu_help->addAction("AvZ 脚本示例");
    auto avz_github = menu_help->addAction("AvZ GitHub");
    menu_help->addSeparator();
    auto pvz_english_original = menu_help->addAction("获取 PvZ 英文原版");
    menu_help->addSeparator();
    auto about_me = menu_help->addAction("关于 AvZ Tools");
    auto about_qt = menu_help->addAction("关于 Qt");

    connect(new_script, &QAction::triggered, [=]() {
        NewScript *new_script = new NewScript(this);
        new_script->show();

        connect(new_script, &NewScript::newScript, [=](bool is_new) {
            disconnect(new_script);
            delete new_script;
            if (is_new)
            {
                updateScriptList();
            }
        });
    });

    connect(add_script, &QAction::triggered, [=]() {
        auto path = QFileDialog::getOpenFileName(this, "添加脚本", "", "*.cpp");
        if (!path.isEmpty())
        {
            if (QFile::copy(path, QString(SCRIPTS_PATH) + path.section('/', -1)))
            {
                updateScriptList();
            }
            else
            {
                global::ErrorBox(this, "添加脚本失败");
            }
        }
    });

    connect(update_script_lst, &QAction::triggered, [=]() { updateScriptList(); });

    connect(delete_script_action, &QAction::triggered, [=]() {
        ScriptWidget::setIsHaveDeleteWindow(delete_script_action->isChecked());
    });

    connect(error_script_action, &QAction::triggered, [=]() {
        ScriptWidget::setIsHaveErrorWindow(error_script_action->isChecked());
    });

    connect(avz_qq_group, &QAction::triggered, [=]() {
        QDesktopServices::openUrl(QUrl("https://jq.qq.com/?_wv=1027&k=vQXcVcLf"));
    });

    connect(avz_tutor, &QAction::triggered, [=]() {
        QDesktopServices::openUrl(QUrl("https://www.bilibili.com/read/readlist/rl289963"));
    });

    connect(avz_update_tip, &QAction::triggered, [=]() {
        QMessageBox::about(this, "AvZ 更新说明", "1.从 AvZ 版本获取处下载相应的版本号压缩文件并解压\n\n\
2.点击 AvZ 工作目录打开要替换的文件\n\n\
3.将下载的文件替换工作目录下的同名文件即可");
    });

    connect(avz_update, &QAction::triggered, [=]() {
        QDesktopServices::openUrl(QUrl("https://wwe.lanzous.com/b015az8yj"));
    });

    connect(avz_locate, &QAction::triggered, [=]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile("./AvZ"));
    });

    connect(avz_example, &QAction::triggered, [=]() {
        QDesktopServices::openUrl(QUrl("https://github.com/vector-wlc/AsmVsZombies/tree/master/script"));
    });

    connect(pvz_english_original, &QAction::triggered, [=]() {
        QDesktopServices::openUrl(QUrl("https://wwe.lanzous.com/iKMmtfhfz8f"));
    });

    connect(avz_github, &QAction::triggered, [=]() {
        QDesktopServices::openUrl(QUrl("https://github.com/vector-wlc/AsmVsZombies"));
    });

    connect(about_me, &QAction::triggered, [=]() {
        QMessageBox::about(this,
                           "关于 AvZ Tools",
                           "AvZ Tools 是为了简化使用 PvZ 键控框架 AsmVsZombies 而设计的软件\n\n\
构建工具：MSVC 2019 \n\
                   Qt 5.15.0 32bit\n\
作者： vector-wlc");
    });

    connect(about_qt, &QAction::triggered, [=]() {
        QMessageBox::aboutQt(this);
    });
}

void AvZTools::saveEditorPath()
{
    if (editor_line->text().isEmpty())
    {
        return;
    }
    QFile file(EDITOR_PATH);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file.write(editor_line->text().toStdString().c_str());
    }
    else
    {
        global::ErrorBox(this, "保存编辑器路径出错");
    }

    file.close();
}

QString AvZTools::initEditorPath()
{
    QString editor_path;
    QFile file(EDITOR_PATH);
    if (!file.exists())
    {
        QSettings settings("HKEY_CLASSES_ROOT\\vscode\\shell\\open\\command", QSettings::NativeFormat);
        editor_path = settings.value(".", "error").toString();

        if (editor_path == "error")
        {
            auto result = QMessageBox::information(this,
                                                   "获取 VSCode 失败",
                                                   "未能从你的计算机内找到 VSCode，你是否要安装 VSCode ?",
                                                   QMessageBox::Yes | QMessageBox::No);

            if (result == QMessageBox::Yes)
            {
                QDesktopServices::openUrl(QUrl("https://code.visualstudio.com/"));
            }
            else
            {
                editor_path = QFileDialog::getOpenFileName(this, "选择编辑器", "", "*.exe");

                if (editor_path.isEmpty())
                {
                    global::ErrorBox(this, "编辑器路径选择失败");
                }
            }
        }
        else
        {
            editor_path = *(editor_path.split("\"").begin() + 1);
        }
    }
    else
    {
        file.open(QIODevice::ReadOnly);
        editor_path = file.readAll();
    }

    file.close();

    if (editor_path.indexOf("Code.exe") != -1 && auto_open_vscode_action->isChecked())
    {
        QProcess::startDetached(editor_path, QString("./AvZ").split(" "));
    }
    return editor_path;
}

void AvZTools::loadSettings()
{
    delete_script_action->setChecked(true);
    error_script_action->setChecked(true);
    auto_open_vscode_action->setChecked(true);

    QString settings;
    QFile file(SETTINGS_PATH);
    if (file.exists())
    {
        if (file.open(QIODevice::ReadOnly))
        {
            QTextStream text_stream(&file);
            while (!text_stream.atEnd())
            {
                settings = text_stream.readLine();
                if (settings.indexOf("IsHaveDeleteWindow") != -1)
                {
                    bool is_checked = settings[settings.size() - 1].toLatin1() - '0';
                    delete_script_action->setChecked(is_checked);
                }
                else if (settings.indexOf("IsHaveErrorWindow") != -1)
                {
                    bool is_checked = settings[settings.size() - 1].toLatin1() - '0';
                    error_script_action->setChecked(is_checked);
                }
                else if (settings.indexOf("IsAutoOpenVSCode") != -1)
                {
                    bool is_checked = settings[settings.size() - 1].toLatin1() - '0';
                    auto_open_vscode_action->setChecked(is_checked);
                }
            }
        }
    }
    file.close();
    ScriptWidget::setIsHaveDeleteWindow(delete_script_action->isChecked());
    ScriptWidget::setIsHaveErrorWindow(error_script_action->isChecked());
}

void AvZTools::saveSettings()
{
    QFile file(SETTINGS_PATH);

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QString settings = QString("IsHaveDeleteWindow:%1\nIsHaveErrorWindow:%2\nIsAutoOpenVSCode:%3")
                               .arg(delete_script_action->isChecked())
                               .arg(error_script_action->isChecked())
                               .arg(auto_open_vscode_action->isChecked());
        file.write(settings.toStdString().c_str());
    }

    file.close();
}