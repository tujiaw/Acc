#include "ClipboardWidget.h"
#include <QMimeData>
#include <QUrl>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QDateTime>
#include "common/Util.h"
#include "component/ElidedLabel.h"

const int MAX_CONTENT_LENGTH = 128;
const int MAX_ROW_COUNT = 100;
ClipboardRowWidget::ClipboardRowWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("ClipboardRowWidget");
    image_ = new QLabel(this);
    content_ = new ElidedLabel(this);
    time_ = new QLabel(this);
    time_->setObjectName("TimeLabel");
    image_->setFixedSize(40, 40);
    time_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    QHBoxLayout *mLayout = new QHBoxLayout();
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(10);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(5);

    rightLayout->addStretch();
    rightLayout->addWidget(content_);
    rightLayout->addWidget(time_);
    rightLayout->addStretch();

    mLayout->addWidget(image_);
    mLayout->addLayout(rightLayout);
    this->setLayout(mLayout);

    this->setFixedHeight(70);
    QStringList style = QStringList() << "#ClipboardRowWidget{border-bottom: 1px solid #fff;}"
        << "#TimeLabel{color:#aaa}";
    this->setStyleSheet(style.join(""));
}

ClipboardRowWidget::~ClipboardRowWidget()
{

}

void ClipboardRowWidget::setData(QMimeData *data)
{
    QPixmap pixmap;
    QString showText;
    QString timeText = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");

    qDebug() << "formats:" << data->formats();
    if (data->hasText()) {
        type_ = T_TEXT;
        data_ = data->text();
        data->setText(data->text());
        pixmap = Util::img("icon_text").scaled(image_->size());
        showText = data->text().mid(0, MAX_CONTENT_LENGTH).replace('\n', ' ');
    } else if (data->hasHtml()) {
        type_ = T_HTML;
        data_ = data->html();
        pixmap = Util::img("icon_html").scaled(image_->size());
        showText = data->html().mid(0, MAX_CONTENT_LENGTH).replace('\n', ' ');
    } else if (data->hasUrls()) {
        QStringList strList;
        auto ls = data->urls();
        for (auto iter = ls.begin(); iter != ls.end(); ++iter) {
            strList.push_back(iter->toString());
        }
        type_ = T_URLS;
        data_ = strList;
        pixmap = Util::img("icon_link").scaled(image_->size());
        showText = strList.join(';');
    } else if (data->hasImage()) {
        type_ = T_IMAGE;
        data_ = data->imageData();
        QImage img = qvariant_cast<QImage>(data->imageData());
        pixmap = QPixmap::fromImage(img).scaled(image_->size());
        if (pixmap.isNull()) {
            pixmap = Util::img("icon_image").scaled(image_->size());
        }
        showText = "[Image]";
    }

    image_->setPixmap(pixmap);
    content_->setText(showText.trimmed());
    time_->setText(timeText);
}

QMimeData* ClipboardRowWidget::getData() const
{
    QMimeData *result = new QMimeData();
    if (type_ == T_TEXT) {
        result->setText(data_.toString());
    } else if (type_ == T_HTML) {
        result->setHtml(data_.toString());
    }  else if (type_ == T_URLS) {
        QList<QUrl> urls;
        QStringList strList = data_.toStringList();
        for (auto iter = strList.begin(); iter != strList.end(); ++iter) {
            urls.push_back(QUrl(*iter));
        }
        result->setUrls(urls);
        qDebug() << "urls:" << strList;
    } else if (type_ == T_IMAGE) {
        result->setImageData(data_);
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
ClipboardWidget::ClipboardWidget(QWidget *parent)
    : QFrame(parent), isSelf_(false)
{
    ui.setupUi(this);
    QClipboard *clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &ClipboardWidget::onDataChanged);
    connect(ui.listWidget, &QListWidget::itemClicked, this, &ClipboardWidget::onItemClicked);
    ui.listWidget->setAlternatingRowColors(true);
    QStringList style = QStringList() << "QListView{ alternate-background-color:rgba(0, 0, 0, 0.2); }"
        << "QListView::item{padding-left:2px;}"
        << "QListView::itemDelegate::selected{ background:rgb(0, 0, 0, 0.4); }";
    ui.listWidget->setStyleSheet(style.join(""));
}

ClipboardWidget::~ClipboardWidget()
{
}

void ClipboardWidget::onDataChanged()
{
    if (isSelf_) {
        isSelf_ = false;
        return;
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    QMimeData *data = const_cast<QMimeData*>(clipboard->mimeData());
    if (!data) {
        qDebug() << "mode is not supported";
        return;
    }
    
    QStringList formats = data->formats();
    for (auto iter = formats.begin(); iter != formats.end(); ++iter) {
        QByteArray a = data->data(*iter);
        qDebug() << a;
    }

    if (ui.listWidget->count() >= MAX_ROW_COUNT) {
        QListWidgetItem *delItem = ui.listWidget->takeItem(ui.listWidget->count() - 1);
        if (delItem) {
            QWidget *delWidget = ui.listWidget->itemWidget(delItem);
            delWidget->deleteLater();
            delete delItem;
        }
    }
    ClipboardRowWidget *widget = new ClipboardRowWidget(this);
    widget->setData(data);
    QListWidgetItem *item = new QListWidgetItem();
    ui.listWidget->setContentsMargins(0, 0, 0, 0);
    ui.listWidget->setSpacing(0);
    item->setSizeHint(QSize(widget->width(), widget->height()));
    ui.listWidget->insertItem(0, item);
    ui.listWidget->setItemWidget(item, widget);
}

void ClipboardWidget::onItemClicked(QListWidgetItem *item)
{
    ClipboardRowWidget *row = static_cast<ClipboardRowWidget*>(ui.listWidget->itemWidget(item));
    if (row) {
        isSelf_ = true;
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setMimeData(row->getData());
    }
}

