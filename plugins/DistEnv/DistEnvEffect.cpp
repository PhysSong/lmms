/*
 * DistEnvEffect.cpp - A native distortion effect
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

#include "DistEnvEffect.h"
#include "lmms_math.h"

#include "embed.h"
#include "plugin_export.h"


namespace lmms
{

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT distenv_plugin_descriptor =
{
	LMMS_STRINGIFY(PLUGIN_NAME),
	"DistEnv",
	QT_TRANSLATE_NOOP("pluginBrowser", "A native distortion plugin"),
	"Hyunjin Song <tteu.ingog/at/gmail.com>",
	0x0100,
	Plugin::Type::Effect,
	new PluginPixmapLoader("logo"),
	NULL,
	NULL
} ;

}



DistEnvEffect::DistEnvEffect(Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
	Effect(&distenv_plugin_descriptor, parent, key),
	m_distControls(this),
	m_envLevel(0)
{
}




DistEnvEffect::~DistEnvEffect()
{
}




bool DistEnvEffect::processAudioBuffer(sampleFrame* buf, const fpp_t frames)
{
	if (!isEnabled() || !isRunning ())
	{
		return false;
	}

	double outSum = 0.0;
	const float d = dryLevel();
	const float w = wetLevel();

	const float sr = Engine::audioEngine()->processingSampleRate();

	const ValueBuffer * ratioBuf = m_distControls.m_ratioModel.valueBuffer();
	const ValueBuffer * envAmountBuf = m_distControls.m_envAmountModel.valueBuffer();
	const ValueBuffer * attackBuf = m_distControls.m_attackModel.valueBuffer();
	const ValueBuffer * releaseBuf = m_distControls.m_releaseModel.valueBuffer();
	const ValueBuffer * gainBuf = m_distControls.m_gainModel.valueBuffer();

	float level, delta;

	for (fpp_t f = 0; f < frames; ++f)
	{
		sample_t s[2] = { buf[f][0], buf[f][1] };

		const float ratio = ratioBuf
				? ratioBuf->value(f)
				: m_distControls.m_ratioModel.value();

		const float envAmount = envAmountBuf
				? envAmountBuf->value(f)
				: m_distControls.m_envAmountModel.value();

		const float attack = attackBuf
				? attackBuf->value(f)
				: m_distControls.m_attackModel.value();

		const float release = releaseBuf
				? releaseBuf->value(f)
				: m_distControls.m_releaseModel.value();

		switch (int(m_distControls.m_envModeModel.value()))
		{
		case 0:
			level = sqrt((s[0] * s[0] + s[1] * s[1]) / 2.0f);
		case 1:
		default:
			level = qMax(abs(s[0]), abs(s[1]));
		}

		delta = level - m_envLevel;
		if (delta > 0)
		{
			m_envLevel = delta * attack / 1000.0f * sr > m_envLevel
					? m_envLevel + delta / attack * 1000.0f / sr
					: level;
		}
		else
		{
			m_envLevel = -delta * release / 1000.0f * sr > m_envLevel
					? m_envLevel + delta / release * 1000.0f / sr
					: level;
		}

		s[0] = signedPowf(s[0], ratio) * powf(m_envLevel, (1 - ratio) * envAmount);
		s[1] = signedPowf(s[1], ratio) * powf(m_envLevel, (1 - ratio) * envAmount);

		// apply gain
		if(gainBuf)
		{
			s[0] *= gainBuf->value(f) * 0.01f;
			s[1] *= gainBuf->value(f) * 0.01f;
		}
		else
		{
			s[0] *= m_distControls.m_gainModel.value() * 0.01f;
			s[1] *= m_distControls.m_gainModel.value() * 0.01f;
		}

		buf[f][0] = d * buf[f][0] + w * s[0];
		buf[f][1] = d * buf[f][1] + w * s[1];

		outSum += buf[f][0]*buf[f][0] + buf[f][1]*buf[f][1];
	}

	checkGate(outSum / frames);

	return isRunning();
}



extern "C"
{

// necessary for getting instance out of shared lib
PLUGIN_EXPORT Plugin * lmms_plugin_main(Model* parent, void* data)
{
	return new DistEnvEffect(parent, static_cast<const Plugin::Descriptor::SubPluginFeatures::Key *>(data));
}

}

} // namespace lmms
