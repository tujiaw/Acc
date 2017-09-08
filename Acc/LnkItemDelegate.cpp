#include "lnkitemdelegate.h"
#include <QPainter>
#include <QDebug>
#include <QStyle>
#include <QApplication>
#include "Constants.h"

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
	painter->drawPixmap(rect.x(), rect.y(), vm["pixmap"].value<QPixmap>());
	
	const int titleVSpace = 6;
	int fontSize = painter->fontInfo().pixelSize();

	rect.setX(LNK_ICON_SIZE.width() + 4 * padding);
	rect.setY(option.rect.y() + (ROW_HEIGHT - 2 * fontSize - titleVSpace) / 2);
	painter->drawText(rect, Qt::AlignLeft, vm["name"].toString());

	rect.setY(rect.y() + fontSize + titleVSpace);
	QString path = vm["path"].toString();
	path = painter->fontMetrics().elidedText(path, Qt::TextElideMode::ElideMiddle, rect.width());
	painter->drawText(rect, Qt::AlignLeft, path);

	painter->restore();
	
}

QSize LnkItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int width = option.rect.width();
	return QSize(width, ROW_HEIGHT);
}
