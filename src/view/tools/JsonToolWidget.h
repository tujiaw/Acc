#pragma once

#include <QWidget>
#include "ui_JsonToolWidget.h"

class JsonToolWidget : public QWidget
{
    Q_OBJECT

public:
    JsonToolWidget(QWidget *parent = Q_NULLPTR);
    ~JsonToolWidget();

private:
    Ui::JsonToolWidget ui;
};
