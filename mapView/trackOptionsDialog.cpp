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

#include "trackOptionsDialog.h"
#include "ui_trackOptionsDialog.h"

#include "colorizer.h"

#include <QColorDialog>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QToolTip>

#include <algorithm>
#include <cfloat>

trackOptionsDialog::trackOptionsDialog(QString key, QString pktKey, QStringList hdr, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::trackOptionsDialog)
{
    ui->setupUi(this);
    _hdr = hdr;
    _key=key;
    _pktKey=pktKey;

    _inhibitNormalizeColDsb=0;
    _inhibitNormalizeFiltDsb=0;


    ui->leKey->setText(key);

    ui->cbPreset->setDuplicatesEnabled(false);

    ui->cbItems4Color->addItem("None", -1);
    ui->cbItems4Filter->addItem("None", -1);

    for(int i=0;i<_hdr.size();i++)
    {
        auto h = _hdr[i];
        ui->cbItems4Color->addItem(h, i);
        ui->cbItems4Filter->addItem(h, i);
    }

    ui->cbLineType->addItem("No Line",QVariant::fromValue(Qt::NoPen));
    ui->cbLineType->addItem("Solid Line",QVariant::fromValue(Qt::SolidLine));
    ui->cbLineType->addItem("Dash Line",QVariant::fromValue(Qt::DashLine));
    ui->cbLineType->addItem("Dot Line",QVariant::fromValue(Qt::DotLine));
    ui->cbLineType->addItem("Dash Dot Line",QVariant::fromValue(Qt::DashDotLine));
    ui->cbLineType->addItem("Dash Dot Dot Line",QVariant::fromValue(Qt::DashDotDotLine));

    ui->cbLineThickness->addItem("1pt", 1.0);
    ui->cbLineThickness->addItem("1.5pt", 1.5);
    ui->cbLineThickness->addItem("2.0pt", 2.0);
    ui->cbLineThickness->addItem("2.5pt", 2.5);
    ui->cbLineThickness->addItem("3.0pt", 3.0);
    ui->cbLineThickness->addItem("3.5pt", 3.5);
    ui->cbLineThickness->addItem("4.0pt", 4.0);
    ui->cbLineThickness->addItem("4.5pt", 4.5);

    ui->dsColMin->setRange(-DBL_MAX, DBL_MAX);
    ui->dsColMid->setRange(-DBL_MAX, DBL_MAX);
    ui->dsColMax->setRange(-DBL_MAX, DBL_MAX);
    ui->dsFiltMin->setRange(-DBL_MAX, DBL_MAX);
    ui->dsFiltMax->setRange(-DBL_MAX, DBL_MAX);

    ui->lbSample->installEventFilter(this);
    ui->lbSample->setMouseTracking(true);


    ui->pbSavePreset->setEnabled(false);
    ui->pbDeletePreset->setEnabled(false);

    // default values
    _opt["lineType"] = 0;
    _opt["lineThickness"] = 0;
    _opt["colorType"] =0;
    _opt["item4color"] =0;
    _opt["solidColor"] = QColor(255,0,0);
    _opt["maxColor"] = QColor(0,255,0);
    _opt["midColor"] = QColor(255,255,0);
    _opt["minColor"] = QColor(255,0,0);
    _opt["colMin"] = -1.0;
    _opt["colMax"] = 1.0;
    _opt["step"] = 1;
    _opt["numOfSteps"] = 5;
    _opt["item4filter"] =0;
    _opt["filtMin"] = -1.0;
    _opt["filtMax"] = 1.0;


    QTimer::singleShot(10,[=]()
    {
        loadPresets();
        //updateToUi();
    });


}

