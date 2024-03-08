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
#include <QTimer>
#include <QImage>

#include <QVector>
#include <QVariantList>
#include <QVariantMap>

#include "customFloatingWindow.h"
#include "customMdiSubWindow.h"

#ifdef USE_MAP_VIEW
#include "customMapView.h"
#endif

#ifdef USE_IMAGE_VIEW
#include "customImageWidget.h"
#endif

#ifdef USE_3D_VIEW
#include "customGLWidget.h"
#include "gl_model_entity.h"
#include "gl_poses_entity.h"
#include "gl_pcloud_entity.h"
#include "gl_polyline_entity.h"
#endif

#ifdef USE_PLOT_VIEW
#include "qcpPlotView.h"
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _glWidget = nullptr;
    _stockModelPending = nullptr;
    _stockModel = nullptr;

    ui->peLog->setCenterOnScroll(true);
    //ui->mdiArea->tileSubWindows();

    setWindowTitle(APP_NAME);

#ifdef USE_IMAGE_VIEW
#ifdef EXAMPLE_CODE_IMAGE
    openImage(EXAMPLE_CODE_IMAGE);
#endif
#endif

#ifdef USE_PLOT_VIEW
#ifdef EXAMPLE_CODE_QCP
#ifdef EXAMPLE_CODE_QCP_STATIC_PLOT
    create_qcp_example_static();
#else
    create_qcp_example_realtime();
#endif
#endif
#endif

    create3DView();

#ifdef USE_MAP_VIEW
    createMapView();
