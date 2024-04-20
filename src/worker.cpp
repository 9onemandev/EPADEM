#include "worker.h"

#include <QCoreApplication>
#include <QTextStream>
#include <QProcess>
#include <random>
#include <time.h>
#include <thread>
#include <QFile>
#include <QDebug>

Worker::Worker(const int &_id,
               const QString *_inpData,
               const int &_simNum,
               const float &_top,
               const float &_bottom,
               const float &_minPressure,
               const bool &_rmtmp,
               QObject *parent)
    : QThread{parent},
      id(_id),
      inpData(_inpData),
      simNum(_simNum),
      top(_top),
      bottom(_bottom),
      minPressure(_minPressure),
      rmtmp(_rmtmp)
{}

void Worker::run()
{
    std::mt19937 engine(clock());
    QString epaPath = qApp->applicationDirPath() + "/" + "runepanet.exe";

    for(int i = 0; i < simNum; i++)
    {
        QString newInpFilePath = qApp->applicationDirPath() + "/" + QString("%1_%2.inp").arg(id).arg(i);
        QString reportFilePath = qApp->applicationDirPath() + "/" + QString("%1_%2.rpt").arg(id).arg(i);

        //create new INP file with GE modified values
        QString inpTemp = *inpData;
        QString newInp;

        QStringList junctions_part = inpTemp.split("[JUNCTIONS]");
        newInp = junctions_part[0] + "[JUNCTIONS]\n";

        QStringList ge_part = junctions_part.last().split("\n");

        for(int j = 0; j < ge_part.size(); j++)
        {
            if(ge_part[j][0] == '[')
            {
                //add rest part of original file and go to modeling
                for(int k = j; k < ge_part.size(); k++)
                    newInp += ge_part[k] + "\n";
                break;
            }

            if(ge_part[j][0] != ';' && ge_part[j].size() > 2)
            {
                QStringList id_ge;

                if(ge_part[j].contains('\t'))
                    id_ge = ge_part[j].split('\t');
                else
                    id_ge = ge_part[j].split(' ');

                // id_ge[0] is node ID
                // id_ge[1] is node GE

                // get new GE value using random range
                std::uniform_real_distribution <double> uniform_dist(id_ge[1].toDouble() - bottom, id_ge[1].toDouble() + top);
                double new_ge = uniform_dist(engine);

                ge_part[j].replace(id_ge[1], QString::number(new_ge) + " ");
                newInp += ge_part[j] + "\n";
            }
            else
                newInp += ge_part[j] + "\n";
        }

        QFile file(newInpFilePath);
        file.open(QIODevice::ReadWrite);
        QTextStream io(&file);
        io << newInp;
        file.flush();
        file.close();

        //run epanet process
        QProcess *process = new QProcess;

        //actions in case success run and failed run
        connect(this, &Worker::iterDone, process, &QProcess::deleteLater);
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
            Q_UNUSED(exitStatus)

            if(exitCode != 0)
            {
                //rerun
                qDebug() << id << i << "status" <<  exitCode;

                QStringList rerun_arguments;
                rerun_arguments << newInpFilePath << reportFilePath;
                process->start(epaPath, rerun_arguments);
                if(process->waitForStarted())
                {
                    if(process->waitForFinished());
                    else
                    {
                        emit negativeResult();
                        emit iterDone();
                    }
                }
            }
            else
            {
                //check report
                QFile report(reportFilePath);
                report.open(QIODevice::ReadWrite);
                QTextStream rio(&report);
                QString rpt = rio.readAll();
                report.flush();
                report.close();

                QStringList rptParts = rpt.split("--------------------------------------------------------\r\n");
                QStringList nodeParts = rptParts[2].split("Link Results:\r\n");
                QStringList pureNodeData = nodeParts[0].split("\r\n");

                for(int k = 0; k < pureNodeData.size(); k++)
                {
                    QString pressure = pureNodeData[k].split(' ').last();
                    bool isNumber;
                    float pvalue = pressure.toFloat(&isNumber);

                    if(isNumber && !pressure.isEmpty())
                    {
                        if(pvalue < minPressure)
                        {
                            qDebug() << "Negative pressure" << id << i << pvalue;
                            emit negativeResult();
                            break;
                        }
                    }
                }

                //delete temp files
                if(!rmtmp)
                {
                    QFile::remove(newInpFilePath);
                    QFile::remove(reportFilePath);
                }

                emit iterDone();
            }
        });

        QStringList arguments;
        arguments << newInpFilePath << reportFilePath;
        process->start(epaPath, arguments);
        if(process->waitForStarted())
        {
            if(process->waitForFinished());
            else
            {
                emit negativeResult();
                emit iterDone();
            }
        }
    }
}