void trackOptionsDialog::updateToUi(QVariantMap *x)
{
    _inhibitNormalizeColDsb=1;
    _inhibitNormalizeFiltDsb=1;

    if(x==nullptr) x=&_opt;

    ui->cbLineType->setCurrentIndex((*x)["lineType"].toInt());
    ui->cbLineThickness->setCurrentIndex((*x)["lineThickness"].toInt());
    ui->tab->setCurrentIndex( (*x)["colorType"].toInt() );
    ui->pbSolidColor->setStyleSheet(QString("background-color: %1").arg((*x)["solidColor"].value<QColor>().name()));
    ui->cbItems4Color->setCurrentIndex( (*x)["item4color"].toInt());
    ui->pbMinColor->setStyleSheet(QString("background-color: %1").arg((*x)["minColor"].value<QColor>().name()));
    ui->pbMidColor->setStyleSheet(QString("background-color: %1").arg((*x)["midColor"].value<QColor>().name()));
    ui->pbMaxColor->setStyleSheet(QString("background-color: %1").arg((*x)["maxColor"].value<QColor>().name()));
    ui->dsColMax->setValue( (*x)["colMax"].toDouble() );
    ui->dsColMid->setValue( (*x)["colMid"].toDouble() );
    ui->dsColMin->setValue( (*x)["colMin"].toDouble() );
    ui->sbNumOfSteps->setValue( (*x)["numOfSteps"].toInt());
    ui->cbItems4Filter->setCurrentIndex( (*x)["item4filter"].toInt());
    ui->dsFiltMin->setValue( (*x)["filtMin"].toDouble() );
    ui->dsFiltMax->setValue( (*x)["filtMax"].toDouble() );

    _temp["solidColor"] = (*x)["solidColor"];
    _temp["minColor"] = (*x)["minColor"];
    _temp["midColor"] = (*x)["midColor"];
    _temp["maxColor"] = (*x)["maxColor"];
    _temp["numOfSteps"]=(*x)["numOfSteps"];
    _temp["colMin"] = (*x)["colMin"];
    _temp["colMid"] = (*x)["colMid"];
    _temp["colMax"] = (*x)["colMax"];
    updateColorBar();

    _inhibitNormalizeColDsb=0;
    _inhibitNormalizeFiltDsb=0;

}

void trackOptionsDialog::loadFromUi(QVariantMap *x)
{
    if(x==nullptr) x=&_opt;

    (*x)["lineType"]=ui->cbLineType->currentIndex();
    (*x)["lineThickness"]=ui->cbLineThickness->currentIndex();
    (*x)["colorType"]=ui->tab->currentIndex();
    (*x)["solidColor"] = _temp["solidColor"];
    (*x)["item4color"]=ui->cbItems4Color->currentIndex();
    (*x)["minColor"] = _temp["minColor"];
    (*x)["midColor"] = _temp["midColor"];
    (*x)["maxColor"] = _temp["maxColor"];
    (*x)["colMin"] = _temp["colMin"];
    (*x)["colMid"] = _temp["colMid"];
    (*x)["colMax"] = _temp["colMax"];
    (*x)["numOfSteps"] = _temp["numOfSteps"];
    (*x)["item4filter"] = ui->cbItems4Filter->currentIndex();
    (*x)["filtMin"] = ui->dsFiltMin->value();
    (*x)["filtMax"] = ui->dsFiltMax->value();
}

void trackOptionsDialog::updateColorBar()
{
    double vmin = _temp["colMin"] .toDouble();
    double vmax = _temp["colMax"] .toDouble();

    colorizer col(_temp);

    QSize sz = ui->lbSample->size();
    interp1d<double> i2x(1.0, (double)(sz.width()-1), vmin, vmax);

    QImage image(sz.width()-2,sz.height()-2,QImage::Format_RGB32);
    {
        QPainter p(&image);

        for(int i=0;i<sz.width();i++)
        {
            double x = i2x.interp((double)i);
            QColor c = col.solve(x);

            p.setPen(QPen(c));
            p.setBrush(QBrush(c));
            p.drawLine(i,0,i,sz.height()-1);
        }
        p.end();
    }
    ui->lbMin->setText(QString("%1").arg(vmin));
    ui->lbMid->setText(QString("%1").arg(vmin + (vmax-vmin)*0.5));
    ui->lbMax->setText(QString("%1").arg(vmax));

    ui->lbSample->setPixmap(QPixmap::fromImage(image));
}


bool trackOptionsDialog::eventFilter(QObject *object, QEvent *event)
{
    if(object == ui->lbSample)
    {
        if(event->type() == QEvent::MouseMove)
        {
            QMouseEvent *mEvent = (QMouseEvent*)event;
            double vmin = _temp["colMin"] .toDouble();
            double vmax = _temp["colMax"] .toDouble();
            QSize sz = ui->lbSample->size();
            interp1d<double> i2x(1.0, (double)(sz.width()-1), vmin, vmax);
            double x=i2x.interp(mEvent->x());
            QString sx=QString("%1").arg(x);
            QToolTip::showText(ui->lbSample->mapToGlobal(mEvent->pos()),sx,ui->lbSample);
        }
        else if(event->type() == QEvent::Resize)
        {
            updateColorBar();
        }
    }
    return false;
}

