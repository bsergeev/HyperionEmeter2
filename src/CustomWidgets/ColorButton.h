#pragma once

#include <QPushButton>
#include <QColor>

class QMouseEvent;
class QPaintEvent;

class ColorButton : public QPushButton
{
public:
    explicit ColorButton(QColor color, QWidget* parent = nullptr);
    ~ColorButton() = default;
   
   const QColor& GetColor() const noexcept { return m_Color; }

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void paintEvent(QPaintEvent*) override;

private:
  QColor m_Color;
};
