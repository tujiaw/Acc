#pragma once

#include <QFrame>

class FramelessWidget : public QFrame
{
	Q_OBJECT

public:
	FramelessWidget(QWidget *parent = Q_NULLPTR);
	~FramelessWidget();
	void setContent(QWidget *content);

protected:
	void resizeEvent(QResizeEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	bool nativeEvent(const QByteArray & eventType, void * message, long * result);

private:
	QPoint movePoint_;
	bool isPressed_;
	QWidget *content_;
};
