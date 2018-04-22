/*
 * EffectRackView.cpp - view for effectChain model
 *
 * Copyright (c) 2006-2007 Danny McRae <khjklujn@netscape.net>
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#include <QApplication>
#include <QLayout>
#include <QPushButton>
#include <QScrollArea>

#include "EffectRackView.h"
#include "EffectSelectDialog.h"
#include "EffectView.h"
#include "GroupBox.h"


EffectRackView::EffectRackView( EffectChain* model, QWidget* parent ) :
	QWidget( parent ),
	ModelView( NULL, this ),
	m_needToRecreateViews( true ),
	m_lastY( 0 )
{
	QVBoxLayout* mainLayout = new QVBoxLayout( this );
	mainLayout->setMargin( 5 );

	m_effectsGroupBox = new GroupBox( tr( "EFFECTS CHAIN" ) );
	mainLayout->addWidget( m_effectsGroupBox );

	QVBoxLayout* effectsLayout = new QVBoxLayout( m_effectsGroupBox );
	effectsLayout->setSpacing( 0 );
	effectsLayout->setContentsMargins( 2, m_effectsGroupBox->titleBarHeight() + 2, 2, 2 );

	m_scrollArea = new QScrollArea;
	m_scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
	m_scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	m_scrollArea->setPalette( QApplication::palette( m_scrollArea ) );
	m_scrollArea->setFrameStyle( QFrame::NoFrame );
	m_scrollArea->setWidget( new QWidget );

	effectsLayout->addWidget( m_scrollArea );

	QPushButton* addButton = new QPushButton;
	addButton->setText( tr( "Add effect" ) );

	effectsLayout->addWidget( addButton );

	connect( addButton, SIGNAL( clicked() ), this, SLOT( addEffect() ) );

	setModel( model );
}



EffectRackView::~EffectRackView()
{
	clearViews();
}





void EffectRackView::clearViews()
{
	while( m_effectViews.size() )
	{
		EffectView * e = m_effectViews[m_effectViews.size() - 1];
		m_effectViews.pop_back();
		delete e;
	}
}




void EffectRackView::moveUp( EffectView* view )
{
	if( view != m_effectViews.first() )
	{
		int i = m_effectViews.indexOf( view );

		EffectView * temp = m_effectViews[ i - 1 ];

		m_effectViews[i - 1] = view;
		m_effectViews[i] = temp;

		m_needToRecreateViews = false;
	}
	fxChain()->moveUp( view->effect() );
}




void EffectRackView::moveDown( EffectView* view )
{
	if( view != m_effectViews.last() )
	{
		// moving next effect up is the same
		moveUp( *( qFind( m_effectViews.begin(), m_effectViews.end(), view ) + 1 ) );
	}
}




void EffectRackView::addEffect()
{
	EffectSelectDialog esd( this );
	esd.exec();

	if( esd.result() == QDialog::Rejected )
	{
		return;
	}

	Effect * fx = esd.instantiateSelectedPlugin( fxChain() );
	EffectView * view = createEffectView( fx );

	fxChain()->m_enabledModel.setValue( true );
	m_effectViews.append( view );
	m_needToRecreateViews = false;
	fxChain()->appendEffect( fx );

	view->editControls();
}




void EffectRackView::replacePlugin( EffectView* view )
{
	EffectSelectDialog esd( this );
	esd.exec();

	if( esd.result() == QDialog::Rejected )
	{
		return;
	}

	Effect * fx = esd.instantiateSelectedPlugin( fxChain() );
	int idx = m_effectViews.indexOf( view );
	EffectView * newView = createEffectView( fx );
	m_effectViews.replace( idx, newView );
	delete view;
	m_needToRecreateViews = false;
	fxChain()->replaceEffect( fx, idx );
	view->effect()->deleteLater();

	newView->editControls();
}




void EffectRackView::deletePlugin( EffectView* view )
{
	Effect * e = view->effect();
	m_effectViews.erase( qFind( m_effectViews.begin(), m_effectViews.end(), view ) );
	delete view;
	m_needToRecreateViews = false;
	fxChain()->removeEffect( e );
	e->deleteLater();
}




void EffectRackView::update()
{
	QWidget * w = m_scrollArea->widget();
	const int EffectViewMargin = 3;
	m_lastY = EffectViewMargin;

	// Don't recreate views if we've already updated
	if( m_needToRecreateViews )
	{
		m_effectViews.clear();
		for( Effect * fx : fxChain()->m_effects )
		{
			m_effectViews.append( createEffectView( fx ) );
		}
	}
	m_needToRecreateViews = true;

	for( EffectView * view : m_effectViews )
	{
		view->move( EffectViewMargin, m_lastY );
		view->show();
		m_lastY += view->height();
	}

	w->setFixedSize( 210 + 2*EffectViewMargin, m_lastY );

	QWidget::update();
}




EffectView* EffectRackView::createEffectView(Effect* effect)
{
	EffectView* view = new EffectView( effect, m_scrollArea->widget() );
	connect( view, SIGNAL( moveUp( EffectView * ) ),
			this, SLOT( moveUp( EffectView * ) ) );
	connect( view, SIGNAL( moveDown( EffectView * ) ),
			this, SLOT( moveDown( EffectView * ) ) );
	connect( view, SIGNAL( replacePlugin( EffectView * ) ),
			this, SLOT( replacePlugin( EffectView * ) ),
					Qt::QueuedConnection );
	connect( view, SIGNAL( deletePlugin( EffectView * ) ),
			this, SLOT( deletePlugin( EffectView * ) ),
					Qt::QueuedConnection );

	return view;
}




void EffectRackView::modelChanged()
{
	//clearViews();
	m_effectsGroupBox->setModel( &fxChain()->m_enabledModel );
	connect( fxChain(), SIGNAL( aboutToClear() ), this, SLOT( clearViews() ) );
	m_needToRecreateViews = true;
	update();
}





