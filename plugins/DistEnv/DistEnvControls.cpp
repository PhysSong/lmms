/*
 * DistEnvControls.cpp - controls for amplifier effect
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
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


#include <QDomElement>

#include "DistEnvControls.h"
#include "DistEnvEffect.h"
#include "Engine.h"
#include "Song.h"


namespace lmms
{

DistEnvControls::DistEnvControls(DistEnvEffect* effect) :
	EffectControls(effect),
	m_effect(effect),
	m_ratioModel(1.0f, 0.01f, 1.0f, 0.01f, this, tr("Ratio")),
	m_envAmountModel(0.0f, 0.0f, 1.0f, 0.01f, this, tr("Envelope Amount")),
	m_attackModel(10.0f, 0.0f, 2000.0f, 1.0f, this, tr("Attack")),
	m_releaseModel(100.0f, 0.0f, 2000.0f, 1.0f, this, tr("Release")),
	m_envModeModel(0.0f, 0.0f, 2.0f, 1.0f, this, tr("Envelope Mode")),
	m_gainModel(100.0f, 0.0f, 200.0f, 0.1f, this, tr("Gain"))
{
}




void DistEnvControls::changeControl()
{
}




void DistEnvControls::loadSettings(const QDomElement& elem)
{
	m_ratioModel.loadSettings(elem, "ratio");
	m_envAmountModel.loadSettings(elem, "envamt");
	m_attackModel.loadSettings(elem, "att");
	m_releaseModel.loadSettings(elem, "rel");
	m_envModeModel.loadSettings(elem, "envmode");
	m_gainModel.loadSettings(elem, "gain");
}




void DistEnvControls::saveSettings(QDomDocument& doc, QDomElement& elem)
{
	m_ratioModel.saveSettings(doc, elem, "ratio");
	m_envAmountModel.saveSettings(doc, elem, "envamt");
	m_attackModel.saveSettings(doc, elem, "att");
	m_releaseModel.saveSettings(doc, elem, "rel");
	m_envModeModel.saveSettings(doc, elem, "envmode");
	m_gainModel.saveSettings(doc, elem, "gain");
}
} // namespace lmms
