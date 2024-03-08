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

// QCustomplot itself is GPL License

#include "qcpPlotView.h"

QSize qcpPlotView::sizeHint() const
{
    return QSize(1024,768);
}

qcpPlotView::qcpPlotView(QVariantMap m, QWidget *parent): QCustomPlot(parent), customPlotView(m)
{
    customPlotView::setWidget(this);

    QList<QColor> colors;
    //colors << Qt::red << Qt::darkGreen << Qt::darkBlue << Qt::darkYellow << Qt::cyan << Qt::magenta;
    colors << QColor(0x00, 0x72, 0xbd)
           << QColor(0xD9, 0x53, 0x19)
           << QColor(0xED, 0xB1, 0x20)
           << QColor(0x7E, 0x2F, 0x8E)
           << QColor(0x77, 0xAC, 0x30)
           << QColor(0x4D, 0xBE, 0xEE)
           << QColor(0xA2, 0x14, 0x2F);



    QStringList header=m["headers"].toStringList();

    if(m["realtime"].toBool())
    {
        for(int i=1;i<header.size();i++)
        {
            QCPGraph *g=addGraph();
            g->setPen(QPen(colors[(i-1)%colors.size()],1.0,i<colors.size() ? Qt::SolidLine:Qt::DashLine));
            g->setName(header[i]);
        }
        _realtimeMode = REALTIME_AUTOSCROLL;
    }
    else
    {
        QVariantList columns = m["columns"].toList();
        QVector<double> x = columns.front().value<QVector<double>>();

        for(int i=1;i<columns.size();i++)
        {
            QVector<double> y = columns[i].value<QVector<double>>();

            QCPGraph *g=addGraph();
            g->setPen(QPen(colors[(i-1)%colors.size()],1.0,i<colors.size() ? Qt::SolidLine:Qt::DashLine));
            g->setName(header[i]);
            g->setData(x, y);
        }
        _realtimeMode = STATIC;
    }
    //plotLayout()->setMinimumMargins(QMargins(64,0,0,0));
    xAxis->setLabel(header.front());
    //yAxis->setLabel("y");

    rescaleAxes();

    if(m.contains("y_min"))
    {
        yAxis->setRangeLower(m["y_min"].toDouble());
    }
    if(m.contains("y_max"))
    {
        yAxis->setRangeUpper(m["y_max"].toDouble());
    }

    int x_item=0;
    if(m.contains("x_item"))
    {
        x_item=m["x_item"].toInt();
    }

    _x_item = x_item;


    legend->setVisible(true);
    setInteraction(QCP::iRangeDrag, true);
    setInteraction(QCP::iRangeZoom, true);
    //setInteraction(QCP::iSelectLegend, true);


    connect(this,&QCustomPlot::legendClick, this,[=](QCPLegend* legend, QCPAbstractLegendItem* item, QMouseEvent* event)
    {
        qDebug()<<"QCustomPlot::legendClick"<<legend<<item<<event->button();
        if(!item) return;
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        if(!plItem) return;
        if(event->button()!=Qt::LeftButton) return;
        plItem->plottable()->setVisible(!plItem->plottable()->visible());
        this->replot();
    });

    connect(this, &QCustomPlot::plottableClick, this, [=](QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event)
    {
        qDebug()<<"QCustomPlot::plottableClick"<<plottable<<event->button();
         if(!plottable) return;
         if(event->button()!=Qt::LeftButton) return;
         QCPGraph *g = qobject_cast<QCPGraph*>(plottable);
         if(!g) return;

         qDebug()<<g->scatterStyle().shape();

         if(g->scatterStyle().shape()==QCPScatterStyle::ssNone)
         {
            g->setScatterStyle(QCPScatterStyle::ssCircle);
            qDebug()<<g->scatterStyle().shape();
         }
         else
         {
             g->setScatterStyle(QCPScatterStyle::ssNone);
             qDebug()<<g->scatterStyle().shape();
         }
         this->replot();
    });

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [=](const QPoint &pos)
    {
        QMenu contextMenu(this);
        QAction action1("Capture image", this);
        connect(&action1, &QAction::triggered, this, [=]()
        {
            auto image=this->grab().toImage();
            QApplication::clipboard()->setImage(image, QClipboard::Clipboard);
        });
        contextMenu.addAction(&action1);

        QAction action2("Time sync", this);
        action2.setEnabled(x_item!=0);
        connect(&action2, &QAction::triggered, this, [=]()
        {
            auto r=xAxis->range();
            qInfo()<<"Time sync triggered:"<< header.front() << r.center() << r.size();
            QVariantMap p;
            p["task"] = "TimeSync";
            p["x_axis"] = header.front();
            p["x_item"] = x_item;
            p["center"] = r.center();
            p["size"] = r.size();
            p["this"] = QVariant::fromValue<QObject*>(this);
            QMetaObject::invokeMethod(parent,"uiTaskRequest",Qt::QueuedConnection,Q_ARG(QVariantMap, p));

        });
        contextMenu.addAction(&action2);

        QAction action3("Window sync", this);

        connect(&action3, &QAction::triggered, this, [=]()
        {
            auto r=xAxis->range();
            auto s=yAxis->range();
            qInfo()<<"Window sync triggered:"<< header.front() << r.center() << r.size() << s.center() << s.size();
            QVariantMap p;
            p["task"] = "PlotWindowSync";
            p["x_axis"] = header.front();
            p["x_item"] = x_item;
            p["xcenter"] = r.center();
            p["xsize"] = r.size();
            p["ycenter"] = s.center();
            p["ysize"] = s.size();
            p["this"] = QVariant::fromValue<QObject*>(this);
            p["window"] = rect();
            auto w=property("frame").value<QWidget*>();
            if(w!=nullptr)
            {
                p["frame"] = w->rect();
            }
            QMetaObject::invokeMethod(parent,"uiTaskRequest",Qt::QueuedConnection,Q_ARG(QVariantMap, p));
        });
        contextMenu.addAction(&action3);

        QAction action4("Auto scroll", this);
        switch(_realtimeMode)
        {
        case    STATIC: action4.setEnabled(false); break;
        case    REALTIME_AUTOSCROLL: action4.setEnabled(true); action4.setCheckable(true); action4.setChecked(true);    break;
        case    REALTIME_NOAUTOSCROLL: action4.setEnabled(true); action4.setCheckable(true); action4.setChecked(false);    break;
        }
        connect(&action4, &QAction::triggered, this, [=](bool checked)
        {
            _realtimeMode = checked ? REALTIME_AUTOSCROLL : REALTIME_NOAUTOSCROLL;
        });
        contextMenu.addAction(&action4);


        contextMenu.exec(mapToGlobal(pos));
    });

    connect(this, &QCustomPlot::mouseMove, this, [=](QMouseEvent *e){
        if(e->buttons() & Qt::LeftButton)
        {
            if(_realtimeMode == REALTIME_AUTOSCROLL) _realtimeMode= REALTIME_NOAUTOSCROLL;
        }
    });
}

