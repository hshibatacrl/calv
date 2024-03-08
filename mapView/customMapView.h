#ifndef CUSTOMMAPVIEW_H
#define CUSTOMMAPVIEW_H

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

#include <QGeoView/QGVMap.h>
class QNetworkAccessManager;
class QNetworkDiskCache;
class QMenu;
class QGVLayer;
class QGVWidget;

class customMapView : public QGVMap
{
    Q_OBJECT
public:
    explicit customMapView(QWidget *parent = nullptr);
    virtual ~customMapView();

    Q_INVOKABLE void initView(void);
    virtual QSize sizeHint() const;
    void flyTo(QVariantList rc);

    void removeItems(QString path);

signals:

private:
    int prepareContextMenu(const QPoint &p, QMenu &menu);

    QAction *addLayerMenu(QMenu &menu, QGVItem *item, int enabled = -1);

private:
    QGVWidget *_compass;
    QGVWidget *_zoom;
    QGVWidget *_scale;
    QList<QGVLayer*> _basemapLayers;
    static QNetworkAccessManager *_manager;
    static QNetworkDiskCache *_cache;
};

#endif // CUSTOMMAPVIEW_H
