#include "AvZTools.h"
#include <QtWidgets/QApplication>
#include <qsharedmemory.h>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    AvZTools w;
    QSharedMemory shared("AvZTools");
    if (shared.attach()) //共享内存被占用则直接返回
    {
        QMessageBox::information(&w, "Warning", "AvZ Tools 已经在运行中");
        return 0;
    }
    w.show();
    shared.create(1); //共享内存没有被占用则创建UI
    
    return a.exec();
}
