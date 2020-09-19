#pragma once
#pragma execution_character_set("utf-8")

#include <qdir.h>
#include <qevent.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qtextstream.h>

#include "Injector.h"

#define SCRIPT_WIDGET_HEIGHT 50
#define SCRIPT_WIDGET_WIDTH 300

class ScriptWidget : public QWidget {
    Q_OBJECT

private:
    static bool is_have_error_window;
    static bool is_have_delete_window;
    QPushButton* run_btn;
    QPushButton* edit_script_btn;
    QPushButton* delete_btn;
    QLabel* script_name;
   
    void compileScript();
    void injectScript();
    void creatUi();

signals:
    void deleteScript(bool is_delete);
    void scriptCompiling();
    void scriptCompiled();
    void editScript(QString name);

public:
    static void setIsHaveErrorWindow(bool _is_have_error_window) {
        is_have_error_window = _is_have_error_window;
    }
    static void setIsHaveDeleteWindow(bool _is_have_delete_window) {
        is_have_delete_window = _is_have_delete_window;
    }
    ScriptWidget(QString label);
    void setRunDisabled(bool is_disabled)
    {
        run_btn->setDisabled(is_disabled);
    }

    ~ScriptWidget();

protected:
    void enterEvent(QEvent* e);

    void leaveEvent(QEvent* e);
};
