/*
 * NoyzrEffect.h - declaration of NoyzrEffect class, the Noyzr plugin
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

#ifndef NOYZREFFECT_H
#define NOYZREFFECT_H

#include "Effect.h"
#include "NoyzrControls.h"

namespace lmms
{

class NoyzrEffect : public Effect
{
public:
	NoyzrEffect(Model* parent , const Descriptor::SubPluginFeatures::Key* key);
	virtual ~NoyzrEffect();
	virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);
	virtual EffectControls* controls()
	{
		return &m_noyzrControls;
	}

private:
	NoyzrControls m_noyzrControls;
	float m_amt;
};

} // namespace lmms

#endif // NOYZREFFECT_H