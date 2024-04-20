#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include <QObject>

class Worker : public QThread
{
    Q_OBJECT;
public:
    explicit Worker(const int &_id,
                    const QString *_inpData,
                    const int &_simNum,
                    const float &_top,
                    const float &_bottom,
                    const float &_minPressure,
                    const bool &_rmtmp,
                    QObject *parent = nullptr);

protected:
    void run() override;

signals:
    void iterDone();
    void negativeResult();

private:
    int id;
    const QString *inpData;
    int simNum;
    float top;
    float bottom;
    float minPressure;
    bool rmtmp;
};

#endif // WORKER_H
