/*
MIT License

Copyright (c) 2021 WagonWheelRobotics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "customFloatingWindow.h"
#include <QMenuBar>
#include <QCloseEvent>

#include <QDebug>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#define IDM_ATTACH 0x0110
#endif

customFloatingWindow::customFloatingWindow(QWidget *widget, QWidget *parent) : QMainWindow(parent)
{
    setWindowIcon(QIcon(":/icons/crl.png"));

    setCentralWidget(widget);
    widget->setProperty("frame", QVariant::fromValue(this));

#ifdef Q_OS_WINDOWS
    HMENU hMenu=::GetSystemMenu((HWND)winId(),FALSE);
    if (hMenu != NULL)
    {
        ::AppendMenuA(hMenu, MF_SEPARATOR, 0, 0);
        ::AppendMenuA(hMenu, MF_STRING, IDM_ATTACH, "Attach");
    }
#endif

    //qDebug()<<windowFlags();
}

customFloatingWindow::~customFloatingWindow()
{
    //qDebug()<<"customFloatingWindow::~customFloatingWindow()";
}

QWidget * customFloatingWindow::takeWidget()
{
    return takeCentralWidget();
}

bool customFloatingWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#ifdef Q_OS_WINDOWS
    MSG* msg = reinterpret_cast<MSG*>(message);
    if(msg->message==WM_SYSCOMMAND)
    {
        if((msg->wParam & 0xfff0) ==IDM_ATTACH)
        {
            *result=0;

            //qDebug()<<"customFloatingWindow::nativeEvent IDM_ATTACH";

            QMetaObject::invokeMethod(parent(),"attachToMdi", Q_ARG(QObject*, this));
            return true;
        }
    }
#endif
    return false;
}

void customFloatingWindow::closeEvent(QCloseEvent *e)
{
    QVariant closable = property("CLOSABLE");
    if(closable.isValid())
    {
        if(closable.toBool())
        {
            e->accept();
            return;
        }
    }
    this->hide();
    e->ignore();
}

QWidget *customFloatingWindow::widget() const
{
    return centralWidget();
}
