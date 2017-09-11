#include "TitleWidget.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

TitleWidget::TitleWidget(QWidget *parent)
	: QWidget(parent)
{
	labelTitle_ = new QLabel(this);
	pbMin_ = new QPushButton(this);
	pbClose_ = new QPushButton(this);

	QHBoxLayout *mLayout = new QHBoxLayout(this);
	mLayout->setContentsMargins(0, 0, 0, 0);
	mLayout->setSpacing(0);
	mLayout->addWidget(labelTitle_);
	mLayout->addStretch();
	mLayout->addWidget(pbMin_);
	mLayout->addWidget(pbClose_);

	this->setFixedHeight(40);
}