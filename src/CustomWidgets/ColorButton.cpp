#include "ColorButton.h"

#include <QPainter>
#include <QPalette>
#include <QColorDialog>
#include <QMouseEvent>
#include <QStyleOption>

ColorButton::ColorButton(QColor color, QWidget* parent)
            : QPushButton(parent)
            , m_Color(color)
{
    setMaximumWidth(height());

    setAutoFillBackground(true);
    setPalette(color);
}

void ColorButton::mousePressEvent(QMouseEvent* event)
{
    if (event && event->button() == Qt::LeftButton)
    {
        QPalette pal = palette();
        QColor color = QColorDialog::getColor(pal.color(QPalette::Window), this);
        if (color.isValid()) {
			setPalette(QPalette(m_Color = color));
        } else {
			Q_ASSERT(!"Invalid color");
		}
    }
}

void ColorButton::paintEvent(QPaintEvent*)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
