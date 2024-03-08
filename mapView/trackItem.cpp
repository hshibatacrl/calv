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

#include "trackItem.h"

#include <QPen>
#include <QBrush>
#include <QPainter>

trackItem::trackItem(QList<QGV::GeoPos> track, QGV::GeoRect rc, QGV::ItemFlags flags, QPen pen)
{
    _geoTrack = track;
    _geoRc = rc;
    _pen = pen;
    flags|=QGV::ItemFlag::SelectCustom;
    setFlags(flags);
    setSelectable(true);

    _pen.setCosmetic(true);
}

QPainterPath trackItem::projShape() const
{
    QPainterPath path;
    path.addRect(_projRc);

    return path;
    //return _path;
}

void trackItem::projPaint(QPainter *painter)
{
#if 0
    QPen pen;
    if (isFlag(QGV::ItemFlag::Highlighted) && isFlag(QGV::ItemFlag::HighlightCustom)) {
        pen = QPen(QBrush(_color), 5);
    } else {
        pen = QPen(QBrush(_color), 1);
    }
#endif
    if(_pen.style()==Qt::NoPen)
    {
        QPen p(_pen.color());
        p.setWidthF(_pen.widthF());
        p.setCapStyle(Qt::RoundCap);
        painter->setPen(p);
        painter->drawPoints(_projTrack.toVector().data(), _projTrack.size());
    }
    else
    {
        painter->setPen(_pen);
        painter->drawPolyline(_projTrack.toVector().data(), _projTrack.size());
    }
    //pen.setCosmetic(true);
    //painter->setBrush(QBrush(_color));
    //painter->drawPath(_path);
//    painter->drawPolyline(_projTrack.toVector().data(), _projTrack.size());
    //painter->drawPoints(_projTrack.toVector().data(), _projTrack.size());
#if 0
    if (isSelected() && isFlag(QGV::ItemFlag::SelectCustom))
    {
//        painter->drawLine(mProjRect.topLeft(), mProjRect.bottomRight());
//        painter->drawLine(mProjRect.topRight(), mProjRect.bottomLeft());
    }
#endif
}


void trackItem::onProjection(QGVMap *geoMap)
{
    QGVDrawItem::onProjection(geoMap);
    _projTrack.clear();
    _projTrack.reserve(_geoTrack.size());
    foreach(auto g, _geoTrack)
    {
        _projTrack.append(geoMap->getProjection()->geoToProj(g));
    }    
    _projRc = geoMap->getProjection()->geoToProj(_geoRc);
}
