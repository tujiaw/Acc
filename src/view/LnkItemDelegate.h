#ifndef LNKITEMDELEGATE_H
#define LNKITEMDELEGATE_H

#include <QStyledItemDelegate>

class LnkItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	LnkItemDelegate(QObject *parent);
	~LnkItemDelegate();

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // LNKITEMDELEGATE_H
