#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class TitleWidget : public QWidget
{
	Q_OBJECT

public:
	TitleWidget(QWidget *parent = Q_NULLPTR);

private:
	QLabel *labelTitle_;
	QPushButton *pbMin_;
	QPushButton *pbClose_;
};