void qcpPlotView::addData(const QVector<double> &data)
{
    if(_realtimeMode==STATIC) return;

    double x = data.front();
    for(int i=1;i<data.size();i++)
    {
        double y = data.at(i);
        if(std::isnan(y))
        {

        }
        else
        {
            graph(i-1)->addData(x,y);
        }
    }

    if(_realtimeMode == REALTIME_AUTOSCROLL)
    {
        xAxis->setRange(x, xAxis->range().size(), Qt::AlignRight);
    }
    replot();
}

void qcpPlotView::uiTaskRequest(QVariantMap params)
{
    auto obj= params["this"].value<QObject*>();
    if(obj==this) return;   // broadcasted by myself

    if(params["task"].toString()=="TimeSync")
    {
        int x_item = params["x_item"].toInt();
        if(_x_item==x_item)
        {   //sync only same type
            double center = params["center"].toDouble();
            double size = params["size"].toDouble();
            xAxis->setRange(center,size,Qt::AlignCenter);
            this->replot();
        }
    }
    else if(params["task"].toString()=="PlotWindowSync")
    {
        int x_item = params["x_item"].toInt();
        if(_x_item==x_item)
        {   //sync only same type
            double xcenter = params["xcenter"].toDouble();
            double xsize = params["xsize"].toDouble();
            xAxis->setRange(xcenter,xsize,Qt::AlignCenter);
            double ycenter = params["ycenter"].toDouble();
            double ysize = params["ysize"].toDouble();
            yAxis->setRange(ycenter,ysize,Qt::AlignCenter);
            auto rc=params["window"].toRect();
            auto prc=params["frame"].toRect();
            auto w=property("frame").value<QWidget*>();
            qDebug()<<w->size()<<prc<<prc.size();
            qDebug()<<size()<<rc<<rc.size();
            w->resize(prc.size());
            resize(rc.size());
            qDebug()<<w->size()<<prc<<prc.size();
            qDebug()<<size()<<rc<<rc.size();
            this->replot();
        }
    }
    else
    {

    }
}
