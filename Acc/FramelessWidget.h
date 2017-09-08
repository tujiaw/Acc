#pragma once

#include <QWidget>

class FramelessWidget : public QWidget
{
	Q_OBJECT

public:
	FramelessWidget(QWidget *parent = Q_NULLPTR);
	~FramelessWidget();
	void setContent(QWidget *content);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

private:
	QPoint movePoint_;
	bool isPressed_;
	QWidget *content_;
};
