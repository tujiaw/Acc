#pragma once

#include <QWidget>
#include "ui_UuidToolWidget.h"

class UuidToolWidget : public QWidget
{
    Q_OBJECT

public:
    UuidToolWidget(QWidget *parent = Q_NULLPTR);
    ~UuidToolWidget();

private:
    Ui::UuidToolWidget ui;
};
