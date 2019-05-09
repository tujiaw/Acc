#include "ImageListDlg.h"
#include "common/Util.h"
#include <QScrollArea>
#include <QTimer>

ImageWidget::ImageWidget(QWidget *parent)
        : QLabel(parent)
{
    this->setBackgroundRole(QPalette::Base);
    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    this->setScaledContents(true);
    connect(this, &ImageWidget::sigData, this, &ImageWidget::onData);
}

void ImageWidget::setUrl(const QString &url)
{
    url_ = url;
    QString name = getName(url);
    QString filepath = Util::getImagesDir() + "/" + name;
    QDir dir(Util::getImagesDir());
    if (dir.exists(name)) {
        pixmap_ = QPixmap(filepath);
        resizePixmap();
    } else {
        HttpRequest *request = new HttpRequest();
        connect(request, &HttpRequest::sigHttpResponse, [=](int err, const QByteArray &data) {
            if (err == 0) {
                emit sigData(data);
            }
            request->deleteLater();
        });
        request->get(url);
    }
}

QString ImageWidget::getUrl() const
{
    return url_;
}

QString ImageWidget::getName(const QString &url)
{
    return Util::md5(url) + ".jpg";
}

void ImageWidget::onData(const QByteArray &data)
{
    if (data.isEmpty()) {
        qDebug() << "data is empty, url:" << url_;
        return;
    }

    QString name = getName(url_);
    QString filepath = Util::getImagesDir() + "/" + name;
    QFile f(filepath);
    f.open(QIODevice::WriteOnly);
    QDataStream out(&f);
    out.writeRawData(data.constData(), data.size());
    f.flush();

    QPixmap pixmap(filepath);
    if (!pixmap.isNull() && pixmap.width() > 0) {
        pixmap_ = pixmap;
        resizePixmap();
    } else {
        qDebug() << data;
    }
}

void ImageWidget::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    resizePixmap();
}

void ImageWidget::resizePixmap()
{
    if (pixmap_.width() > 0) {
        int width = this->width();
        int height = (width * 1.0 / pixmap_.width()) * pixmap_.height();
        this->setPixmap(pixmap_.scaled(width, height));
    }
}

ImageListDlg::ImageListDlg(QWidget *parent)
    : QDialog(parent)
{
    content_ = new QScrollArea(this);
    mLayout_ = new QVBoxLayout(content_);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(content_);
    this->setLayout(layout);
}

void ImageListDlg::setImageUrlList(const QStringList &urlList)
{
    urlList_ = urlList_;
    foreach(const QString &url, urlList) {
        ImageWidget *widget = new ImageWidget(this);
        widget->setUrl(url);
        mLayout_->addWidget(widget);
    }
}

