#pragma once

#include <QListView>

class ImageButton;
class QLineEdit;
class LnkListView : public QListView
{
	Q_OBJECT

public:
	LnkListView(QWidget *parent);
	~LnkListView();

    void setLineEdit(QLineEdit *lineEdit);
	void selectNext();
	void selectPrev();
	void setSelect(int row);
	QModelIndex currentIndex();
	void openIndex(const QModelIndex &index);
	void setEnableButtonsVisible(bool visible);
    bool enableButtonsVisible() const;
    QString getPathFromIndex(const QModelIndex &index) const;
    void openCurrentFolder();

protected:
	void mouseMoveEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);

protected slots:
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private slots :
	void slotFolerOpen();
    void slotShieldOpen();
	void slotItemClicked(const QModelIndex &index);

private:
    QLineEdit *lineEdit_;
	ImageButton *folderOpenBtn_;
    ImageButton *shieldBtn_;
};
