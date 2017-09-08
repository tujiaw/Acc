#include "FramelessWidget.h"
#include <QtWidgets>

FramelessWidget::FramelessWidget(QWidget *parent)
	: QWidget(parent)
{
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Window);
	setMouseTracking(true);

	content_ = new QWidget(this);
	QVBoxLayout *mLayout = new QVBoxLayout(this);
	mLayout->setContentsMargins(0, 0, 0, 0);
	mLayout->setSpacing(0);
	mLayout->addWidget(content_);
}

FramelessWidget::~FramelessWidget()
{

}

void FramelessWidget::setContent(QWidget *content)
{
	this->layout()->replaceWidget(content_, content);
	content_->deleteLater();
	content_ = content;
}

void FramelessWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		isPressed_ = true;
		movePoint_ = event->pos();
	}
	event->ignore();
}

void FramelessWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		isPressed_ = false;
	}
	event->ignore();
}

void FramelessWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (!this->isMaximized() && isPressed_) {
		QPoint distance = event->globalPos() - movePoint_;
		if (distance.manhattanLength() > QApplication::startDragDistance()) {
			this->move(distance);
		}
	}
	event->ignore();
}
