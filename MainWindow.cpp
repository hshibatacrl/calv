/**
 * @file MainWindow.cpp
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

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QMdiSubWindow>
#include <QStandardPaths>
#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QFile>
#include <QTimer>
#include <QImage>
#include <QLabel>

#include "configStorage.h"
#include "customGLWidget.h"
#include "customFloatingWindow.h"
#include "customMdiSubWindow.h"
#include "aboutDialog.h"
#include "fdd.h"
#include "tcpClientDialog.h"
#include "pointcloud_packet.h"
#include "pose_packet.h"
#include "orb_packet_type.h"

#include "gl_pcloud_entity.h"
#include "gl_polyline_entity.h"
#include "gl_poses_entity.h"
#include "gl_model_entity.h"

void storeLastFolder(QString fileName, QString label)
{
    QFileInfo fi(fileName);
    configStorage c(QString("lastFolder"),nullptr);
    auto p=c.load("last");
    p[label]=fi.absolutePath();
    c.save(p,"last");
}

QString getLastFolder(QString label)
{
    configStorage c(QString("lastFolder"),nullptr);
    auto p=c.load("last");
    if(p.contains(label)) return p[label].toString();
    return QString();
}

#define VIEW3D_INDEX -1
#define IMAGE_INDEX 0
#define ELAPSED_INDEX 100

#define IS_TCP_ACTIVE _dec->status().startsWith("CONN")
#define IS_PBK_ACTIVE _dec->status().startsWith("PLAY")

bool MainWindow::IS_ENABLE(int x) const
{
    return _mdi.contains(x);
}

bool MainWindow::IS_SHOWN(int x) const
{
    if(IS_ENABLE(x)) return _mdi[x]->isVisible();
    return false;
}

void MainWindow::reset()
{
    foreach(auto key, _mdi.keys())
    {
        QMetaObject::invokeMethod(_mdi[key]->widget(),"reset",Qt::QueuedConnection);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/icons/crl.png"));

    _dec = new fdd(this);
    _inpStatus = new QLabel(this);
    _logStatus = new QLabel(this);
    ui->statusbar->addWidget(_inpStatus);
    ui->statusbar->addWidget(_logStatus);

    _glWidget = nullptr;

    create3DView();

    _logging = 0;
    _init = 1;

    _timer = new QTimer(this);
    _timer->start(1000);

    connect(_timer,&QTimer::timeout, this, [=](){
        QString text="LOG ";
        if(_logging && _log!=nullptr)
        {
            QFileInfo fi(*_log);
            text += fi.fileName()+ QString(" %1MB").arg(fi.size()/(1024.0*1024.0),0, 'f', 3);
        }
        else
        {
            text += "IDLE";
        }
        _logStatus->setText(text);
        _inpStatus->setText(_dec->status());
    });

    connect(_dec, &fdd::received, this, [=](const QByteArray &packet)
    {
        if(packet.size()>4)
        {
            const uint32_t *magic=(const uint32_t*)packet.data();
            if(magic[0]==PC_MAGIC)
            {
                const pc_packet_header_t *p = (const pc_packet_header_t *)packet.data();
                qDebug()<<"Point Cloud" << p->length;
                auto obj=new gl_pcloud_entity;
                ui->tree->created(obj);
                _glWidget->delayLoad(obj, packet);
            }
            if(magic[0]==POSE_MAGIC)
            {
                const pose_packet_header_t *p = (const pose_packet_header_t *)packet.data();
                qDebug()<<"Pose Cloud" << p->length;
                if(_stockModel.contains(p->data[0]))
                {
                    auto obj=new gl_poses_entity(_stockModel[p->data[0]]);
                    ui->tree->created(obj);
                    _glWidget->delayLoad(obj, packet);
                }
                else
                {

                }
            }
        }
#if 0
        const orb_packet_header_t *p = (const orb_packet_header_t*)packet.data();
        int index = IMAGE_INDEX + (p->type & ORB_PACKET_TYPE_RIGHT);
        if(_init)
        {
            if(index) return; //wait for LEFT for first
            _init=0;
        }
        if(!_mdi.contains(index))
        {
            auto w=new customMdiSubWindow(this);
            w->setWidget(new fdImage(this));
            ui->mdiArea->addSubWindow(w);
            _mdi[index]=w;
            w->resize(640,408);
            w->show();
            w->setWindowTitle(index?"RIGHT":"LEFT");
            auto params=imageViewOptionsDialog::iniParams();
            QMetaObject::invokeMethod(w->widget(),"paramChanged",Qt::QueuedConnection,Q_ARG(QVariantMap, params));
        }
        QMetaObject::invokeMethod(_mdi[index]->widget(),"received",Qt::QueuedConnection,Q_ARG(QByteArray, packet));

        if(p->type & ORB_PACKET_TYPE_ELAPSED)
        {
            if(!_mdi.contains(ELAPSED_INDEX))
            {
                auto w=new customMdiSubWindow(this);
                w->setWidget(new fdElapsed(this));
                ui->mdiArea->addSubWindow(w);
                _mdi[ELAPSED_INDEX]=w;
                w->resize(640,816);
                w->show();
                w->setWindowTitle("ELAPSED");
            }
            QMetaObject::invokeMethod(_mdi[ELAPSED_INDEX]->widget(),"received",Qt::QueuedConnection,Q_ARG(QByteArray, packet));
        }
#endif
        writeLog(packet);
    });

    connect(ui->menuComm,&QMenu::aboutToShow, this, [=](){
        auto a=_dec->status();
        ui->actionConnect_to_Server->setChecked(IS_TCP_ACTIVE);
    });
    connect(ui->menuFile,&QMenu::aboutToShow, this, [=](){
        bool recActive=_logging;
        ui->actionRecord_Log->setChecked(recActive);
        ui->actionPlayback_Log->setChecked(IS_PBK_ACTIVE);
    });
    connect(ui->menuView, &QMenu::aboutToShow, this, [=](){
        if(_glWidget->parentWidget())
        {
            ui->action3D_View->setChecked(_glWidget->parentWidget()->isVisible());
        }
    });

    setWindowTitle(APP_NAME);

    ui->peLog->setCenterOnScroll(true);
    //ui->mdiArea->tileSubWindows();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::create3DView()
{
    if(_glWidget==nullptr)
    {
        _glWidget = new customGLWidget(this);
        _glWidget->setProperty("DETACHABLE",false);
        auto sub=new customMdiSubWindow(_glWidget, this);
        ui->mdiArea->addSubWindow(sub);
        _mdi[VIEW3D_INDEX] = sub;
        sub->setWindowTitle("3D View");
        sub->show();

        ui->tree->link(_glWidget);

        connect(_glWidget, &customGLWidget::initialized,[=]()
        {
            QStringList models;
            models << ":/gl/models/camera.obj" << ":/gl/models/camera2.obj";

            connect(_glWidget, &customGLWidget::entityLoadedByWidget,[=](QObject*o){
                for(auto i:_stockModelPending.keys())
                {
                    if(qobject_cast<gl_entity_ctx*>(o) == _stockModelPending[i])
                    {
                        _stockModel[i] = _stockModelPending[i];
                    }
                }
            });

            for(int i=0;i<models.size();i++)
            {
                QString model = models[i];
                _stockModelPending[i] = new gl_model_entity;
                _stockModelPending[i]->setReference(true);
                _glWidget->delayLoad(_stockModelPending[i], model.toStdString().c_str());
            }
        });
    }
}


void MainWindow::logMessage(int level,QString text)
{
    if(level<0)
    {
        ui->peLog->appendHtml(text);
    }
    else
    {
        ui->statusbar->showMessage(text,5000);
//            ui->peLog->appendPlainText(text);
    }
}


void MainWindow::attachToMdi(QObject *fltWindow)
{
    customFloatingWindow *w = qobject_cast<customFloatingWindow*>(fltWindow);
    if(w!=nullptr)
    {
        QSize sz = w->widget()->size();
        customMdiSubWindow *sub = new customMdiSubWindow(w->takeWidget(), this);
        sub->setWindowTitle(w->windowTitle());

        if(w->property("CLOSABLE").isValid()) sub->setProperty("CLOSABLE", w->property("CLOSABLE"));

        _floating.removeAll(w);

        ui->mdiArea->addSubWindow(sub);

        w->deleteLater();

        sub->resize(sz);
        sub->show();
    }
}

void MainWindow::detachFromMdi(QObject *mdiWindow)
{
    QMdiSubWindow *w=qobject_cast<QMdiSubWindow*>(mdiWindow);
    if(w!=nullptr)
    {
        QPoint global=w->mapToGlobal(w->pos());
        QSize sz = w->widget()->size();
        customFloatingWindow *floating=new customFloatingWindow(w->widget(), this);
        floating->setWindowTitle(w->windowTitle());
        if(w->property("CLOSABLE").isValid()) floating->setProperty("CLOSABLE", w->property("CLOSABLE"));

        _floating.append(floating);

        ui->mdiArea->removeSubWindow(w);
        w->deleteLater();

        floating->move(global);
        //floating->move(this->mapFromGlobal(global));
        floating->resize(sz);
        floating->show();
        qDebug()<<floating->pos();
    }
}



void MainWindow::closeLog()
{
    if(_logging)
    {
        if(_log!=nullptr)
        {
            _log->close();
            _log->deleteLater();
            _log = nullptr;
        }
        _logging = 0;
    }
}

void MainWindow::writeLog(const QByteArray &bytes)
{
    if(_logging)
    {
        if(_log!=nullptr)
        {
            _log->write(bytes);
        }
    }
}

QString MainWindow::logFolder()
{
    QString x=getLastFolder("log");
    if(x.isEmpty())
    {
        x=QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    return x;
}



void MainWindow::on_actionConfigure_3D_View_triggered()
{
    _glWidget->viewOptionsTriggered();
}


void MainWindow::on_actionAbout_triggered()
{
    aboutDialog dlg;
    dlg.exec();
}


void MainWindow::on_actionConnect_to_Server_triggered()
{
    if(IS_TCP_ACTIVE)
    {
        _dec->idle();
    }
    else
    {
        tcpClientDialog dlg;
        dlg.setWindowTitle("Server ip");
        if(dlg.exec()==QDialog::Accepted)
        {
            auto p=dlg.param();
            if(p.contains("lePort") && p.contains("leAddr"))
            {
                int port = p["lePort"].toInt();
                if(port>0)
                {
                    reset();
                    _dec->connectToCamera(p["leAddr"].toString(),port);
                }
            }
        }
    }
}


void MainWindow::on_actionRecord_Log_triggered()
{
    if(_logging)
    {
        closeLog();
    }
    else
    {
        auto def=logFolder()+"/*.calog";
        auto fileName=QFileDialog::getSaveFileName(this,"Log File to Save",def,"Calib Log(*.calog)");
        if(!fileName.isEmpty())
        {
            _log = new QFile(fileName);
            if(_log->open(QFile::WriteOnly))
            {
                _logging = 1;
                storeLastFolder(fileName,"log");
            }
            else
            {
                _log->deleteLater();
            }
        }
    }
}


void MainWindow::on_actionPlayback_Log_triggered()
{
    if(IS_PBK_ACTIVE)
    {
        _dec->idle();
    }
    else
    {
        auto fileName=QFileDialog::getOpenFileName(this,"Log File to Playback",logFolder(),"Calib Log(*.calog)");
        if(!fileName.isEmpty())
        {
            storeLastFolder(fileName,"log");
            reset();
            _dec->startPlayback(fileName);
        }
    }
}

void MainWindow::on_action3D_View_triggered()
{
    if(_glWidget->parentWidget()->isHidden()) _glWidget->parentWidget()->show(); else _glWidget->parentWidget()->hide();
}
