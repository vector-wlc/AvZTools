#include "NewScript.h"

NewScript::NewScript(QWidget* parent)
    :QDialog(parent)
{
    setWindowTitle("新建脚本");
    setModal(true);
    setFont(QFont("Microsoft YaHei"));
    label.setText("请输入脚本文件名称");
    btn.setText("确认");
    v_layout.addWidget(&label);
    v_layout.addWidget(&line_edit);
    h_layout.addStretch();
    h_layout.addWidget(&btn);
    h_layout.addStretch();
    v_layout.addLayout(&h_layout);

    setLayout(&v_layout);

    connect(&btn, &QPushButton::released, [=]() {
        if (line_edit.text().isEmpty()) {
            emit newScript(false);
            return;
        }
        // 截断 / 之前的内容
        auto name = line_edit.text().section('/', -1);
        name = QString(SCRIPTS_PATH) + name;
        if (name.lastIndexOf(".cpp") == -1) {
            name += ".cpp";
        }

        // 创建脚本
        QFile file(name);
        if (file.exists()) {
            QMessageBox::information(this, "Error", name.section('/', -1) + "已存在，创建脚本失败");
            file.close();
            return;
        }

        // 增添默认内容
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        file.write("#include \"avz.h\"\nvoid Script() \n{\n\n}");
        file.close();
        emit newScript(true);
    });
} 

