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

#include "tcpClientDialog.h"
#include "ui_tcpClientDialog.h"
#include "configStorage.h"

#include <QDebug>

tcpClientDialog::tcpClientDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::tcpClientDialog)
{
    ui->setupUi(this);
    load();
}

tcpClientDialog::~tcpClientDialog()
{
    delete ui;
}


const QVariantMap &tcpClientDialog::param() const
{
    return _param;
}

void tcpClientDialog::save(void)
{
    configStorage s("tcpClientDialog", this);
    s.save(_param,"last");
}

void tcpClientDialog::load(void)
{
    configStorage s("tcpClientDialog", this);
    auto params = s.load("last");
    if(params.contains("lePort")) ui->lePort->setText(params["lePort"].toString());
    if(params.contains("leAddr")) ui->leAddr->setText(params["leAddr"].toString());
}



void tcpClientDialog::on_buttonBox_accepted()
{
    _param["leAddr"] = ui->leAddr->text();
    bool ok;
    int port=ui->lePort->text().toInt(&ok);
    if(ok && port>0 && port<65536)
    {
        _param["lePort"] = port;

        save();
    }
    else
    {
        _param["lePort"] = -1;
    }
    QDialog::accept();
}


void tcpClientDialog::on_buttonBox_rejected()
{
    QDialog::reject();
}

