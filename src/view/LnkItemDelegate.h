#ifndef LNKITEMDELEGATE_H
#define LNKITEMDELEGATE_H

#include <QStyledItemDelegate>

class LnkItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	LnkItemDelegate(QObject *parent);
	~LnkItemDelegate();
	static void setSearchText(const QString &text);
	void drawHighlightText(QPainter *painter, QRect rect, const QString &text, const QString &highlightText, 
		const QColor &color, const QColor &highlightColor) const;

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // LNKITEMDELEGATE_H
