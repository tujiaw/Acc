#pragma once

#include <QFrame>

class QLabel;
class QPushButton;
class TitleWidget : public QFrame
{
	Q_OBJECT

public:
	TitleWidget(QWidget *parent = Q_NULLPTR);
	void setTitle(const QString &title);
    void setMinimizeVisible(bool yes);

signals:
	void sigClose();
    void sigMinimize();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
	QLabel *labelTitle_;
	QPushButton *pbClose_;
    QPushButton *pbMinimize_;
};
