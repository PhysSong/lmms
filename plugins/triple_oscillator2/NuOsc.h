/*
 * NuOsc.h - Osillators with unison
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

#ifndef NUOSC_H
#define NUOSC_H

#include "Oscillator.h"
const int MAX_VOICES = 10;

class NuOsc{
	MM_OPERATORS
public:
	enum WaveShapes
	{
		SineWave,
		TriangleWave,
		SawWave,
		SquareWave,
		MoogSawWave,
		ExponentialWave,
		WhiteNoise,
		UserDefinedWave,
		NumWaveShapes
	} ;

	enum ModulationAlgos
	{
		PhaseModulation,
		AmplitudeModulation,
		SignalMix,
		SynchronizedBySubOsc,
		FrequencyModulation,
		NumModulationAlgos
	} ;

	NuOsc(const IntModel * waveShapeModel,
				const IntModel * modAlgoModel,
				const float & freq,
				const float & detuning,
				const float & offset,
				const float & volume,
				const int nvoice,
				const float fspread,
				const float frand,
				bool fsame,
				const float prand,
				bool psame,
				NuOsc * sub = NULL);
	virtual ~NuOsc()
	{
		for (int i = 0; i < m_nvoice; ++i) if (m_sub) m_sub->m_pOsc[i] = NULL;
		delete m_sub;
		for (int i = 0; i < m_nvoice; ++i) delete m_pOsc[i];
	}
	inline void setUserWave(const SampleBuffer * wave)
	{
		m_userWave = wave;
		for (int i = 0; i < m_nvoice; ++i) m_pOsc[i]->setUserWave(m_userWave);
	}
	void update(sampleFrame * _ab, const fpp_t _frames,
				const ch_cnt_t _chnl);
protected:
	void recalcFreq();
private:
	const IntModel * m_waveShapeModel;
	const IntModel * m_modulationAlgoModel;
	const float & m_freq;
	const float & m_detuning;
	float m_offset;
	float m_phase;
	const float & m_ext_phase;
	const float & m_volume;
	int m_nvoice;
	const SampleBuffer * m_userWave;
	NuOsc * m_sub;
	float m_weight[MAX_VOICES];
	float m_mult[MAX_VOICES];
	float m_ifreq[MAX_VOICES];
	float m_ifmult[MAX_VOICES];
	float m_iofs[MAX_VOICES];
	Oscillator *m_pOsc[MAX_VOICES];

};
#endif
