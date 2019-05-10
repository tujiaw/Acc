#pragma once

#include <QFrame>
#include <QClipboard>
#include <memory>
#include "ui_ClipboardWidget.h"

class QLabel;
class ElidedLabel;
class ClipboardRowWidget : public QWidget
{
Q_OBJECT
public:
    enum Type {
        T_TEXT,
        T_HTML,
        T_URLS,
        T_IMAGE,
    };
    ClipboardRowWidget(QWidget *parent = nullptr);
    ~ClipboardRowWidget();
    void setData(QMimeData *data);
    QMimeData* getData() const;
    void updateTime();

private:
    Type type_;
    QVariant data_;
    QLabel *image_;
    QLabel *content_;
    qint64 millisecond_;
    QLabel *time_;
    QLabel *timeInterval_;
};

class ClipboardWidget : public QFrame
{
    Q_OBJECT

public:
    ClipboardWidget(QWidget *parent = Q_NULLPTR);
    ~ClipboardWidget();

signals:
    void sigHide();

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void onDataChanged();
    void onItemClicked(QListWidgetItem *item);
    void onTimer();

private:
    Ui::ClipboardWidget ui;
    bool isSelf_;
    QTimer *timer_;
};
