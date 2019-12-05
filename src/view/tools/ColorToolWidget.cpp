#include "ColorToolWidget.h"
#include <QtWidgets>
#include <QScreen>

ColorToolWidget::ColorToolWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    timer_ = new QTimer(this);
    connect(timer_, SIGNAL(timeout()), this, SLOT(slotTimer()));
}

ColorToolWidget::~ColorToolWidget()
{
}

void ColorToolWidget::keyPressEvent(QKeyEvent *e)
{
    QWidget::keyPressEvent(e);
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->key() == Qt::Key_S) {
            emit ui.pbPickColorStart->clicked();
            ui.pbPickColorStart->setFocus();
        } else if (e->key() == Qt::Key_D) {
            emit ui.pbPickColorStop->clicked();
            ui.pbPickColorStop->setFocus();
        }
    }
}

void ColorToolWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    this->grabKeyboard();
}

void ColorToolWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    this->releaseKeyboard();
}

void ColorToolWidget::on_leRgb_returnPressed()
{
    bool isOk = false;
    QString str = ui.leRgb->text();
    QStringList strList = str.split(',');
    if (strList.count() >= 3) {
        int r = strList[0].toInt();
        int g = strList[1].toInt();
        int b = strList[2].toInt();
        if ((r <= 255 && r >= 0) && (g <= 255 && g >= 0) && (b <= 255 && b >= 0)) {
            isOk = true;
            QColor clr(r, g, b);
            ui.leHex->setText(clr2hex(clr));
            clrShow(clr);
        }
    }

    if (!isOk) {
        QMessageBox::information(this, tr("show"), tr("RGB(255,255,255),格式错误!"));
    }
}

void ColorToolWidget::on_leHex_returnPressed()
{
    QString str = ui.leHex->text();
    if (str.count() == 6) {
        QColor clr("#" + str);
        int r, g, b;
        clr.getRgb(&r, &g, &b);
        ui.leRgb->setText(QString("%1,%2,%3").arg(r).arg(g).arg(b));
        clrShow(clr);
    } else {
        QMessageBox::information(this, tr("show"), tr("#ffffff,格式错误!"));
    }
}

QString ColorToolWidget::clr2rgb(QColor clr)
{
    int r, g, b;
    clr.getRgb(&r, &g, &b);
    QString xx = QString("%1,%2,%3").arg(r).arg(g).arg(b);
    return xx;
}

QString ColorToolWidget::clr2hex(QColor clr)
{
    int r, g, b;
    clr.getRgb(&r, &g, &b);
    QString xx = QString("").sprintf("%02x%02x%02x", r, g, b);
    return xx;
}

void ColorToolWidget::clrShow(QColor clr, const QString &str)
{
    int r, g, b;
    clr.getRgb(&r, &g, &b);
    QColor fontClr(255 - r, 255 - g, 255 - b);
    if (str.isEmpty()) {
        ui.laColor->setText("RGB(" + clr2rgb(clr) + ")" + " #" + clr2hex(clr));
    } else {
        ui.laColor->setText(str);
    }
    ui.laColor->setStyleSheet(QString("color:#%1;background-color:#%2").arg(clr2hex(fontClr)).arg(clr2hex(clr)));
}

void ColorToolWidget::on_pbPickColorStart_clicked()
{
    ui.leRgb->setEnabled(false);
    ui.leHex->setEnabled(false);
    timer_->start(100);
}

void ColorToolWidget::on_pbPickColorStop_clicked()
{
    ui.leRgb->setEnabled(true);
    ui.leHex->setEnabled(true);
    timer_->stop();
}

void ColorToolWidget::slotTimer()
{
    QPoint pos = QCursor::pos();
    QScreen *screen = QGuiApplication::primaryScreen();
    QPixmap pixmap = screen->grabWindow(QApplication::desktop()->winId(), pos.x(), pos.y(), 1, 1);
    if (!pixmap.isNull()) {
        QImage image = pixmap.toImage();
        if (!image.isNull()) {
            if (image.valid(0, 0)) {
                QColor clr = image.pixel(0, 0);
                ui.leRgb->setText(clr2rgb(clr));
                ui.leHex->setText(clr2hex(clr));
                clrShow(clr, QString("X:%1, Y:%2").arg(pos.x()).arg(pos.y()));
            }
        }
    }

    QPixmap magnifyPixmap = screen->grabWindow(QApplication::desktop()->winId(), pos.x() - 20, pos.y() - 20, 40, 40);
    if (!magnifyPixmap.isNull()) {
        int magnifySize = 160;
        magnifyPixmap = magnifyPixmap.scaled(magnifySize, magnifySize);
        QPainter painter(&magnifyPixmap);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#696969"));
        painter.drawRect(0, magnifySize / 2, magnifySize, 3);
        painter.drawRect(magnifySize / 2, 0, 3, magnifySize);
        painter.setBrush(QColor("#ffffff"));
        painter.drawRect(magnifySize / 2, magnifySize / 2, 3, 3);
        ui.magnifyLabel->setPixmap(magnifyPixmap);
    }
}
