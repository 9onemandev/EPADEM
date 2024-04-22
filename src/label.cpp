#include "label.h"

Label::Label(QWidget *parent) : QLabel(parent)
{}

void Label::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)

    QStringList l1 = text().split("www");
    QString sp = "www" + l1.last();
    QStringList l2 = sp.split("\n");

    emit clicked(l2.first());
}
