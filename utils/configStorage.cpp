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

#include "configStorage.h"

#include <QVariantMap>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QProcess>
#include <QDebug>

configStorage::configStorage(QString subFolder, QObject *parent) : QObject(parent)
{
    folder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/"+subFolder+"/";
    //qDebug()<<folder;
}

int configStorage::save(const QVariantMap &params, QString basename) const
{
    QDir d(folder);
    if(!d.exists()) d.mkpath(folder);
    if(!d.exists()) return 0;

    QJsonObject obj;
    obj= QJsonObject::fromVariantMap(params);
    QFile f(folder+basename+".json");
    if(f.open(QIODevice::WriteOnly))
    {
        QJsonDocument doc(obj);
        f.write(doc.toJson());
        f.close();
        return 1;
    }
    return 0;
}

QVariantMap configStorage::load(const QString basename) const
{
    QFile f(folder+basename+".json");
    QJsonObject obj;
    if(f.open(QIODevice::ReadOnly))
    {
        QByteArray b=f.readAll();
        obj = QJsonObject(QJsonDocument::fromJson(b).object());
        f.close();
        return obj.toVariantMap();
    }
    return QVariantMap();
}

int configStorage::check(const QVariantMap &params, const QStringList &items) const
{
    for(auto item:items)
    {
        if(!params.contains(item)) return 0;
    }
    return 1;
}



int configStorage::explore() const
{
    QString p=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

#ifdef Q_OS_WIN
    return QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(p)});
#elif defined(Q_OS_LINUX)
    return 0;
#elif defined(Q_OS_MAC)
    return 0;
#endif
}

const QString &configStorage::getFolder() const
{
    return folder;
}

