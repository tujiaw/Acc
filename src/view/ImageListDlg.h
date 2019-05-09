#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include "common/HttpRequest.h"

class ImageWidget : public QLabel
{
    Q_OBJECT
public:
    ImageWidget(QWidget *parent = nullptr);
    void setUrl(const QString &url);
    QString getUrl() const;
    static QString getName(const QString &url);

signals:
    void sigData(const QByteArray &data);

private slots:
    void onData(const QByteArray &data);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void resizePixmap();

private:
    QString url_;
    QPixmap pixmap_;
};

class ImageListDlg : public QDialog
{
    Q_OBJECT
public:
    ImageListDlg(QWidget *parent = nullptr);
    void setImageUrlList(const QStringList &urlList);

private:
    QVBoxLayout *mLayout_;
    class QScrollArea *content_;
    QStringList urlList_;
};