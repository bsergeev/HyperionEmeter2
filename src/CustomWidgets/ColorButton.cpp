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

            pal.setColor(QPalette::Background, m_Color = color);
            setPalette(pal);

            //setPalette(QPalette(m_Color = color));
        }
    }
}

void ColorButton::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
