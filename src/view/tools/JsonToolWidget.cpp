#include "JsonToolWidget.h"
#include <QJsonDocument> 

JsonToolWidget::JsonToolWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    connect(ui.pbCheck, &QPushButton::clicked, this, &JsonToolWidget::onCheck);
    connect(ui.pbFormat, &QPushButton::clicked, this, &JsonToolWidget::onFormat);
    connect(ui.pbCompact, &QPushButton::clicked, this, &JsonToolWidget::onCompact);
    ui.teJsonText->setAcceptRichText(false);
    ui.labelTips->hide();
}

JsonToolWidget::~JsonToolWidget()
{
    
}

void JsonToolWidget::setTips(const QJsonParseError &error)
{
    if (error.error == QJsonParseError::NoError) {
        ui.labelTips->setText("");
    } else {
        ui.labelTips->setText(QStringLiteral("提示：") + error.errorString());
    }
    ui.labelTips->setVisible(!ui.labelTips->text().isEmpty());
}

void JsonToolWidget::setTips(const QString &text)
{
    ui.labelTips->setText(text);
    ui.labelTips->setVisible(!ui.labelTips->text().isEmpty());
}

void JsonToolWidget::onCheck()
{
    QString text = ui.teJsonText->toPlainText().trimmed();
    if (text.isEmpty()) {
        return;
    }

    QJsonParseError error;
    QJsonDocument::fromJson(text.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError) {
        setTips(QStringLiteral("提示：校验正确"));
    } else {
        setTips(error);
    }
}

void JsonToolWidget::onFormat()
{
    QString text = ui.teJsonText->toPlainText().trimmed();
    if (text.isEmpty()) {
        return;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
    ui.labelTips->setVisible(!doc.isNull());
    if (!doc.isNull()) {
        ui.teJsonText->setPlainText(doc.toJson(QJsonDocument::Indented));
    }
    setTips(error);
}

void JsonToolWidget::onCompact()
{
    QString text = ui.teJsonText->toPlainText().trimmed();
    if (text.isEmpty()) {
        return;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
    ui.labelTips->setVisible(!doc.isNull());
    if (!doc.isNull()) {
        ui.teJsonText->setPlainText(doc.toJson(QJsonDocument::Compact));
    }
    setTips(error);
}
