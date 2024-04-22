#ifndef LABEL_H
#define LABEL_H

#include <QLabel>
#include <QObject>
#include <QMouseEvent>

class Label : public QLabel
{
    Q_OBJECT
public:
    Label(QWidget *parent = nullptr);
signals:
    void clicked(const QString &link);
protected:
    void mousePressEvent(QMouseEvent *e);
};

#endif // LABEL_H
