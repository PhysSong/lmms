/*
 * PeakControllerDialog.cpp - per-controller-specific view for changing a
 * controller's settings
 *
 * Copyright (c) 2008-2009 Paul Giblock <drfaygo/at/gmail.com>
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


#include <QLabel>
#include <QPainter>
#include <QIcon>

#include "embed.h"

#include "PeakController.h"
#include "EffectControlDialog.h"
#include "plugins/PeakControllerEffect/PeakControllerEffect.h"
#include "plugins/PeakControllerEffect/PeakControllerEffectControlDialog.cpp"

namespace lmms::gui
{


PeakControllerDialog::PeakControllerDialog( PeakController * _model, QWidget * _parent ) :
	ControllerDialog( _model, _parent )
{
	EffectControlDialog * dlg = _model->getPeakControllerEffect()->controls()->createView();
	setWindowTitle( tr( "PEAK" ) );
	setWindowIcon( embed::getIconPixmap( "controller" ) );
	setFixedSize( dlg->size() );
	dlg->setParent( this );
	dlg->show();

	setModel( _model );
}




/*
void effectView::closeEffects()
{
	m_subWindow->hide();
	m_show = true;
}
*/


void PeakControllerDialog::contextMenuEvent( QContextMenuEvent * )
{
}




void PeakControllerDialog::paintEvent( QPaintEvent * )
{
	QPainter p( this );
}



void PeakControllerDialog::modelChanged()
{
	m_peakController = castModel<PeakController>();
}


} // namespace lmms::gui
