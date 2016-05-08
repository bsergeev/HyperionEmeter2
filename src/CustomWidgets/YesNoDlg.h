#pragma once
#ifndef YESNODLG_H
#define YESNODLG_H


#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>

class QWidget;
class QCheckBox;
//------------------------------------------------------------------------------

class YesNoDlg : public QDialog
{
   Q_OBJECT
public:
    YesNoDlg( QString windowTitle
            , QString question
            , QString dontAskTxt = QString()
            , QMessageBox::StandardButtons = QMessageBox::Ok
            , QWidget* parent=0 );
    virtual ~YesNoDlg() {}

    bool IsDontAskAgainSelected() const;
    bool WasCancelled() const { return m_Cancelled; }

private:
    void Cancelled();

//private data:
    QCheckBox* m_DontAskAgain_CkBx;
    bool       m_Cancelled;
};

//------------------------------------------------------------------------------
#endif // YESNODLG_H

