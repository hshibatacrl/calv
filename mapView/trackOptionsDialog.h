#ifndef TRACKOPTIONDIALOG_H
#define TRACKOPTIONDIALOG_H

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


#include <QDialog>
#include <QVariantMap>

namespace Ui {
class trackOptionsDialog;
}

class trackOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit trackOptionsDialog(QString key,QString pktKey, QStringList hdr, QWidget *parent = nullptr);
    ~trackOptionsDialog();

    QVariantMap opt() const;

    bool eventFilter(QObject *object, QEvent *event);

private:
    void pickColor(QString key);
    void updateToUi(QVariantMap *x=nullptr);
    void loadFromUi(QVariantMap *x=nullptr);
    void updateColorBar(void);
    void normalizeColDsb(void);
    void normalizeFiltDsb(void);

    void loadPresets();

    QString presetFolder();
    QString presetFileName();

private slots:
    void on_pbSavePreset_clicked();
    void on_pbSolidColor_clicked();
    void on_pbMinColor_clicked();
    void on_pbMaxColor_clicked();

    void on_buttonBox_accepted();

    void on_pbMidColor_clicked();

    void on_sbNumOfSteps_valueChanged(int arg1);

    void on_dsColMin_valueChanged(double arg1);

    void on_dsColMid_valueChanged(double arg1);

    void on_dsColMax_valueChanged(double arg1);

    void on_dsFiltMin_valueChanged(double arg1);

    void on_dsFiltMax_valueChanged(double arg1);

    void on_cbPreset_currentIndexChanged(const QString &arg1);

    void on_cbPreset_editTextChanged(const QString &arg1);

    void on_pbDeletePreset_clicked();

private:
    Ui::trackOptionsDialog *ui;

    QStringList _hdr;
    QString _key;
    QString _pktKey;
    QVariantMap _opt;
    QVariantMap _temp;

    int _inhibitNormalizeColDsb;
    int _inhibitNormalizeFiltDsb;
};

#endif // TRACKOPTIONDIALOG_H
