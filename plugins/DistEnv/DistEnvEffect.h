/*
 * DistEnvEffect.h - amplifier-effect-plugin
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


#ifndef DISTENV_EFFECT_H
#define DISTENV_EFFECT_H

#include "Effect.h"
#include "DistEnvControls.h"
#include "ValueBuffer.h"

namespace lmms
{

class DistEnvEffect : public Effect
{
public:
	DistEnvEffect(Model* parent, const Descriptor::SubPluginFeatures::Key* key);
	virtual ~DistEnvEffect();
	virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);

	virtual EffectControls* controls()
	{
		return &m_distControls;
	}


private:
	DistEnvControls m_distControls;

	float m_envLevel;

	friend class DistEnvControls;

};
} // namespace lmms

#endif
