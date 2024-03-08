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

#include "entitiesTree.h"

#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QDebug>

#define ROLE_CTX (Qt::UserRole+1)

entitiesTree::entitiesTree(QWidget *parent):QTreeWidget(parent)
{
    _glWidget = nullptr;

    header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);


    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,&QTreeWidget::customContextMenuRequested,this,[=]( const QPoint & pos){
        QMenu menu(this);

        if(_rev.size())
        {
            foreach(auto prefix, prefixes())
            {
                auto sub=new QMenu (prefix, &menu);
                auto a=new QAction("Hide "+prefix, this);
                connect(a, &QAction::triggered, this, [=](){ for(auto &i:_rev) hideItem(i,prefix); });
                sub->addAction(a);

                auto b=new QAction("Show "+prefix, this);
                connect(b, &QAction::triggered, this, [=](){ for(auto &i:_rev) showItem(i,prefix); });
                sub->addAction(b);
                menu.addMenu(sub);
            }
            menu.addSeparator();
        }

        QTreeWidgetItem *nd = itemAt( pos );
        if(nd!=nullptr)
        {
            gl_entity_ctx *gle= qobject_cast<gl_entity_ctx*>( nd->data(0,ROLE_CTX).value<QObject*>() );
            if(gle!=nullptr)
            {
                if(gle->isUnloadable())
                {
                    auto a=new QAction("Unload "+nd->text(0), this);
                    connect(a, &QAction::triggered, this, [=](){_glWidget->entityUnload(gle);});
                    menu.addAction(a);
                }
            }
        }

        if(_rev.size())
        {
            {
                auto a=new QAction("Unload all", this);
                connect(a, &QAction::triggered, this, [=](){
                    auto rev = _rev;
                    for(auto &i:rev) unloadItem(i);
                });
                menu.addAction(a);
            }
        }


        if(menu.actions().size())
        {
            QPoint pt(pos);
            menu.exec( mapToGlobal(pt) );
        }
    });

    connect(this,&QTreeWidget::itemDoubleClicked, this, [=](QTreeWidgetItem *item, int column){
        gl_entity_ctx *gle= qobject_cast<gl_entity_ctx*>( item->data(0,ROLE_CTX).value<QObject*>() );
        if(gle!=nullptr)
        {
            _glWidget->entityClicked(gle);
        }

    });
}

void entitiesTree::link(customGLWidget *glWidget)
{
    _glWidget = glWidget;
    connect(_glWidget, SIGNAL(entityLoadedByWidget(QObject*)), this, SLOT(loaded(QObject*)));
    connect(_glWidget, SIGNAL(entityUnloaded(QObject*)), this, SLOT(unloaded(QObject*)));

    connect(this, &entitiesTree::itemChanged, this,[=](QTreeWidgetItem *item, int column)
    {

        if(!column)
        {
            gl_entity_ctx *gle= qobject_cast<gl_entity_ctx*>( item->data(0,ROLE_CTX).value<QObject*>() );
            gle->setShow(item->checkState(0));
            _glWidget->redrawEntity();
        }
    });
}

void entitiesTree::loaded(QObject *x)
{
    gl_entity_ctx *loaded= qobject_cast<gl_entity_ctx*>(x);
    if(loaded->is_valid())
    {
        if(!loaded->isReference())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(this,QStringList()<<loaded->getCaption());
            item->setData(0,ROLE_CTX,QVariant::fromValue<QObject*>(loaded));
            item->setCheckState(0, Qt::Checked);
            addTopLevelItem(item);
            _rev[loaded->uniqueId()] = item;
        }
        emit _glWidget->entityLoaded(x);
    }
    else
    {
        delete loaded;
    }
}

void entitiesTree::unloaded(QObject *x)
{
    gl_entity_ctx *unloaded= qobject_cast<gl_entity_ctx*>(x);
    if(unloaded->is_valid())
    {
        if(_rev.contains(unloaded->uniqueId()))
        {
            auto item = _rev[unloaded->uniqueId()];
            _rev.remove(unloaded->uniqueId());
            auto index=indexFromItem(item);
            takeTopLevelItem(index.row());
        }
    }
}

void entitiesTree::unloadItem(QTreeWidgetItem *x)
{
    if(x!=nullptr)
    {
        gl_entity_ctx *gle= qobject_cast<gl_entity_ctx*>( x->data(0,ROLE_CTX).value<QObject*>() );
        if(gle!=nullptr)
        {
            if(gle->isUnloadable())
            {
                _glWidget->entityUnload(gle);
            }
        }
    }
}

void entitiesTree::hideItem(QTreeWidgetItem *x, QString prefix)
{
    if(x!=nullptr)
    {
        if(x->text(0).startsWith(prefix)) x->setCheckState(0,Qt::Unchecked);
    }
}

void entitiesTree::showItem(QTreeWidgetItem *x, QString prefix)
{
    if(x!=nullptr)
    {
        if(x->text(0).startsWith(prefix)) x->setCheckState(0,Qt::Checked);
    }
}

QStringList entitiesTree::prefixes()
{
    QStringList ret;
    ret <<"pose"<<"pcloud";
    return ret;
}
