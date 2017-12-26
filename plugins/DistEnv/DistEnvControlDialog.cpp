/*
 * DistEnvControlDialog.cpp - control dialog for amplifier effect
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include <QLayout>

#include "DistEnvControlDialog.h"
#include "DistEnvControls.h"
#include "embed.h"


namespace lmms
{

namespace gui
{

DistEnvControlDialog::DistEnvControlDialog(DistEnvControls* controls) :
	EffectControlDialog(controls)
{
	setAutoFillBackground(true);
	QPalette pal;
	pal.setBrush(backgroundRole(), PLUGIN_NAME::getIconPixmap("artwork"));
	setPalette(pal);
	setFixedSize(200, 110);

	Knob * ratioKnob = new Knob(KnobType::Bright26, this);
	ratioKnob -> move(16, 10);
	ratioKnob->setModel(&controls->m_ratioModel);
	ratioKnob->setLabel(tr("RATIO"));
	ratioKnob->setHintText(tr("Ratio:") , "");

	Knob * envAmtKnob = new Knob(KnobType::Bright26, this);
	envAmtKnob -> move(66, 10);
	envAmtKnob->setModel(&controls->m_envAmountModel);
	envAmtKnob->setLabel(tr("ENV AMT"));
	envAmtKnob->setHintText(tr("Envelope Amount:") , "");

	Knob * envModeKnob = new Knob(KnobType::Bright26, this);
	envModeKnob -> move(116, 10);
	envModeKnob->setModel(&controls->m_envModeModel);
	envModeKnob->setLabel(tr("ENV MODE"));
	envModeKnob->setHintText(tr("Envelope Mode") , "");

	Knob * attackKnob = new Knob(KnobType::Bright26, this);
	attackKnob -> move(16, 64);
	attackKnob->setModel(&controls->m_attackModel);
	attackKnob->setLabel(tr("ATT"));
	attackKnob->setHintText(tr("Attack:") , "ms");

	Knob * releaseKnob = new Knob(KnobType::Bright26, this);
	releaseKnob -> move(66, 64);
	releaseKnob->setModel(&controls->m_releaseModel);
	releaseKnob->setLabel(tr("REL"));
	releaseKnob->setHintText(tr("Release:") , "ms");

	Knob * gainKnob = new Knob(KnobType::Bright26, this);
	gainKnob -> move(116, 64);
	gainKnob -> setVolumeKnob(true);
	gainKnob->setModel(&controls->m_gainModel);
	gainKnob->setLabel(tr("GAIN"));
	gainKnob->setHintText(tr("Gain:") , "");
}

} // namespace gui

} // namespace lmms
