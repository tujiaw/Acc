#pragma once

#include <QWidget>
#include "ui_ColorToolWidget.h"

class ColorToolWidget : public QWidget
{
    Q_OBJECT

public:
    ColorToolWidget(QWidget *parent = Q_NULLPTR);
    ~ColorToolWidget();

protected:
    void keyPressEvent(QKeyEvent *);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

private:
    QString clr2rgb(QColor clr);
    QString clr2hex(QColor clr);
    void clrShow(QColor clr, const QString &str = "");

private slots:
    void on_leRgb_returnPressed();
    void on_leHex_returnPressed();
    void on_pbPickColorStart_clicked();
    void on_pbPickColorStop_clicked();
    void slotTimer();

private:
    Ui::ColorToolWidget ui;
    QTimer *timer_;
};
