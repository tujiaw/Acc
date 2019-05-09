#pragma once

#include <QObject>

class Cleaner : public QObject
{
    Q_OBJECT

public:
    Cleaner(QObject *parent);
    ~Cleaner();
};
