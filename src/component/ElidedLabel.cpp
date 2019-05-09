#include "elidedlabel.h"
#include <QtWidgets>

ElidedLabel::ElidedLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    this->setMinimumWidth(0);
    setTextFormat(Qt::PlainText);
}

ElidedLabel::ElidedLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f), m_fullText(text)
{
    this->setMinimumWidth(0);
    setTextFormat(Qt::PlainText);
}

void ElidedLabel::setPaddingBlank(int blankCount)
{
    for (int i = 0; i < blankCount; i++) {
        m_paddingBlank.append(" ");
    }
}

void ElidedLabel::setText(const QString &text)
{
    setFullText(text);
}

void ElidedLabel::setFullText(const QString &text)
{
    m_fullText = text;
    update();
}

void ElidedLabel::setTextLimitShrink(const QString &text, int width)
{
    this->setMinimumWidth(qMin(this->fontMetrics().width(text), width));
    setFullText(text);
}

void ElidedLabel::setTextLimitExpand(const QString &text)
{
    int textWidth = this->fontMetrics().width(text);
    this->setMaximumWidth(textWidth);
    setFullText(text);
}

QString ElidedLabel::fullText() const
{
    return m_fullText;
}

void ElidedLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    elideText();
}

void ElidedLabel::elideText()
{
    QFontMetrics fm = this->fontMetrics();
    int dif = fm.width(m_fullText) - this->width();
    if (dif > 0) {
        QString showText = fm.elidedText(m_fullText, Qt::ElideRight, this->width());
        QLabel::setText(showText + m_paddingBlank);
        if (showText != m_fullText) {
            this->setToolTip(m_fullText.left(1024));
        } else {
            this->setToolTip("");
        }
    } else {
        QLabel::setText(m_fullText);
        this->setToolTip("");
    }
}

