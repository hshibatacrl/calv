#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/**
 * @file MainWindow.h
 *
 * Definitions of camera objects used in the calibration process
 *
 * Copyright 2023
 * Carnegie Robotics, LLC
 * 4501 Hatfield Street, Pittsburgh, PA 15201
 * https://www.carnegierobotics.com
 *
 * Significant history (date, user, job code, action):
 *   2023-12-13, hshibata@carnegierobotics.com, IRAD2033, Taken and modified file from open source.
 */

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


#include <QMainWindow>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class customMapView;
class customFloatingWindow;
class customMdiSubWindow;
class QMdiSubWindow;
class customGLWidget;
class QFile;
class QLabel;
class fdd;
class gl_entity_ctx;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Q_INVOKABLE void attachToMdi(QObject *fltWindow);
    Q_INVOKABLE void detachFromMdi(QObject *mdiWindow);
    
private:
    void closeLog(void);
    void writeLog(const QByteArray &bytes);
    QString logFolder(void);

    bool IS_ENABLE(int x) const;
    bool IS_SHOWN(int x) const;

    void reset(void);

public slots:
    void logMessage(int level,QString text);

protected:

    void create3DView(void);

private slots:
    void on_action3D_View_triggered();

    void on_actionConfigure_3D_View_triggered();

    void on_actionAbout_triggered();

    void on_actionConnect_to_Server_triggered();

    void on_actionRecord_Log_triggered();

    void on_actionPlayback_Log_triggered();

private:
    Ui::MainWindow *ui;

    fdd *_dec;

    customGLWidget *_glWidget;

    QMap<int, customMdiSubWindow*> _mdi;
    QList<customFloatingWindow*> _floating;

    int _init;

    int _logging;
    QFile *_log;

    QTimer *_timer;
    QLabel *_logStatus;
    QLabel *_inpStatus;

    QMap<int, gl_entity_ctx *> _stockModelPending;
    QMap<int, gl_entity_ctx *> _stockModel;

};
#endif // MAINWINDOW_H
