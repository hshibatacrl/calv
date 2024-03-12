/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "viewOptionsDialog.h"
#include "ui_viewOptionsDialog.h"

#include <QPushButton>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

viewOptionsDialog::viewOptionsDialog(const QVariantMap &opts, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::viewOptionsDialog)
{
    ui->setupUi(this);

    ui->dsbPersFOV->setValue(opts["dsbPersFOV"].toDouble());
    ui->dsbPersNear->setValue(opts["dsbPersNear"].toDouble());
    ui->dsbPersFar->setValue(opts["dsbPersFar"].toDouble());
    ui->dsbOrthNear->setValue(opts["dsbOrthNear"].toDouble());
    ui->dsbOrthFar->setValue(opts["dsbOrthFar"].toDouble()); 
    ui->cbPointAntiAlias->setChecked( opts["cbPointAntiAlias"].toInt()==1 );
    updateUi();
}

viewOptionsDialog::~viewOptionsDialog()
{
    delete ui;
}

void viewOptionsDialog::on_buttonBox_accepted()
{
    _opts["dsbPersFOV"]=ui->dsbPersFOV->value();
    _opts["dsbPersNear"]=ui->dsbPersNear->value();
    _opts["dsbPersFar"]=ui->dsbPersFar->value();
    _opts["dsbOrthNear"]=ui->dsbOrthNear->value();
    _opts["dsbOrthFar"]=ui->dsbOrthFar->value();
    _opts["cbPointAntiAlias"]=ui->cbPointAntiAlias->checkState()==Qt::Checked ? 1:0;
}

QVariantMap viewOptionsDialog::load(void)
{
    QVariantMap ret;

    //default values
    ret["dsbPersFOV"]=45.0;
    ret["dsbPersNear"]=0.1;
    ret["dsbPersFar"]=10000.0;
    ret["dsbOrthNear"]=-50.0;
    ret["dsbOrthFar"]=5000.0;
    ret["cbPointAntiAlias"]=(int)0;

    QString config=QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QFile configFile(config+"/glWidget.ini");
    if (configFile.open(QIODevice::ReadOnly))
    {
        QByteArray configData = configFile.readAll();
        configFile.close();

        QJsonObject json(QJsonDocument::fromJson(configData).object());

        ret=json.toVariantMap();
    }

    return ret;
}

void viewOptionsDialog::save(QVariantMap x)
{
    QJsonObject json=QJsonObject::fromVariantMap(x);
    QString config=QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir d(config);
    if(!d.exists()) d.mkpath(config);
    QFile configFile(config+"/glWidget.ini");
    if (configFile.open(QIODevice::WriteOnly))
    {
        QJsonDocument doc(json);
        configFile.write(doc.toJson());
        configFile.close();
    }
}

void viewOptionsDialog::updateUi(void)
{
    double a,b,c,d;
    a=ui->dsbPersNear->value();
    b=ui->dsbPersFar->value();
    c=ui->dsbOrthNear->value();
    d=ui->dsbOrthFar->value();

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled((a<b) && (c<d));
}

void viewOptionsDialog::on_dsbPersNear_valueChanged(double arg1)
{
    updateUi();
}

void viewOptionsDialog::on_dsbPersFar_valueChanged(double arg1)
{
    updateUi();
}

void viewOptionsDialog::on_dsbOrthNear_valueChanged(double arg1)
{
    updateUi();
}

void viewOptionsDialog::on_dsbOrthFar_valueChanged(double arg1)
{
    updateUi();
}