#endif

    connect(ui->menuView, &QMenu::aboutToShow, this, [=](){
        ui->action3D_View->setChecked(_glWidget->parentWidget()->isVisible());
#ifdef USE_MAP_VIEW
        ui->actionMap_View->setChecked(_mapWidget->parentWidget()->isVisible());
#endif
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

#ifdef USE_IMAGE_VIEW
void MainWindow::openImage(const QString &fileName)
{
    QImage image(fileName);
    if(!image.isNull())
    {
        QFileInfo fi(fileName);
        auto img=new customImageWidget(this);
        auto sub=new customMdiSubWindow(img, this);
        ui->mdiArea->addSubWindow(sub);
        img->setImage(image);
        sub->setWindowTitle(fi.baseName());
        sub->show();
        _mdi.append(sub);
    }
}
#endif

void MainWindow::create3DView()
{
    if(_glWidget==nullptr)
    {
        _glWidget = new customGLWidget(this);
        _glWidget->setProperty("DETACHABLE",false);
        auto sub=new customMdiSubWindow(_glWidget, this);
        ui->mdiArea->addSubWindow(sub);
        sub->setWindowTitle("3D View");
        sub->show();

        ui->tree->link(_glWidget);

        connect(_glWidget, &customGLWidget::initialized,[=]()
        {
            _stockModelPending = new gl_model_entity;
            _stockModelPending->setReference(true);
            connect(_glWidget, &customGLWidget::entityLoadedByWidget,[=](QObject*o)
            {
                if(qobject_cast<gl_entity_ctx*>(o) == _stockModelPending)
                {
                    _stockModel = _stockModelPending;

#ifdef EXAMPLE_CODE_3D
                    //test code
                    if(_stockModel!=nullptr)
                    {
                        QByteArray dummy;
                        dummy.append((uint8_t)0x00);
                        _glWidget->delayLoad(new gl_poses_entity(_stockModel), dummy);
                        dummy[0]=1;
                        _glWidget->delayLoad(new gl_poses_entity(_stockModel), dummy);
                    }

                    {
                        QByteArray dummy;
                        dummy.append(1);
                        _glWidget->delayLoad(new gl_polyline_entity, dummy);
                        _glWidget->delayLoad(new gl_pcloud_entity, dummy);
                    }
#endif
                }
            });
            _glWidget->delayLoad(_stockModelPending, ":/gl/models/bunny.mqo");
        });
    }
}

#ifdef USE_MAP_VIEW
void MainWindow::createMapView()
{
    _mapWidget = new customMapView(this);
    _mapWidget->layout()->setContentsMargins(0,0,0,0);
    customMdiSubWindow *sub = new customMdiSubWindow(_mapWidget, this);
    sub->setWindowTitle("Map View");
    ui->mdiArea->addSubWindow(sub);
}
#endif

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

void MainWindow::logMessage(int level,QString text)
{
    if(level<0)
    {
        ui->peLog->appendHtml(text);
    }
    else
    {
        ui->statusbar->showMessage(text,5000);
//      ui->peLog->appendPlainText(text);
    }
}


void MainWindow::on_actionConfigure_3D_View_triggered()
{
    _glWidget->viewOptionsTriggered();
}

#include "aboutDialog.h"
void MainWindow::on_actionAbout_triggered()
{
    aboutDialog dlg;
    dlg.exec();    
}


void MainWindow::on_action3D_View_triggered()
{
    if(_glWidget->parentWidget()->isHidden()) _glWidget->parentWidget()->show(); else _glWidget->parentWidget()->hide();
}

#ifdef USE_MAP_VIEW
void MainWindow::on_actionMap_View_triggered()
{
    if(_mapWidget->parentWidget()->isHidden()) _mapWidget->parentWidget()->show(); else _mapWidget->parentWidget()->hide();
}
#endif

#ifdef USE_PLOT_VIEW
#ifdef EXAMPLE_CODE_QCP
#ifdef EXAMPLE_CODE_QCP_STATIC_PLOT
void MainWindow::create_qcp_example_static()
{
    QVariantMap  m;

    QStringList header;
    header << "X" << "Y";
    m["headers"] = header;
    QVector<double> x,y;
    for(double i=0.0;i<2.0*M_PI;i+=0.1*M_PI)
    {
        x.append(i);
        y.append(std::sin(i));
    }
    QVariantList columns;
    columns.append(QVariant::fromValue(x));
    columns.append(QVariant::fromValue(y));
    m["columns"] = columns;
    m["realtime"] = false;

    auto p=new qcpPlotView(m, this);
    auto sub=new customMdiSubWindow(p->widget(), this);
    ui->mdiArea->addSubWindow(sub);
    sub->setWindowTitle("Plot View");
    sub->show();
}
#else
void MainWindow::create_qcp_example_realtime()
{
    _qcp_example_t = 0.0;

    QVariantMap  m;

    QStringList header;
    header << "X" << "Y1" << "Y2";
    m["headers"] = header;
    m["realtime"] = true;

    auto p=new qcpPlotView(m, this);
    auto sub=new customMdiSubWindow(p->widget(), this);
    ui->mdiArea->addSubWindow(sub);
    sub->setWindowTitle("Plot View");
    sub->show();

    QTimer *tqcp=new QTimer(this);
    connect(tqcp,&QTimer::timeout, this, [=]()
    {
        QVector<double> data;
        data.append(_qcp_example_t); //x axis
        data.append(std::sin(2.0*3.14159*_qcp_example_t/1.0)); //y axis
        if(!(int)(std::fmod(_qcp_example_t,10.0)*100.0)) data.append(std::nan("")); //append nan for no data
        else  data.append(std::cos(2.0*3.14159*_qcp_example_t/1.0)); //y axis
        p->addData(data);
        _qcp_example_t += 0.05; //50msec
    });
    tqcp->setInterval(50);
    tqcp->start();
}
#endif
#endif
#endif


#include "serialPortDialog.h"
#include <QSerialPort>
void MainWindow::on_actionSerial_port_triggered()
{
    serialPortDialog dlg(this);
    if(dlg.exec()==QDialog::Accepted)
    {
        auto port = dlg.get(this);
        if(port->open(QIODevice::ReadWrite))
        {
            qDebug()<< "Serial port is opened" << port->portName();


            port->close();  //this is just example code
        }
        else
        {
            qDebug()<< "Serial port open failed" << port->portName();
        }
    }
}

#include "tcpClientDialog.h"

void MainWindow::on_actionTCP_Client_triggered()
{
    tcpClientDialog dlg(this);
    if(dlg.exec()==QDialog::Accepted)
    {

    }
}




