#pragma once

#include <QWidget>
#include <QJsonParseError>
#include "ui_JsonToolWidget.h"

class JsonToolWidget : public QWidget
{
    Q_OBJECT

public:
    JsonToolWidget(QWidget *parent = Q_NULLPTR);
    ~JsonToolWidget();
    void setTips(const QJsonParseError &error);
    void setTips(const QString &text);

private slots:
    void onCheck();
    void onFormat();
    void onCompact();

private:
    Ui::JsonToolWidget ui;
};
