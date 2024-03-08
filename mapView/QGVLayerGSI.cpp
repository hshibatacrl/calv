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

#include "QGVLayerGSI.h"

namespace {
// clang-format off
const QStringList URLTemplates = {
    "https://cyberjapandata.gsi.go.jp/xyz/std/{z}/{x}/{y}.png",
    "https://cyberjapandata.gsi.go.jp/xyz/pale/{z}/{x}/{y}.png",
    "https://cyberjapandata.gsi.go.jp/xyz/seamlessphoto/{z}/{x}/{y}.jpg",
};
// clang-format on
}

QGVLayerGSI::QGVLayerGSI(int serverNumber) : mUrl(URLTemplates.value(serverNumber))
{
    setName("GSI");
    setDescription("Copyrights 息 GSI");
}

int QGVLayerGSI::minZoomlevel() const
{
    return 2;
}

int QGVLayerGSI::maxZoomlevel() const
{
    return 18;
}

QString QGVLayerGSI::tilePosToUrl(const QGV::GeoTilePos &tilePos) const
{
    QString url = mUrl.toLower();
    url.replace("{z}", QString::number(tilePos.zoom()));
    url.replace("{x}", QString::number(tilePos.pos().x()));
    url.replace("{y}", QString::number(tilePos.pos().y()));
    return url;
}
