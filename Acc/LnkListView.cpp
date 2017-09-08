#include "LnkListView.h"
#include <QMouseEvent>
#include <QProcess>
#include <QDebug>

LnkListView::LnkListView(QWidget *parent)
	: QListView(parent)
{
	this->setAlternatingRowColors(true);
	this->setMouseTracking(true); // ����mouse over��ʽ��������
	this->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(this, &QListView::pressed, this, &LnkListView::slotItemPressed);
}

LnkListView::~LnkListView()
{
}

void LnkListView::selectNext()
{
	int nextRow = 0;
	QModelIndex index = currentIndex();
	if (index.isValid() && index.row() + 1 < this->model()->rowCount()) {
		nextRow = index.row() + 1;
	}
	setSelect(nextRow);
}

void LnkListView::selectPrev()
{
	int prevRow = this->model()->rowCount() - 1;
	QModelIndex index = currentIndex();
	if (index.isValid() && index.row() - 1 >= 0) {
		prevRow = index.row() - 1;
	}
	setSelect(prevRow);
}

void LnkListView::setSelect(int row)
{
	QModelIndex index = this->model()->index(row, 0);
	if (index.isValid()) {
		this->selectionModel()->clear();
		this->selectionModel()->select(index, QItemSelectionModel::Select);
		this->scrollTo(index);
	}
}

QModelIndex LnkListView::currentIndex()
{
	QModelIndexList indexList = this->selectionModel()->selectedRows();
	return indexList.size() > 0 ? indexList[0] : QModelIndex();
}

void LnkListView::openIndex(const QModelIndex &index)
{
	if (!index.isValid()) {
		return;
	}

	qDebug() << index.row();
	QVariantMap vm = this->model()->data(index).toMap();
	QString path = vm["path"].toString();
	if (!path.isEmpty()) {
		bool ok = QProcess::startDetached("\"" + path + "\"");
		qDebug() << path << ",ok:" << ok;
	}
}

void LnkListView::mouseMoveEvent(QMouseEvent *e)
{
	QModelIndex index = this->indexAt(e->pos());
	if (index.isValid()) {
		this->selectionModel()->clear();
		this->selectionModel()->select(index, QItemSelectionModel::Select);
	}
}

void LnkListView::slotItemPressed(const QModelIndex &index)
{
	openIndex(index);
}
