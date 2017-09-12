#include "lnkitemdelegate.h"
#include <QPainter>
#include <QDebug>
#include <QStyle>
#include <QApplication>
#include "common/Constants.h"

LnkItemDelegate::LnkItemDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{

}

LnkItemDelegate::~LnkItemDelegate()
{

}

void LnkItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	painter->save();
	int row = index.row();
	if (option.state & QStyle::State_Selected) {
		painter->fillRect(option.rect, option.palette.highlight());
	}
	
	QRect rect = option.rect;
	int padding = (rect.height() - LNK_ICON_SIZE.height()) / 2;
	rect.setX(rect.x() + padding);
	rect.setY(rect.y() + padding);

	QVariantMap vm = index.data().toMap();
	QIcon icon = vm["icon"].value<QIcon>();
	QRect iconRect(rect.x(), rect.y(), LNK_ICON_SIZE.width(), LNK_ICON_SIZE.height());
	icon.paint(painter, iconRect);

	QFont font = painter->font();
	font.setPixelSize(16);
	painter->setFont(font);
	rect.setX(LNK_ICON_SIZE.width() + 4 * padding);
	painter->drawText(rect, Qt::AlignLeft, vm["lnkName"].toString());

	const int titleVSpace = 4;
	rect.setY(rect.y() + font.pixelSize() + titleVSpace);
	font.setPixelSize(14);
	painter->setPen(QColor("#eee"));
	painter->setFont(font);
	QString path = vm["targetPath"].toString();
	path = painter->fontMetrics().elidedText(path, Qt::TextElideMode::ElideMiddle, rect.width());
	painter->drawText(rect, Qt::AlignLeft, path);
	painter->restore();
}

QSize LnkItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int width = option.rect.width();
	return QSize(width, ROW_HEIGHT);
}