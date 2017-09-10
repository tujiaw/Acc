#pragma once

#include <QListView>

class ImageButton;
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
	void setFolderOpenBtnVisible(bool visible);

protected:
	void mouseMoveEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);

protected slots:
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private slots :
	void slotFolerOpen();
	void slotItemClicked(const QModelIndex &index);

private:
	ImageButton *folderOpenBtn_;
};
