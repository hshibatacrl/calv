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

#include "customMapView.h"

#include <QDir>
#include <QMenu>
#include <QAction>
#include <QDebug>

#include <QGVGlobal.h>
#include <QGVMapQGView.h>
#include <QGVLayerGoogle.h>
#include <QGVLayerBing.h>
#include <QGVLayerOSM.h>
#include "QGVLayerGSI.h"

#include <QGVWidgetZoom.h>
#include <QGVWidgetCompass.h>
#include <QGVWidgetScale.h>

#include <QNetworkAccessManager>
#include <QNetworkDiskCache>

#include <QApplication>
#include <QClipboard>

#include "configStorage.h"


QNetworkAccessManager *customMapView::_manager = nullptr;
QNetworkDiskCache *customMapView::_cache = nullptr;

customMapView::customMapView(QWidget *parent)
    : QGVMap(parent)
{
    if(_manager==nullptr)
    {
        QDir("cacheDir").removeRecursively();
        _cache = new QNetworkDiskCache(this);
        _cache->setCacheDirectory("cacheDir");
        _manager = new QNetworkAccessManager(this);
        _manager->setCache(_cache);
        QGV::setNetworkManager(_manager);
    }

    addWidget(_zoom=new QGVWidgetZoom());
    addWidget(_compass=new QGVWidgetCompass());
    addWidget(_scale=new QGVWidgetScale(Qt::Horizontal));

    _zoom->setProperty("WidgetType","Widget");
    _zoom->setObjectName("Zoom");
    _compass->setProperty("WidgetType","Widget");
    _compass->setObjectName("Compass");
    _scale->setProperty("WidgetType","Widget");
    _scale->setObjectName("Scale");

    QList<QGV::TilesType> types;
    QMap<QGV::TilesType,QString> typeCaptions;
    types << QGV::TilesType::Schema << QGV::TilesType::Satellite << QGV::TilesType::Hybrid;
    typeCaptions[QGV::TilesType::Schema]="Street";
    typeCaptions[QGV::TilesType::Satellite]="Satellite";
    typeCaptions[QGV::TilesType::Hybrid]="Hybrid";

    _basemapLayers.append(new QGVLayerOSM());
    _basemapLayers.back()->setVisible(false);
    _basemapLayers.back()->setObjectName("OpenStreetMap");
    addItem(_basemapLayers.back());

    foreach(auto type, types)
    {
        _basemapLayers.append(new QGVLayerGoogle(type));
        _basemapLayers.back()->setVisible(false);
        _basemapLayers.back()->setObjectName("GoogleMap "+typeCaptions[type]);
        addItem(_basemapLayers.back());
    }

    _basemapLayers.back()->setVisible(true);

    foreach(auto type, types)
    {
        _basemapLayers.append(new QGVLayerBing(type));
        _basemapLayers.back()->setVisible(false);
        _basemapLayers.back()->setObjectName("BingMap "+typeCaptions[type]);
        addItem(_basemapLayers.back());
    }


    _basemapLayers.append(new QGVLayerGSI(0));
    _basemapLayers.back()->setVisible(false);
    _basemapLayers.back()->setObjectName("GSI Street1");
    addItem(_basemapLayers.back());

    _basemapLayers.append(new QGVLayerGSI(1));
    _basemapLayers.back()->setVisible(false);
    _basemapLayers.back()->setObjectName("GSI Street2");
    addItem(_basemapLayers.back());
    _basemapLayers.append(new QGVLayerGSI(2));
    _basemapLayers.back()->setVisible(false);
    _basemapLayers.back()->setObjectName("GSI Satellite");
    addItem(_basemapLayers.back());

    QMetaObject::invokeMethod(this,"initView", Qt::QueuedConnection);



    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &customMapView::customContextMenuRequested,this,[=](const QPoint &p)
    {
        QMenu menu(this);
        if(prepareContextMenu(p,menu))
        {
            QPoint pt(p);
            menu.exec( mapToGlobal(pt) );
        }
    });

}

customMapView::~customMapView()
{
    auto p=getProjection()->projToGeo(geoView()->getCamera().projRect());
    auto tl=p.topLeft();
    auto br=p.bottomRight();
    QVariantMap params;
    params["top"] = tl.latitude();
    params["left"] = tl.longitude();
    params["bottom"] = br.latitude();
    params["right"] = br.longitude();

    configStorage c("mapView",this);
    c.save(params,"mapRect");
}

void customMapView::initView()
{

//    auto target = getProjection()->boundaryGeoRect();
//    cameraTo(QGVCameraActions(this).scaleTo(target));

    QGV::GeoPos p1(38.89877720775513, -77.03942944738623);
    QGV::GeoPos p2(38.89222909394092, -77.0336357530331);


    configStorage c("mapView",this);
    QVariantMap params=c.load("mapRect");
    if(c.check(params, QStringList()<<"top"<<"bottom"<<"left"<<"right"))
    {
        p1.setLat(params["top"].toDouble());
        p1.setLon(params["left"].toDouble());
        p2.setLat(params["bottom"].toDouble());
        p2.setLon(params["right"].toDouble());
    }

    QGV::GeoRect rc(p1,p2);

    //flyTo(QGVCameraActions(this).scaleTo(rc));
    cameraTo(QGVCameraActions(this).scaleTo(rc));


}

QSize customMapView::sizeHint() const
{
    return QSize(1024,768);
}