void trackOptionsDialog::normalizeColDsb()
{
    if(_inhibitNormalizeColDsb) return;
    double vmin = ui->dsColMin->value();
    double vmid = ui->dsColMid->value();
    double vmax = ui->dsColMax->value();
    if(vmin<=vmid && vmid<=vmax)
    {

    }
    else
    {
        QList<double> val;
        val<<vmin<<vmid<<vmax;

        std::sort(val.begin(), val.end());
        ui->dsColMin->setValue(val[0]);
        ui->dsColMid->setValue(val[1]);
        ui->dsColMax->setValue(val[2]);
    }

    _temp["colMin"] = ui->dsColMin->value();
    _temp["colMid"] = ui->dsColMid->value();
    _temp["colMax"] = ui->dsColMax->value();
}

void trackOptionsDialog::normalizeFiltDsb()
{
    if(_inhibitNormalizeFiltDsb) return;
    double vmin = ui->dsFiltMin->value();
    double vmax = ui->dsFiltMax->value();
    if(vmin<=vmax) return;
    ui->dsFiltMin->setValue(vmax);
    ui->dsFiltMax->setValue(vmin);
}


trackOptionsDialog::~trackOptionsDialog()
{
    delete ui;
}

void trackOptionsDialog::on_pbSolidColor_clicked()
{
    pickColor("solidColor");
    ui->pbSolidColor->setStyleSheet(QString("background-color: %1").arg(_temp["solidColor"].value<QColor>().name()));
}

void trackOptionsDialog::on_pbMinColor_clicked()
{
    pickColor("minColor");
    ui->pbMinColor->setStyleSheet(QString("background-color: %1").arg(_temp["minColor"].value<QColor>().name()));
    updateColorBar();

}

void trackOptionsDialog::on_pbMidColor_clicked()
{
    pickColor("midColor");
    ui->pbMidColor->setStyleSheet(QString("background-color: %1").arg(_temp["midColor"].value<QColor>().name()));
    updateColorBar();
}

void trackOptionsDialog::on_pbMaxColor_clicked()
{
    pickColor("maxColor");
    ui->pbMaxColor->setStyleSheet(QString("background-color: %1").arg(_temp["maxColor"].value<QColor>().name()));
    updateColorBar();
}


QVariantMap trackOptionsDialog::opt() const
{
    return _opt;
}

void trackOptionsDialog::pickColor(QString key)
{
    QColorDialog dlg(_temp[key].value<QColor>(),this);
    dlg.setOption(QColorDialog::ShowAlphaChannel,false);
    if (dlg.exec() == QDialog::Accepted)
    {
        _temp[key] = dlg.currentColor();
    }
}

void trackOptionsDialog::on_buttonBox_accepted()
{
    loadFromUi();
    _opt["lineTypeValue"]=ui->cbLineType->itemData(ui->cbLineType->currentIndex());
    _opt["lineThicknessValue"]=ui->cbLineThickness->itemData(ui->cbLineThickness->currentIndex());
    _opt["item4colorValue"] = ui->cbItems4Color->itemData(ui->cbItems4Color->currentIndex());
    _opt["item4filterValue"] = ui->cbItems4Filter->itemData(ui->cbItems4Filter->currentIndex());
}


void trackOptionsDialog::on_sbNumOfSteps_valueChanged(int arg1)
{
    _temp["numOfSteps"] = ui->sbNumOfSteps->value();
    updateColorBar();
}

void trackOptionsDialog::on_dsColMin_valueChanged(double arg1)
{
    normalizeColDsb();
    updateColorBar();
}

void trackOptionsDialog::on_dsColMid_valueChanged(double arg1)
{
    normalizeColDsb();
    updateColorBar();
}

void trackOptionsDialog::on_dsColMax_valueChanged(double arg1)
{
    normalizeColDsb();
    updateColorBar();
}

void trackOptionsDialog::on_dsFiltMin_valueChanged(double arg1)
{
    normalizeFiltDsb();
}

void trackOptionsDialog::on_dsFiltMax_valueChanged(double arg1)
{
    normalizeFiltDsb();
}

