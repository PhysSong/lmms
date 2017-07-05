/*
 * NuOsc.cpp - Osillators with unison
 *
 * Copyright (c) 2017 Hyunjin Song <tteu.ingog/at/gmail.com>
 *
 * This file is part of LMMS - http://lmms.io
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

#include "NuOsc.h"

namespace lmms
{

NuOsc::NuOsc(const IntModel * waveShapeModel,
				const IntModel * modAlgoModel,
				const double & freq,
				const double & detuning,
				const double & offset,
				const float & volume,
				const int nvoice,
				const float fspread,
				const float frand,
				bool fsame,
				const float prand,
				bool psame,
				NuOsc * sub):
	m_waveShapeModel(waveShapeModel),
	m_modulationAlgoModel(modAlgoModel),
	m_freq(freq),
	m_detuning(detuning),
	m_offset(offset),
	m_phase(offset),
	m_ext_phase(offset),
	m_volume(volume),
	m_nvoice(nvoice),
	m_userWave(NULL),
	m_sub(sub)
	{
	//TODO: HANDLING WHEN THE NUMBER OF VOICES IS DIFFERENT
	if (m_nvoice < 1) m_nvoice = 1;
	if (m_nvoice > MAX_VOICES) m_nvoice = MAX_VOICES;
	if (m_sub) m_nvoice = m_sub->m_nvoice;
	float step = fspread / sqrt((float)m_nvoice);
	float mul = 1.0f;
	for (int i = 1; i < m_nvoice - 1; ++i) mul *= static_cast<float>(i) / static_cast<float>(i + m_nvoice - 1);
	m_mult[0]=sqrt(mul);
	for (int i = 1; i < m_nvoice; ++i) m_mult[i] = m_mult[i-1] * static_cast<float>(m_nvoice - i) / static_cast<float>(i);
	for (int i = 0; i < m_nvoice; ++i)
	{
		m_ifmult[i] = m_sub && fsame ? m_sub->m_ifmult[i]
					: pow(2.0f,  step / 1200.0f * (2.0f * i + 1.0f - m_nvoice + frand * 0.01f - rand() * 0.02f * frand / RAND_MAX));
		m_ifreq[i] = m_freq * m_ifmult[i];
		m_iofs[i] = m_sub && psame ? m_offset - m_sub->m_offset + m_sub->m_iofs[i]
					: m_offset + rand() * 0.01f / RAND_MAX * prand - 0.005f * prand;
		m_pOsc[i] = new Oscillator(
					m_waveShapeModel,
					m_modulationAlgoModel,
					m_ifreq[i],
					m_detuning,
					m_iofs[i],
					m_volume,
					m_sub ? m_sub->m_pOsc[i] : NULL);
	}
}

void NuOsc::recalcFreq()
{
	for (int i = 0; i < m_nvoice; ++i) m_ifreq[i] = m_freq * m_ifmult[i];
	if (m_sub) m_sub->recalcFreq();
}

void NuOsc::update(sampleFrame * buf, const fpp_t frames,
					const ch_cnt_t channel)
{
	recalcFreq();
	sampleFrame * tf = new sampleFrame[frames];
	m_pOsc[0]->update(buf, frames, channel);
	for (int i = 1; i < m_nvoice; ++i)
	{
		m_pOsc[i]->update(tf, frames, channel);
		for (fpp_t frame = 0; frame < frames; ++frame)
		{
			buf[frame][channel] += m_mult[i] * tf[frame][channel];
		}
	}
	delete[] tf;
}

} // namespace lmms
