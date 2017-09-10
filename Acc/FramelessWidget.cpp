#include "FramelessWidget.h"
#include <QtWidgets>

FramelessWidget::FramelessWidget(QWidget *parent)
	: QFrame(parent)
{
	setMouseTracking(true);
	setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
	//setAttribute(Qt::WA_NoSystemBackground, true);

	content_ = new QWidget(this);
	QVBoxLayout *mLayout = new QVBoxLayout(this);
	mLayout->setContentsMargins(0, 0, 0, 0);
	mLayout->setSpacing(0);
	mLayout->addWidget(content_);
	this->setObjectName("FramelessWidget");
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

void FramelessWidget::resizeEvent(QResizeEvent *event)
{
	QFrame::resizeEvent(event);
	int radius = 5;
	QSize maskSize(this->size().width(), this->size().height() + radius);
	QBitmap mask(maskSize);
	QPainter maskPainter(&mask);
	maskPainter.setRenderHint(QPainter::Antialiasing);
	maskPainter.setRenderHint(QPainter::SmoothPixmapTransform);
	QColor color = QColor("#FFFFFF");
	maskPainter.fillRect(this->rect(), color);
	maskPainter.setBrush(QColor("#000000"));
	maskPainter.drawRoundedRect(QRect(QPoint(0, 0), maskSize), radius, radius);
	this->setMask(mask);
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

bool FramelessWidget::nativeEvent(const QByteArray & eventType, void * message, long * result)
{
	const MSG *msg = static_cast<MSG*>(message);
	if (!msg) {
		return false;
	}

	if (msg->message == WM_LBUTTONUP) {
		isPressed_ = false;
	}
	
	return false;
}
