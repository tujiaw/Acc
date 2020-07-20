#include "lnkitemdelegate.h"
#include <QPainter>
#include <QDebug>
#include <QStyle>
#include <QApplication>
#include "common/Constants.h"

static QString s_searchText;
LnkItemDelegate::LnkItemDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{

}

LnkItemDelegate::~LnkItemDelegate()
{

}

void LnkItemDelegate::setSearchText(const QString &text)
{
	s_searchText = text;
}

void LnkItemDelegate::drawHighlightText(QPainter *painter, QRect rect, const QString &text, const QString &highlightText,
	const QColor &color, const QColor &highlightColor) const
{
	if (!text.isEmpty() && !highlightText.isEmpty()) {
		int pos = text.indexOf(highlightText, 0, Qt::CaseInsensitive);
		if (pos >= 0) {
			QString left = text.mid(0, pos);
			QString middle = highlightText;
			QString right = text.mid(pos + highlightText.size());

			painter->setPen(color);
			painter->drawText(rect, left);

			painter->setPen(highlightColor);
			rect.setX(rect.x() + painter->fontMetrics().width(left));
			painter->drawText(rect, middle);

			painter->setPen(color);
			rect.setX(rect.x() + painter->fontMetrics().width(middle));
			painter->drawText(rect, right);
			return;
		}
	}
	painter->setPen(color);
	painter->drawText(rect, Qt::AlignLeft, text);
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
	painter->setPen(QColor("#fff"));
	rect.setX(LNK_ICON_SIZE.width() + 2 * padding);
	painter->drawText(rect, Qt::AlignLeft, vm["name"].toString());
	//this->drawHighlightText(painter, rect, vm["lnkName"].toString(), s_searchText, QColor("#fff"), QColor("#ffcc00"));

	const int titleVSpace = 4;
	rect.setY(rect.y() + font.pixelSize() + titleVSpace);
	font.setPixelSize(14);
	painter->setFont(font);

	QString path = vm["path"].toString();
	path = painter->fontMetrics().elidedText(path, Qt::TextElideMode::ElideMiddle, rect.width());
	painter->setPen(QColor("#c5c5c5"));
	painter->drawText(rect, Qt::AlignLeft, path);
	//this->drawHighlightText(painter, rect, path, s_searchText, QColor("#c5c5c5"), QColor("#ffcc00"));

	painter->restore();
}

QSize LnkItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int width = option.rect.width();
	return QSize(width, ROW_HEIGHT);
}
