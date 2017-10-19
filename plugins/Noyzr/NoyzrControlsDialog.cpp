/*
 * NoyzrControlsDialog.cpp - definition of NoyzrControlsDialog class.
 *
 * Copyright (c) 2017 Hyunjin Song <tteu.ingog/at/gmail.com>
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

#include "NoyzrControlsDialog.h"
#include "NoyzrControls.h"
#include "embed.h"
#include <QMouseEvent>
#include <QPainter>

namespace lmms
{

namespace gui
{

NoyzrControlsDialog::NoyzrControlsDialog(NoyzrControls *controls) :
	EffectControlDialog(controls)
{
	setAutoFillBackground(true);
	QPalette pal;
	pal.setBrush(backgroundRole(), PLUGIN_NAME::getIconPixmap("artwork"));
	setPalette(pal);
	setFixedSize(300, 208);

	Knob* AmountKnob = new Knob(KnobType::Bright26, this);
	AmountKnob->move(10,14);
	AmountKnob->setVolumeKnob(true);
	AmountKnob->setModel(&controls->m_noiseAmtModel);
	AmountKnob->setLabel(tr("AMT"));
	AmountKnob->setHintText(tr("Noise Amount") + " ", " %");
}

} // namespace gui

} // namespace lmms
