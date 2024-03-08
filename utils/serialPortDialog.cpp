#include "serialPortDialog.h"
#include "ui_serialPortDialog.h"

#include "configStorage.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPushButton>

serialPortDialog::serialPortDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::serialPortDialog)
{
    ui->setupUi(this);

    auto ports=QSerialPortInfo::availablePorts();
    for(const auto &i:ports)
    {
        ui->cbPorts->addItem(i.portName() + " " + i.description(), i.portName());
    }
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ports.size()>0);

    ui->cbBaudrates->addItem("115200", QSerialPort::Baud115200);
    ui->cbBaudrates->addItem("57600", QSerialPort::Baud57600);
    ui->cbBaudrates->addItem("38400", QSerialPort::Baud38400);
    ui->cbBaudrates->addItem("19200", QSerialPort::Baud19200);
    ui->cbBaudrates->addItem("9600", QSerialPort::Baud9600);
    ui->cbBaudrates->addItem("4800", QSerialPort::Baud4800);
    ui->cbBaudrates->addItem("2400", QSerialPort::Baud2400);
    ui->cbBaudrates->addItem("1200", QSerialPort::Baud1200);

    ui->cbData->addItem("8bit",QSerialPort::Data8);
    ui->cbData->addItem("7bit",QSerialPort::Data7);
    ui->cbData->addItem("6bit",QSerialPort::Data6);
    ui->cbData->addItem("5bit",QSerialPort::Data5);

    ui->cbStop->addItem("1bit",QSerialPort::OneStop);
    ui->cbStop->addItem("1.5bit",QSerialPort::OneAndHalfStop);
    ui->cbStop->addItem("2bit",QSerialPort::TwoStop);

    ui->cbParity->addItem("NoParity",QSerialPort::NoParity);
    ui->cbParity->addItem("Even",QSerialPort::EvenParity);
    ui->cbParity->addItem("Odd",QSerialPort::OddParity);

    configStorage s("serialPortDialog", this);
    QVariantMap m=s.load("last");
    if(m.contains("port"))
    {
        auto i=ui->cbPorts->findData(m["port"]);
        if(i>=0) ui->cbPorts->setCurrentIndex(i);
    }
    if(m.contains("baud"))
    {
        ui->cbBaudrates->setCurrentText(QString("%1").arg(m["baud"].toUInt()));
    }
    if(m.contains("parity"))
    {
        auto i=ui->cbParity->findData(m["parity"]);
        if(i>=0) ui->cbParity->setCurrentIndex(i);
    }
    if(m.contains("data"))
    {
        auto i=ui->cbData->findData(m["data"]);
        if(i>=0) ui->cbData->setCurrentIndex(i);
    }
    if(m.contains("stop"))
    {
        auto i=ui->cbStop->findData(m["stop"]);
        if(i>=0) ui->cbStop->setCurrentIndex(i);
    }

}

serialPortDialog::~serialPortDialog()
{
    delete ui;
}

QSerialPort *serialPortDialog::get(QObject *parent)
{
    QSerialPort *p=new QSerialPort(parent);
    p->setPortName(ui->cbPorts->currentData().toString());
    p->setBaudRate(ui->cbBaudrates->currentData().toUInt());
    p->setParity((QSerialPort::Parity)ui->cbParity->currentData().toInt());
    p->setDataBits((QSerialPort::DataBits)ui->cbData->currentData().toInt());
    p->setStopBits((QSerialPort::StopBits)ui->cbStop->currentData().toInt());
    return p;
}

void serialPortDialog::on_buttonBox_accepted()
{
    QVariantMap m;
    m["port"] = ui->cbPorts->currentData();
    m["baud"] = ui->cbBaudrates->currentData();
    m["parity"] = ui->cbParity->currentData();
    m["data"] = ui->cbData->currentData();
    m["stop"] = ui->cbStop->currentData();

    configStorage s("serialPortDialog", this);
    s.save(m,"last");
}

