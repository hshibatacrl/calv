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

#include <cstdarg>
#include <cstdio>
#include <mutex>

#include <QApplication>
#include <QWidget>

namespace customLogging
{
static QWidget *widget____;
static void customMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString & msg);
static std::mutex logMutex____;
static QtMessageHandler handler____ = nullptr;

void setWidget(QWidget *w)
{
    std::lock_guard<std::mutex> guard(logMutex____);

    widget____ = w;
    if(w!=nullptr)
    {
        handler____=qInstallMessageHandler(customMessageHandler);
    }
    else
    {
        qInstallMessageHandler(handler____);
    }


}

void logPrintf(const char *format, ...)
{
    std::lock_guard<std::mutex> guard(logMutex____);
    if(nullptr!=widget____)
    {
        va_list argptr;
        va_start(argptr, format);
        QString buf=QString::vasprintf(format, argptr);
        va_end(argptr);
        QMetaObject::invokeMethod(widget____,
              "logMessage",
              Qt::QueuedConnection,
              Q_ARG(int,-1),
              Q_ARG(QString,buf));
    }
}

void logPrint(int type,const QString &line)
{
    std::lock_guard<std::mutex> guard(logMutex____);
    if(nullptr!=widget____)
    {
        QMetaObject::invokeMethod(widget____,
              "logMessage",
              Qt::QueuedConnection,
              Q_ARG(int,type),
              Q_ARG(QString,line));
    }
}

static void customMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString & msg_)
{
    QString msg;
    if(msg_.startsWith('"')&&msg_.endsWith('"'))
    {
        msg = msg_.mid(1, msg_.length()-2);
    }
    else
    {
        msg=msg_;
    }

    if(msg.endsWith("*SB*"))
    {
        logPrint(1,msg.left(msg.length()-4));
        return;
    }

    QString m=qFormatLogMessage(type,ctx,msg.trimmed());
    QString txt;
    QString typ;
    switch (type) {
    case QtDebugMsg:
        typ = QString("<b>[DEBUG]</b>");
        txt = QString("<font color=\"blue\"><b>%1</b></font>").arg(m);
        break;
    case QtWarningMsg:
        typ = QString("<b>[WARNING]</b>");
        txt = QString("<font color=\"orange\">%1</font>").arg(m);
        break;
    case QtCriticalMsg:
        typ = QString("<b>[CRITICAL]</b>");
        txt = QString("<font color=\"red\"><b>%1</b></font>").arg(m);
        break;
    case QtFatalMsg:
        typ = QString("<b>[FATAL]</b>");
        txt = QString("<font color=\"red\"><b>%1</b></font>").arg(m);
        break;
    case QtInfoMsg:
        typ = QString("<b>[INFO]</b>");
        txt = QString("<font color=\"green\">%1</font>").arg(m);
        break;
    default:
        typ = QString("<b>[%1]</b>").arg((int)type);
        txt = QString("%1").arg(m);
    }

    txt=typ+" "+txt;

//    QDateTime t=QDateTime::currentDateTime();
//    txt=t.toString("yyyy/MM/dd hh:mm:ss.zzz ")+txt;

    logPrint(-1,txt);

}

} // namespace w2rLogging
