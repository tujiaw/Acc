#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class TitleWidget : public QWidget
{
	Q_OBJECT

public:
	TitleWidget(QWidget *parent = Q_NULLPTR);
	void setTitle(const QString &title);

signals:
	void sigClose();

private:
	QLabel *labelTitle_;
	QPushButton *pbClose_;
};
