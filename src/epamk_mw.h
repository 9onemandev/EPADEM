#ifndef EPAMK_MW_H
#define EPAMK_MW_H

#include <QWidget>
#include <QSet>

QT_BEGIN_NAMESPACE
namespace Ui { class EPAMK_MW; }
QT_END_NAMESPACE

class Worker;

class EPAMK_MW : public QWidget
{
    Q_OBJECT

public:
    EPAMK_MW(QWidget *parent = nullptr);
    ~EPAMK_MW();

private slots:
    void findInpFile();
    void run();

private:
    Ui::EPAMK_MW *ui;
    QString lastPath;
    QString *inpData;
    QSet <Worker*> workers;
    int negativeRunCounter;

    bool validateSourceData();
};
#endif // EPAMK_MW_H