void customMapView::flyTo(QVariantList rc)
{
    //QVariantList()<<rc.latTop()<<rc.lonLeft()<<rc.latBottom()<<rc.lonRigth();
    QGV::GeoPos p1(rc[0].toDouble(), rc[1].toDouble());
    QGV::GeoPos p2(rc[2].toDouble(), rc[3].toDouble());
    QGV::GeoRect grc(p1,p2);
    QGVMap::flyTo(QGVCameraActions(this).scaleTo(grc));
    //cameraTo(QGVCameraActions(this).scaleTo(grc));

}

void customMapView::removeItems(QString path)
{
    QList<QGVItem*> itemsToRemove;
    int n=countItems();
    for(int i=0;i<n;i++)
    {
        QGVItem *item=getItem(i);
        if(item!=nullptr)
        {
            auto a=item->property("taskArg");
            if(a.isValid())
            {
                auto arg=a.toMap();
                if(arg.contains("decoder"))
                {
                    auto obj = arg["decoder"].value<QObject*>();
                    qDebug()<<obj<<obj->objectName();
                    if(obj->objectName() == path)
                    {
                        itemsToRemove.append(item);
                    }
                }
            }
        }
    }

    foreach(auto i, itemsToRemove) this->removeItem(i);
}

static QAction *add(QMenu &menu, QString caption, QVariant data, int checked=-1, int enabled = -1, QWidget *p=nullptr )
{
    QAction *a = new QAction(caption,p);
    a->setData(data);
    if(checked>=0)
    {
        a->setCheckable(true);
        a->setChecked(checked);
    }
    if(enabled>=0)
    {
        a->setEnabled(enabled?true:false);
    }
    menu.addAction(a);
    return a;
}

QAction *customMapView::addLayerMenu(QMenu &menu, QGVItem *item, int enabled)
{
    QMenu * m =menu.addMenu(item->objectName());
    QAction *a = m->menuAction();
    a->setCheckable(true);
    a->setChecked(item->isVisible());
    if(enabled>=0)
    {
        a->setEnabled(enabled?true:false);
    }


    QAction *toggleAction = new QAction(item->isVisible()?"Hide":"Show", this);
    m->addAction(toggleAction);
    connect(toggleAction,&QAction::triggered, this, [=]()
    {
        item->setVisible( !item->isVisible() );
    });

    m->addSeparator();

    QVariant v;
    QAction *exportAction = new QAction("Export", this);
    m->addAction(exportAction);
    v=item->property("ExportAction");
    exportAction->setEnabled(v.isValid());
    connect(exportAction,&QAction::triggered, this, [=]()
    {
        QMetaObject::invokeMethod(item, v.toString().toLocal8Bit().data());
    } );

    QAction *propertyAction = new QAction("Property", this);
    m->addAction(propertyAction);
    v=item->property("PropertyAction");
    propertyAction->setEnabled(v.isValid());
    connect(propertyAction,&QAction::triggered, this, [=]()
    {
        QMetaObject::invokeMethod(item, v.toString().toLocal8Bit().data());
    } );

    m->addSeparator();

    QAction *delAction = new QAction("Delete", this);
    m->addAction(delAction);
    connect(delAction,&QAction::triggered, this, [=]()
    {
        removeItem(item);
        item->deleteLater();
    } );

    return a;
}


template <typename T> static void showOne(QList<T> &items, T itemToShow)
{
    foreach(auto item, items)
    {
        item->setVisible(item==itemToShow);
    }
}

int customMapView::prepareContextMenu(const QPoint &p, QMenu &menu)
{
    int ret=0;

    QAction *capture=new QAction("Capture image", this);
    connect(capture, &QAction::triggered, this, [=]()
    {
        auto image=this->grab().toImage();
        QApplication::clipboard()->setImage(image, QClipboard::Clipboard);
    });
    menu.addAction(capture);
    ret++;
    menu.addSeparator();


    QMenu *basemap=menu.addMenu("Basemap");
    foreach(auto layer, _basemapLayers)
    {        
        connect(add(*basemap,layer->objectName(),QVariant(),layer->isVisible(),-1,this), &QAction::triggered, this, [=]()
        {
            showOne(_basemapLayers, layer);
        });
        ret++;
    }

    QMap<QString, QMenu*> layerMenu;
    int n=countItems();
    for(int i=0;i<n;i++)
    {
        QGVItem *item=getItem(i);
        if(item!=nullptr)
        {
            QVariant t=item->property("LayerType");
            if(t.isValid())
            {
                QString layerType = t.toString();
                if(!layerMenu.contains(layerType))
                {
                    layerMenu[layerType] = menu.addMenu(layerType);
                }
                addLayerMenu(*layerMenu[layerType],item);
                ret++;
            }
        }
    }

    n=countWidgets();
    for(int i=0;i<n;i++)
    {
        QGVWidget *item = getWigdet(i);
        if(item!=nullptr)
        {
            QVariant t=item->property("WidgetType");
            if(t.isValid())
            {
                QString widgetType = t.toString();
                if(!layerMenu.contains(widgetType))
                {
                    layerMenu[widgetType] = menu.addMenu(widgetType);
                }
                connect( add(*layerMenu[widgetType], item->objectName(), QVariant(), item->isVisible(), -1, this), &QAction::triggered, this, [=]()
                {
                    item->setVisible( !item->isVisible() );
                });
                ret++;
            }

        }
    }


    return ret;
}


