#ifndef ENTITIESTREE_H
#define ENTITIESTREE_H

/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <QTreeWidget>
#include <QUuid>
#include <QMap>
#include "customGLWidget.h"

class QTreeWidgetItem;
class entitiesTree : public QTreeWidget
{
    Q_OBJECT
public:
    entitiesTree(QWidget *parent = nullptr);

    void link(customGLWidget *glWidget);

private slots:
    void check(void);

public slots:
    void created(QObject *x);
    void loaded(QObject *x);
    void unloaded(QObject *x);

protected:
    void unloadItem(QTreeWidgetItem *x);
    void hideItem(QTreeWidgetItem *x, QString prefix);
    void showItem(QTreeWidgetItem *x, QString prefix);

    virtual QStringList prefixes(void);

    void loadedCore(QObject *x);
private:
    customGLWidget *_glWidget;
    QMap<QUuid, QTreeWidgetItem*> _rev;
    QList<QObject*> _order;
    QList<QObject*> _loaded;
};

#endif // ENTITIESTREE_H
