#pragma once

#include <QListView>

class LnkListView : public QListView
{
	Q_OBJECT

public:
	LnkListView(QWidget *parent);
	~LnkListView();
	void selectNext();
	void selectPrev();
	void setSelect(int row);
	QModelIndex currentIndex();
	void openIndex(const QModelIndex &index);

protected:
	void mouseMoveEvent(QMouseEvent *e);
	
private slots :
	void slotItemPressed(const QModelIndex &index);
};
