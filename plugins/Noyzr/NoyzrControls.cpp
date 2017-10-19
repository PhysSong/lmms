/*
 * NoyzrControls.cpp - definition of NoyzrControls class.
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

#include <QtXml/QDomElement>

#include "NoyzrControls.h"
#include "NoyzrEffect.h"
#include "Engine.h"
#include "Song.h"

namespace lmms
{

NoyzrControls::NoyzrControls(NoyzrEffect* effect):
	EffectControls(effect),
	m_effect (effect),
	m_noiseAmtModel(0.0, 0.0, 1.0, 0.001, this, tr ("Noise Amount"))
{
}




void NoyzrControls::loadSettings(const QDomElement& de)
{
	m_noiseAmtModel.loadSettings(de, "NoiseAmt");
}




void NoyzrControls::saveSettings(QDomDocument& doc, QDomElement& de)
{
	m_noiseAmtModel.saveSettings(doc, de, "NoiseAmt");
}

} // namespace lmms
