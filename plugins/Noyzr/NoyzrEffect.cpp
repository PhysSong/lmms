/*
 * NoyzrEffect.cpp - definition of the NoyzrEffect class, the Noyzr Plugin
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

#include "NoyzrEffect.h"

#include "Engine.h"
#include "Oscillator.h"
#include "embed.h"
#include "plugin_export.h"

namespace lmms
{

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT noyzr_plugin_descriptor =
{
	LMMS_STRINGIFY(PLUGIN_NAME),
	"Noyzr",
	QT_TRANSLATE_NOOP("pluginBrowser", "A native noise multiplier plugin"),
	"Hyunjin Song <tteu.ingog/at/gmail.com>",
	0x0100,
	Plugin::Type::Effect,
	new PluginPixmapLoader("logo"),
	NULL,
	NULL
} ;

}




NoyzrEffect::NoyzrEffect(Model* parent, const Plugin::Descriptor::SubPluginFeatures::Key* key) :
	Effect(&noyzr_plugin_descriptor, parent, key),
	m_noyzrControls(this),
	m_amt(0.0f)
{
}




NoyzrEffect::~NoyzrEffect()
{
}




bool NoyzrEffect::processAudioBuffer(sampleFrame* buf, const fpp_t frames)
{
	if (!isEnabled() || !isRunning ())
	{
		return false;
	}
	double outSum = 0.0;
	m_amt = m_noyzrControls.m_noiseAmtModel.value();
	for (fpp_t f = 0; f < frames; ++f)
	{
		buf[f][0] *= 1.f + m_amt * Oscillator::noiseSample(0.f);
		buf[f][1] *= 1.f + m_amt * Oscillator::noiseSample(0.f);
		outSum += buf[f][0]*buf[f][0] + buf[f][1]*buf[f][1];
	}
	checkGate(outSum / frames);

	return isRunning();
}


extern "C"
{

//needed for getting plugin out of shared lib
PLUGIN_EXPORT Plugin * lmms_plugin_main(Model* parent, void* data)
{
	return new NoyzrEffect(parent , static_cast<const Plugin::Descriptor::SubPluginFeatures::Key *>(data));
}

}

} // namespace lmms
