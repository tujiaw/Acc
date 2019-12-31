#include "ToolsWidget.h"
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "ColorToolWidget.h"
#include "TimeToolWidget.h"
#include "JsonToolWidget.h"
#include "UuidToolWidget.h"

ToolsWidget::ToolsWidget(QWidget *parent)
    : QWidget(parent)
{
    m_list = new QListWidget(this);
    m_list->setFixedWidth(100);
    m_list->addItems(QStringList() << "color" << "time" << "json" << "url");
    for (int i = 0; i < m_list->count(); i++) {
        m_list->item(i)->setSizeHint(QSize(this->width(), 30));
    }

    m_stacked = new QStackedWidget(this);
    m_stacked->addWidget(new ColorToolWidget(this));
    m_stacked->addWidget(new TimeToolWidget(this));
    m_stacked->addWidget(new JsonToolWidget(this));
    m_stacked->addWidget(new UuidToolWidget(this));

    QHBoxLayout *mLayout = new QHBoxLayout();
    this->setLayout(mLayout);
    mLayout->addWidget(m_list);
    mLayout->addWidget(m_stacked);

    connect(m_list, SIGNAL(currentRowChanged(int)), m_stacked, SLOT(setCurrentIndex(int)));

    m_list->setCurrentRow(0);

    init();
}

ToolsWidget::~ToolsWidget()
{
}

void ToolsWidget::init()
{

}

