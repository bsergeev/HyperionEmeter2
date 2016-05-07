
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

#include "YesNoDlg.h"


//------------------------------------------------------------------------------

YesNoDlg::YesNoDlg( QString windowTitle
                  , QString question
                  , QString dontAskTxt
                  , bool showDontAskAgain
                  , QMessageBox::StandardButtons buttons
                  , QWidget* parent )
        : QDialog( parent )
        , m_DontAskAgain_CkBx( 0 )
        , m_Cancelled( false ) 
{
    setWindowTitle( windowTitle );

    QVBoxLayout* layout = new QVBoxLayout;

    QLabel* txt = new QLabel( question, this );
    txt->setWordWrap( true );
    layout->addWidget( txt );

    if (showDontAskAgain)
    {
        layout->addSpacing(10);
        m_DontAskAgain_CkBx = new QCheckBox( this );
        m_DontAskAgain_CkBx->setText( (dontAskTxt.isEmpty())? tr("Don't ask me again") : dontAskTxt );
        m_DontAskAgain_CkBx->setCheckState( Qt::Unchecked );
        layout->addWidget( m_DontAskAgain_CkBx );
    }


    QHBoxLayout* lay1 = new QHBoxLayout;
    lay1->addStretch( 2 );

    QPushButton* pb = 0;
    if (buttons & QMessageBox::Yes  ||  buttons & QMessageBox::Ok)
    {
        pb = new QPushButton( (buttons & QMessageBox::Yes)? tr("Yes") : tr("Ok") );
        connect( pb, SIGNAL(clicked()), SLOT(accept()) );
        pb->setDefault (true);
        lay1->addWidget( pb );

        lay1->addStretch( 1 );
    }

    if (buttons & QMessageBox::No)
    {
        pb = new QPushButton( tr("No") );
        connect( pb, SIGNAL(clicked()), this, SLOT(reject()) );
        lay1->addWidget( pb );
    }

    if (buttons & QMessageBox::Cancel) 
    {
        lay1->addStretch( 1 );
        pb = new QPushButton( tr("Cancel") );
        connect( pb, SIGNAL(clicked()), this, SLOT(Cancelled()) );
        lay1->addWidget( pb );
    }

    lay1->addStretch( 2 );
    layout->addLayout(lay1);

    this->setLayout(layout);
}

//------------------------------------------------------------------------------

void YesNoDlg::Cancelled()
{
    m_Cancelled = true;
    reject();
}

//------------------------------------------------------------------------------

bool YesNoDlg::IsDontAskAgainSelected() const
{
    bool checked = false;
    if (m_DontAskAgain_CkBx)
    {
        checked = ( m_DontAskAgain_CkBx->isChecked() );
    }
    else
    {
        Q_ASSERT( !"Can't check DontAskAgain" );
    }

    return checked;
}

//------------------------------------------------------------------------------
