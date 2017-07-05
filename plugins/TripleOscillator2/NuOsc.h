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

namespace lmms
{

const int MAX_VOICES = 10;

class NuOsc
{
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
	void update(sampleFrame * buf, const fpp_t frames,
				const ch_cnt_t channel);
protected:
	void recalcFreq();
private:
	const IntModel * m_waveShapeModel;
	const IntModel * m_modulationAlgoModel;
	const double & m_freq;
	const double & m_detuning;
	double m_offset;
	double m_phase;
	const double & m_ext_phase;
	const float & m_volume;
	int m_nvoice;
	const SampleBuffer * m_userWave;
	NuOsc * m_sub;
	float m_weight[MAX_VOICES];
	float m_mult[MAX_VOICES];
	double m_ifreq[MAX_VOICES];
	double m_ifmult[MAX_VOICES];
	double m_iofs[MAX_VOICES];
	Oscillator *m_pOsc[MAX_VOICES];

};

} // namespace lmms

#endif
