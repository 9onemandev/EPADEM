#include "epamk_mw.h"
#include "ui_epamk_mw.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QThread>
#include <QDebug>
#include "worker.h"
#include "math.h"

EPAMK_MW::EPAMK_MW(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EPAMK_MW),
      inpData(new QString),
      negativeRunCounter(0)
{
    ui->setupUi(this);

    //get max num of thread available from hardware
    int atv = QThread::idealThreadCount();
    //leave 1 thread available just to don`t freeze machine while modelling
    if(atv > 1)
    {
        //but user can increase it to max value from gui
        ui->tc_spin->setMaximum(atv);
        atv -= 1;
        ui->tc_spin->setValue(atv);
    }
    else
        ui->tc_spin->setMaximum(atv);

    ui->sn_spin->setValue(1000);
    ui->progressBar->hide();

    //actions
    connect(ui->inp, &QPushButton::clicked, this, &EPAMK_MW::findInpFile);
    connect(ui->run, &QPushButton::clicked, this, &EPAMK_MW::run);
}

EPAMK_MW::~EPAMK_MW()
{
    delete inpData;
    delete ui;
}

void EPAMK_MW::findInpFile()
{
    QString fpath = QFileDialog::getOpenFileName(this, tr("Network input file"), lastPath, tr("Format (*.inp)"));
    lastPath = QDir(fpath).path();
    ui->inp_path_le->setText(fpath);
    ui->inp_path_le->repaint();
}

bool EPAMK_MW::validateSourceData()
{
    if(ui->inp_path_le->text().isEmpty())
    {
        QMessageBox mb;
        mb.setText(tr("INP file not specified"));
        mb.exec();
        return false;
    }

    QString epaPath = qApp->applicationDirPath() + "/runepanet.exe";
    if(!QFile::exists(epaPath))
    {
        QMessageBox mb;
        mb.setText(tr("Program should be placed in Epanet exe folder or Epanet2w.exe doesn`t exist"));
        mb.exec();
        return false;
    }

    QString reportTemplate =
    "[REPORT]\n"
    "Status No\n"
    "Summary No\n"
    "NODES All\n"
    "LINKS All\n"
    "FLOW PRECISION 4\n"
    "ELEVATION PRECISION 4\n"
    "HEAD PRECISION 4\n"
    "PRESSURE PRECISION 4\n"
    "Page 0";

    QString reportTemplate_v2 =
    "[REPORT]\r\nStatus No\r\n"
    "Summary No\r\n"
    "NODES All\r\n"
    "LINKS All\r\n"
    "FLOW PRECISION 4\r\n"
    "ELEVATION PRECISION 4\r\n"
    "HEAD PRECISION 4\r\n"
    "PRESSURE PRECISION 4\r\n"
    "Page 0";

    QFile file(ui->inp_path_le->text());
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox mb;
        mb.setText(tr("Failed to open specified INP file"));
        mb.exec();
        return false;
    }
    else
    {
        inpData->clear();

        QTextStream io(&file);
        *inpData = io.readAll();
        file.flush();
        file.close();

        //qDebug() << "[INPUT DATA]";
        //qDebug() << *inpData;

        if(!inpData->contains(reportTemplate) && !inpData->contains(reportTemplate_v2))
        {
            QMessageBox mb;
            mb.setText(tr("INP file has wrong report template.\nFix it according to README file and rerun modeling"));
            mb.exec();
            return false;
        }
    }

    return true;
}

void EPAMK_MW::run()
{
    if(validateSourceData())
    {
        qDebug() << "INP data is correct. Can run epanet series";

        //show progress info gui
        ui->progressBar->setValue(0);
        ui->progressBar->setMaximum(ui->sn_spin->value());
        ui->progressBar->show();

        //init threads
        negativeRunCounter = 0;
        int runsPerThread = std::floor(ui->sn_spin->value() / ui->tc_spin->value());
        qDebug() << "runsPerThread" << runsPerThread;
        for(int i = 0; i < ui->tc_spin->value(); i++)
        {
            if(i == ui->tc_spin->value() - 1)
            {
                //condition to fit full number of necessary runs (1000 / 3 case for example)
                if(ui->sn_spin->value() != runsPerThread * ui->tc_spin->value())
                {
                    runsPerThread += ui->sn_spin->value() - runsPerThread * ui->tc_spin->value();
                    qDebug() << "last iter correction" << runsPerThread;
                }
            }

            Worker *worker = new Worker(i, inpData, runsPerThread, ui->td_spin->value(), ui->bd_spin->value(), ui->mp_spin->value(), ui->checkBox->isChecked());

            //update progress
            connect(worker, &Worker::iterDone, this, [=](){
                ui->progressBar->setValue(ui->progressBar->value() + 1);
            });

            //negative run counter
            connect(worker, &Worker::negativeResult, this, [=](){
                negativeRunCounter++;
            });

            //finish thread
            connect(worker, &Worker::finished, this, [=](){
                workers.remove(static_cast <Worker*> (sender()));
                sender()->deleteLater();

                if(workers.isEmpty())
                {
                    //show final results
                    int positiveRunCounter = ui->sn_spin->value() - negativeRunCounter;
                    double success = positiveRunCounter * 100 / ui->sn_spin->value();

                    setEnabled(true);
                    ui->progressBar->hide();

                    QMessageBox mb;
                    mb.setText(tr("Network configuration success rate is %1 %").arg(success));
                    mb.exec();
                }
            });

            workers.insert(worker);
            worker->start();
        }

        setEnabled(false);
    }
}
