#pragma once

#include <QListView>

class LnkListView : public QListView
{
	Q_OBJECT

public:
	LnkListView(QWidget *parent);
	~LnkListView();
};
