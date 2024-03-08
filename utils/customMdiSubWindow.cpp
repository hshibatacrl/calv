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

#include "customMdiSubWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QMenu>
#include <QAction>
#include <QDebug>

#include <QVBoxLayout>

customMdiSubWindow::customMdiSubWindow(QWidget *widget, QWidget *parent, Qt::WindowFlags flags) : QMdiSubWindow(parent, flags)
{
    _mainWindow=parent;

    layout()->setContentsMargins(0,0,0,0);
    setWidget(widget);

    widget->setProperty("frame", QVariant::fromValue(this));


    bool detachable=true;

    auto propDetachable=widget->property("DETACHABLE");

    if(propDetachable.isValid())
    {
        detachable=propDetachable.toBool();
    }
    if(detachable)
    {
        systemMenu()->addSeparator();
        connect(systemMenu()->addAction("Detach"), &QAction::triggered, this, [=]()
        {
            QMetaObject::invokeMethod(_mainWindow,"detachFromMdi", Q_ARG(QObject*, this));
        });
    }
    //qDebug()<<windowFlags();
    setWindowFlags( windowFlags() & ~Qt::Dialog);
}

customMdiSubWindow::~customMdiSubWindow()
{
    //qDebug()<<"customMdiSubWindow::~customMdiSubWindow()";
}

void customMdiSubWindow::closeEvent(QCloseEvent *e)
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

void customMdiSubWindow::keyPressEvent(QKeyEvent *event)
{
    QApplication::sendEvent(widget(),event);
    //qDebug()<<"key mdi";
    //widget()->keyPressEvent0(event);
}


