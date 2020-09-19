#pragma once
#pragma execution_character_set("utf-8")

#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qmessagebox.h>

#include "global.h"

class NewScript : public QDialog {
    Q_OBJECT
private:
    QLabel label;
    QLineEdit line_edit;
    QVBoxLayout v_layout;
    QPushButton btn;
    QHBoxLayout h_layout;

public:
    NewScript(QWidget* parent);

signals:
    void newScript(bool is_new);
};
