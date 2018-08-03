/*
 * FBFilterInstance.cpp - definition of FBFilterInstance class.
 *
 * Copyright (c) 2014 David French <dave/dot/french3/at/googlemail/dot/com>
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

#include "FBFilterInstance.h"
#include "FBSynthInstrument.h"

#include "AudioEngine.h"
#include "Engine.h"


#include <QDebug> // DEBUGGING
namespace lmms
{

void FBFilterInstance::applyFilter(sampleFrame &buf)
{
	const int sampleRate = Engine::audioEngine()->processingSampleRate();

	// setup sample exact controls
	float hpRes = m_inst->m_hpResModel.value();
	float lowShelfRes = m_inst->m_lowShelfResModel.value();
	float para1Bw = m_inst->m_para1BwModel.value();
	float para2Bw = m_inst->m_para2BwModel.value();
	float para3Bw = m_inst->m_para3BwModel.value();
	float para4Bw = m_inst->m_para4BwModel.value();
	float highShelfRes = m_inst->m_highShelfResModel.value();
	float lpRes = m_inst->m_lpResModel.value();

	float hpFreq = m_inst->m_hpFreqModel.value();
	float lowShelfFreq = m_inst->m_lowShelfFreqModel.value();
	float para1Freq = m_inst->m_para1FreqModel.value();
	float para2Freq = m_inst->m_para2FreqModel.value();
	float para3Freq = m_inst->m_para3FreqModel.value();
	float para4Freq = m_inst->m_para4FreqModel.value();
	float highShelfFreq = m_inst->m_highShelfFreqModel.value();
	float lpFreq = m_inst->m_lpFreqModel.value();


	bool hpActive = m_inst->m_hpActiveModel.value();
	bool hp24Active = m_inst->m_hp24Model.value();
	bool hp48Active = m_inst->m_hp48Model.value();
	bool lowShelfActive = m_inst->m_lowShelfActiveModel.value();
	bool para1Active = m_inst->m_para1ActiveModel.value();
	bool para2Active = m_inst->m_para2ActiveModel.value();
	bool para3Active = m_inst->m_para3ActiveModel.value();
	bool para4Active = m_inst->m_para4ActiveModel.value();
	bool highShelfActive = m_inst->m_highShelfActiveModel.value();
	bool lpActive = m_inst->m_lpActiveModel.value();
	bool lp24Active = m_inst->m_lp24Model.value();
	bool lp48Active = m_inst->m_lp48Model.value();

	float lowShelfGain = m_inst->m_lowShelfGainModel.value();
	float para1Gain = m_inst->m_para1GainModel.value();
	float para2Gain = m_inst->m_para2GainModel.value();
	float para3Gain = m_inst->m_para3GainModel.value();
	float para4Gain = m_inst->m_para4GainModel.value();
	float highShelfGain = m_inst->m_highShelfGainModel.value();

	//set all filter parameters once per frame, FBSynthFilter handles
	//smooth xfading, reducing pops clicks and dc bias offsets

	m_hp12.setParameters(sampleRate, hpFreq, hpRes, 1);
	m_hp24.setParameters(sampleRate, hpFreq, hpRes, 1);
	m_hp480.setParameters(sampleRate, hpFreq, hpRes, 1);
	m_hp481.setParameters(sampleRate, hpFreq, hpRes, 1);
	m_lowShelf.setParameters(sampleRate, lowShelfFreq, lowShelfRes, lowShelfGain);
	m_para1.setParameters(sampleRate, para1Freq, para1Bw, para1Gain);
	m_para2.setParameters(sampleRate, para2Freq, para2Bw, para2Gain);
	m_para3.setParameters(sampleRate, para3Freq, para3Bw, para3Gain);
	m_para4.setParameters(sampleRate, para4Freq, para4Bw, para4Gain);
	m_highShelf.setParameters(sampleRate, highShelfFreq, highShelfRes, highShelfGain);
	m_lp12.setParameters(sampleRate, lpFreq, lpRes, 1);
	m_lp24.setParameters(sampleRate, lpFreq, lpRes, 1);
	m_lp480.setParameters(sampleRate, lpFreq, lpRes, 1);
	m_lp481.setParameters(sampleRate, lpFreq, lpRes, 1);

	float periodProgress = (float)m_frameIdx / (float)(m_fpp-1); // percentage of period processed
	if (hpActive)
	{
		buf[0] = m_hp12.update(buf[0], 0, periodProgress);
		buf[1] = m_hp12.update(buf[1], 1, periodProgress);

		if (hp24Active || hp48Active)
		{
			buf[0] = m_hp24.update(buf[0], 0, periodProgress);
			buf[1] = m_hp24.update(buf[1], 1, periodProgress);
		}

		if (hp48Active)
		{
			buf[0] = m_hp480.update(buf[0], 0, periodProgress);
			buf[1] = m_hp480.update(buf[1], 1, periodProgress);

			buf[0] = m_hp481.update(buf[0], 0, periodProgress);
			buf[1] = m_hp481.update(buf[1], 1, periodProgress);
		}
	}

	if (lowShelfActive)
	{
		buf[0] = m_lowShelf.update(buf[0], 0, periodProgress);
		buf[1] = m_lowShelf.update(buf[1], 1, periodProgress);
	}

	if (para1Active)
	{
		buf[0] = m_para1.update(buf[0], 0, periodProgress);
		buf[1] = m_para1.update(buf[1], 1, periodProgress);
	}

	if (para2Active)
	{
		buf[0] = m_para2.update(buf[0], 0, periodProgress);
		buf[1] = m_para2.update(buf[1], 1, periodProgress);
	}

	if (para3Active)
	{
		buf[0] = m_para3.update(buf[0], 0, periodProgress);
		buf[1] = m_para3.update(buf[1], 1, periodProgress);
	}

	if (para4Active)
	{
		buf[0] = m_para4.update(buf[0], 0, periodProgress);
		buf[1] = m_para4.update(buf[1], 1, periodProgress);
	}

	if (highShelfActive)
	{
		buf[0] = m_highShelf.update(buf[0], 0, periodProgress);
		buf[1] = m_highShelf.update(buf[1], 1, periodProgress);
	}

	if (lpActive)
	{
		buf[0] = m_lp12.update(buf[0], 0, periodProgress);
		buf[1] = m_lp12.update(buf[1], 1, periodProgress);

		if (lp24Active || lp48Active)
		{
			buf[0] = m_lp24.update(buf[0], 0, periodProgress);
			buf[1] = m_lp24.update(buf[1], 1, periodProgress);
		}

		if (lp48Active)
		{
			buf[0] = m_lp480.update(buf[0], 0, periodProgress);
			buf[1] = m_lp480.update(buf[1], 1, periodProgress);

			buf[0] = m_lp481.update(buf[0], 0, periodProgress);
			buf[1] = m_lp481.update(buf[1], 1, periodProgress);
		}
	}
	++m_frameIdx;
}

} // namespace lmms