void trackOptionsDialog::on_cbPreset_currentIndexChanged(const QString &arg1)
{
//    qDebug()<<"on_cbPreset_currentIndexChanged"<<arg1;
    ui->pbSavePreset->setEnabled( !ui->cbPreset->currentText().isEmpty() );
    int i=ui->cbPreset->currentIndex();
    if(i>=0)
    {
        QVariantMap m = ui->cbPreset->itemData(i).toMap();
        updateToUi(&m);

        ui->pbDeletePreset->setEnabled(i>=0);
    }
}

void trackOptionsDialog::on_cbPreset_editTextChanged(const QString &arg1)
{
//    qDebug()<<"on_cbPreset_editTextChanged"<<arg1;

    ui->pbSavePreset->setEnabled( !arg1.isEmpty() );

    int index=ui->cbPreset->findText(ui->cbPreset->currentText());
    ui->pbDeletePreset->setEnabled(index>=0);
}

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>

void trackOptionsDialog::on_pbSavePreset_clicked()
{
    QDir d(presetFolder());
    if(!d.exists()) d.mkpath(presetFolder());

    QVariantMap m;
    loadFromUi(&m);

    QString preset=ui->cbPreset->currentText();
    QJsonObject obj;

    QFile f(presetFileName());
    if(f.open(QIODevice::ReadOnly))
    {
        QByteArray b=f.readAll();
        obj = QJsonObject(QJsonDocument::fromJson(b).object());
        f.close();
    }

    if(obj.contains(preset))
    {
        QMessageBox msg;
        msg.setIcon(QMessageBox::Warning);
        msg.setText(preset+" already exists.");
        msg.setInformativeText("Replace existing profile?");
        msg.setStandardButtons(QMessageBox::Yes|QMessageBox::Cancel);
        msg.setDefaultButton(QMessageBox::Cancel);
        int ret = msg.exec();
        if(ret == QMessageBox::Cancel) return;
    }

    qDebug()<<"check "<<ui->cbPreset->currentText()<<ui->cbPreset->currentIndex()<<ui->cbPreset->lineEdit()->text();

    int index=ui->cbPreset->findText(ui->cbPreset->currentText());
    if( index >=0 )
    {
        ui->cbPreset->setItemData(index,m);
//        qDebug()<<"replaced"<<index;
    }
    else
    {
//        qDebug()<<"added"<<ui->cbPreset->lineEdit()->text();
        ui->cbPreset->addItem(ui->cbPreset->lineEdit()->text(),m);
    }

    obj[preset] = QJsonObject::fromVariantMap(m);

    if(f.open(QIODevice::WriteOnly))
    {
        QJsonDocument doc(obj);
        f.write(doc.toJson());
        f.close();
    }

}

void trackOptionsDialog::loadPresets()
{

    QJsonObject obj;
    QFile f(presetFileName());
    if(f.open(QIODevice::ReadOnly))
    {
        QByteArray b=f.readAll();
        obj = QJsonObject(QJsonDocument::fromJson(b).object());
        f.close();
    }

    QVariantMap jm=obj.toVariantMap();
    foreach(auto k, jm.keys())
    {
        QVariantMap m = jm[k].toMap();
        ui->cbPreset->addItem(k, m);
    }

    if(jm.size()>0)
    {
        ui->cbPreset->setCurrentIndex(0);
    }
    else
    {
        updateToUi();
    }

}

QString trackOptionsDialog::presetFolder()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/trackPresets/";
}

QString trackOptionsDialog::presetFileName()
{
    return presetFolder()+_pktKey+".xml";
}

void trackOptionsDialog::on_pbDeletePreset_clicked()
{
    QMessageBox msg;
    msg.setIcon(QMessageBox::Warning);
    msg.setText("Are you sure?");
    msg.setStandardButtons(QMessageBox::Yes|QMessageBox::Cancel);
    msg.setDefaultButton(QMessageBox::Cancel);
    int ret = msg.exec();
    if(ret == QMessageBox::Cancel) return;

    QString key=ui->cbPreset->currentText();

    QJsonObject obj;

    QFile f(presetFileName());
    if(f.open(QIODevice::ReadOnly))
    {
        QByteArray b=f.readAll();
        obj = QJsonObject(QJsonDocument::fromJson(b).object());
        f.close();
    }

    if(obj.contains(key))
    {
        obj.remove(key);
        if(f.open(QIODevice::WriteOnly))
        {
            QJsonDocument doc(obj);
            f.write(doc.toJson());
            f.close();
        }
        ui->cbPreset->removeItem( ui->cbPreset->currentIndex() );
    }

}
