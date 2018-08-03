/*
 * FBSyntheffect.cpp - definition of FBSynthInstrument class.
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

#include "FBSynthInstrument.h"
#include "FBSynthInstrumentView.h"

#include "FBSynthFader.h"
#include "FBFilterInstance.h"

#include "Engine.h"
#include "InstrumentTrack.h"
#include "NotePlayHandle.h"
#include "Oscillator.h"
#include "RingBuffer.h"
#include "interpolation.h"
#include "lmms_math.h"

#include "embed.h"
#include "plugin_export.h"

namespace lmms
{

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT fbsynth_plugin_descriptor =
{
	LMMS_STRINGIFY(PLUGIN_NAME),
	"FBSynth",
	QT_TRANSLATE_NOOP("pluginBrowser", "A native FBSynth plugin"),
	"Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>",
	0x0100,
	Plugin::Type::Instrument,
	new PluginPixmapLoader("logo"),
	NULL,
	NULL
} ;

}

// Plugin data

class FBEnvelope
{
	MM_OPERATORS
public:
	FBEnvelope(float att, float dec, float sus, float rel, float dtime) :
		a(att),
		d(dec),
		s(sus),
		r(rel),
		t(0.0f),
		trel(0.0f),
		dt(dtime),
		released(false)
	{
	}

	// TODO allow nonlinear envelope
	inline float tick()
	{
		float ret;
		if (released)
		{
			ret = t >= r ? 0.f : s * (1.0f - t / r);
		}
		else
		{
			if (t < a)
			{
				ret = t / a;
			}
			else if (t < a + d)
			{
				ret = s + (1.0f - s) * (1.0f - (t - a) / d);
			}
			else
			{
				ret = s;
			}
		}
		t += dt;
		return ret;
	}

	inline void release()
	{
		released = true;
		t = 0.0f;
	}

	float a, d, s, r;
	float t, trel, dt;
	bool released;
};

class FBSynthProcessor
{
	MM_OPERATORS
public:
	FBSynthProcessor(f_cnt_t bufLen, FBSynthInstrument *inst, float att, float dec, float sus, float rel) :
		ringBuf(bufLen),
		filter(inst),
		env(att, dec, sus, rel, 1.f / Engine::audioEngine()->processingSampleRate())
	{
	}

	RingBuffer ringBuf;
	FBFilterInstance filter;
	FBEnvelope env;
};


FBSynthInstrument::FBSynthInstrument(InstrumentTrack* instrumentTrack) :
	Instrument(instrumentTrack, &fbsynth_plugin_descriptor),
	m_feedbackDecayModel(0.5, 0.1, 2.0, 0.005, this, tr("Decay time")),
	m_impulseAddModel(0.5, 0.0, 2.0, 0.001, this, tr("Impulse Add")),
	m_impulseAttackModel(0.05, 0.0, 1.0, 0.001, this, tr("Impulse Attack")),
	m_impulseDecayModel(0.5, 0.01, 3.0, 0.001, this, tr("Impulse Decay")),
	m_impulseSustainModel(0.5, 0.0, 1.0, 0.005, this, tr("Impulse Sustain")),
	m_impulseReleaseModel(0.3, 0.0, 2.0, 0.001, this, tr("Impulse Release")),
	m_lowShelfGainModel(0.0, -3, 3, 0.001, this, tr("Low-shelf gain")),
	m_para1GainModel(0.0, -3, 3, 0.001, this, tr("Peak 1 gain")),
	m_para2GainModel(0.0, -3, 3, 0.001, this, tr("Peak 2 gain")),
	m_para3GainModel(0.0, -3, 3, 0.001, this, tr("Peak 3 gain")),
	m_para4GainModel(0.0, -3, 3, 0.001, this, tr("Peak 4 gain")),
	m_highShelfGainModel(0.0, -3, 3, 0.001, this, tr("High-shelf gain")),
	m_hpResModel(0.707, 0.003, 10.0, 0.001, this, tr("HP res")),
	m_lowShelfResModel(0.707, 0.55, 10.0, 0.001, this, tr("Low-shelf res")),
	m_para1BwModel(0.3, 0.1, 4, 0.001, this, tr("Peak 1 BW")),
	m_para2BwModel(0.3, 0.1, 4, 0.001, this, tr("Peak 2 BW")),
	m_para3BwModel(0.3, 0.1, 4, 0.001, this, tr("Peak 3 BW")),
	m_para4BwModel(0.3, 0.1, 4, 0.001, this, tr("Peak 4 BW")),
	m_highShelfResModel(0.707, 0.55, 10.0, 0.001, this, tr("High-shelf res")),
	m_lpResModel(0.707, 0.003, 10.0, 0.001, this, tr("LP res")),
	m_hpFreqModel(31.0, 20.0, 20000, 0.001, this, tr("HP Freq")),
	m_lowShelfFreqModel(80.0, 20.0, 20000, 0.001, this, tr("Low-shelf Freq")),
	m_para1FreqModel(120.0, 20.0, 20000, 0.001, this, tr("Peak 1 Freq")),
	m_para2FreqModel(250.0, 20.0, 20000, 0.001, this, tr("Peak 2 Freq")),
	m_para3FreqModel(2000.0, 20.0, 20000, 0.001, this, tr("Peak 3 Freq")),
	m_para4FreqModel(4000.0, 20.0, 20000, 0.001, this, tr("Peak 4 Freq")),
	m_highShelfFreqModel(12000.0, 20.0, 20000, 0.001, this, tr("High-shelf Freq")),
	m_lpFreqModel(18000.0, 20.0, 20000, 0.001, this, tr("LP Freq")),
	m_hpActiveModel(false, this, tr("HP active")),
	m_lowShelfActiveModel(false, this, tr("Low-shelf active")),
	m_para1ActiveModel(false, this, tr("Peak 1 active")),
	m_para2ActiveModel(false, this, tr("Peak 2 active")),
	m_para3ActiveModel(false, this, tr("Peak 3 active")),
	m_para4ActiveModel(false, this, tr("Peak 4 active")),
	m_highShelfActiveModel(false, this, tr("High-shelf active")),
	m_lpActiveModel(false, this, tr("LP active")),
	m_lp12Model(false, this, tr("LP 12")),
	m_lp24Model(false, this, tr("LP 24")),
	m_lp48Model(false, this, tr("LP 48")),
	m_hp12Model(false, this, tr("HP 12")),
	m_hp24Model(false, this, tr("HP 24")),
	m_hp48Model(false, this, tr("HP 48")),
	m_lpTypeModel(0, 0, 2, this, tr("Low-pass type")),
	m_hpTypeModel(0, 0, 2, this, tr("High-pass type"))
{
}




FBSynthInstrument::~FBSynthInstrument()
{
}


void FBSynthInstrument::saveSettings(QDomDocument& doc, QDomElement& parent)
{
	// Save the settings for EQ
	m_lowShelfGainModel.saveSettings(doc, parent, "Lowshelfgain");
	m_para1GainModel.saveSettings(doc, parent, "Peak1gain");
	m_para2GainModel.saveSettings(doc, parent, "Peak2gain");
	m_para3GainModel.saveSettings(doc, parent, "Peak3gain");
	m_para4GainModel.saveSettings(doc, parent, "Peak4gain");
	m_highShelfGainModel.saveSettings(doc, parent, "HighShelfgain");
	m_hpResModel.saveSettings(doc, parent,"HPres");
	m_lowShelfResModel.saveSettings(doc, parent, "LowShelfres");
	m_para1BwModel.saveSettings(doc, parent,"Peak1bw");
	m_para2BwModel.saveSettings(doc, parent,"Peak2bw");
	m_para3BwModel.saveSettings(doc, parent,"Peak3bw");
	m_para4BwModel.saveSettings(doc, parent,"Peak4bw");
	m_highShelfResModel.saveSettings(doc, parent, "HighShelfres");
	m_lpResModel.saveSettings(doc, parent, "LPres");
	m_hpFreqModel.saveSettings(doc, parent, "HPfreq");
	m_lowShelfFreqModel.saveSettings(doc, parent, "LowShelffreq");
	m_para1FreqModel.saveSettings(doc, parent, "Peak1freq");
	m_para2FreqModel.saveSettings(doc, parent, "Peak2freq");
	m_para3FreqModel.saveSettings(doc, parent, "Peak3freq");
	m_para4FreqModel.saveSettings(doc, parent, "Peak4freq");
	m_highShelfFreqModel.saveSettings(doc, parent, "Highshelffreq");
	m_lpFreqModel.saveSettings(doc, parent, "LPfreq");
	m_hpActiveModel.saveSettings(doc, parent, "HPactive");
	m_lowShelfActiveModel.saveSettings(doc, parent, "Lowshelfactive");
	m_para1ActiveModel.saveSettings(doc, parent, "Peak1active");
	m_para2ActiveModel.saveSettings(doc, parent, "Peak2active");
	m_para3ActiveModel.saveSettings(doc, parent, "Peak3active");
	m_para4ActiveModel.saveSettings(doc, parent, "Peak4active");
	m_highShelfActiveModel.saveSettings(doc, parent, "Highshelfactive");
	m_lpActiveModel.saveSettings(doc, parent, "LPactive");
	m_lp12Model.saveSettings(doc, parent, "LP12");
	m_lp24Model.saveSettings(doc, parent, "LP24");
	m_lp48Model.saveSettings(doc, parent, "LP48");
	m_hp12Model.saveSettings(doc, parent, "HP12");
	m_hp24Model.saveSettings(doc, parent, "HP24");
	m_hp48Model.saveSettings(doc, parent, "HP48");
	m_lpTypeModel.saveSettings(doc, parent, "LP");
	m_hpTypeModel.saveSettings(doc, parent, "HP");
	// Load the other settings
	m_feedbackDecayModel.saveSettings(doc, parent, "FBDecay");
	m_impulseAttackModel.saveSettings(doc, parent, "ImpAttack");
	m_impulseDecayModel.saveSettings(doc, parent, "ImpDecay");
	m_impulseSustainModel.saveSettings(doc, parent, "ImpSustain");
	m_impulseReleaseModel.saveSettings(doc, parent, "ImpRelease");

	// Save the other settings
}

void FBSynthInstrument::loadSettings(const QDomElement & e)
{
	// Load the settings for EQ
	m_lowShelfGainModel.loadSettings(e, "Lowshelfgain");
	m_para1GainModel.loadSettings(e, "Peak1gain");
	m_para2GainModel.loadSettings(e, "Peak2gain");
	m_para3GainModel.loadSettings(e, "Peak3gain");
	m_para4GainModel.loadSettings(e, "Peak4gain");
	m_highShelfGainModel.loadSettings(e, "HighShelfgain");
	m_hpResModel.loadSettings(e,"HPres");
	m_lowShelfResModel.loadSettings(e, "LowShelfres");
	m_para1BwModel.loadSettings(e,"Peak1bw");
	m_para2BwModel.loadSettings(e,"Peak2bw");
	m_para3BwModel.loadSettings(e,"Peak3bw");
	m_para4BwModel.loadSettings(e,"Peak4bw");
	m_highShelfResModel.loadSettings(e, "HighShelfres");
	m_lpResModel.loadSettings(e, "LPres");
	m_hpFreqModel.loadSettings(e, "HPfreq");
	m_lowShelfFreqModel.loadSettings(e, "LowShelffreq");
	m_para1FreqModel.loadSettings(e, "Peak1freq");
	m_para2FreqModel.loadSettings(e, "Peak2freq");
	m_para3FreqModel.loadSettings(e, "Peak3freq");
	m_para4FreqModel.loadSettings(e, "Peak4freq");
	m_highShelfFreqModel.loadSettings(e, "Highshelffreq");
	m_lpFreqModel.loadSettings(e, "LPfreq");
	m_hpActiveModel.loadSettings(e, "HPactive");
	m_lowShelfActiveModel.loadSettings(e, "Lowshelfactive");
	m_para1ActiveModel.loadSettings(e, "Peak1active");
	m_para2ActiveModel.loadSettings(e, "Peak2active");
	m_para3ActiveModel.loadSettings(e, "Peak3active");
	m_para4ActiveModel.loadSettings(e, "Peak4active");
	m_highShelfActiveModel.loadSettings(e, "Highshelfactive");
	m_lpActiveModel.loadSettings(e, "LPactive");
	m_lp12Model.loadSettings(e, "LP12");
	m_lp24Model.loadSettings(e, "LP24");
	m_lp48Model.loadSettings(e, "LP48");
	m_hp12Model.loadSettings(e, "HP12");
	m_hp24Model.loadSettings(e, "HP24");
	m_hp48Model.loadSettings(e, "HP48");
	m_lpTypeModel.loadSettings(e, "LP");
	m_hpTypeModel.loadSettings(e, "HP");

	// Load the other settings
	m_feedbackDecayModel.loadSettings(e, "FBDecay");
	m_impulseAttackModel.loadSettings(e, "ImpAttack");
	m_impulseDecayModel.loadSettings(e, "ImpDecay");
	m_impulseSustainModel.loadSettings(e, "ImpSustain");
	m_impulseReleaseModel.loadSettings(e, "ImpRelease");
}

void FBSynthInstrument::playNote(NotePlayHandle* n, sampleFrame* workingBuffer)
{
	const int sampleRate = Engine::audioEngine()->processingSampleRate();
	const f_cnt_t frames = n->framesLeftForCurrentPeriod();
	const f_cnt_t offset = n->noteOffset();

	if (n->m_pluginData == nullptr)
	{
		n->m_pluginData = new FBSynthProcessor(sampleRate * 2, this,
			m_impulseAttackModel.value(),
			m_impulseDecayModel.value(),
			m_impulseSustainModel.value(),
			m_impulseReleaseModel.value());
	}

	FBSynthProcessor* const proc = static_cast<FBSynthProcessor*>(n->m_pluginData);
	proc->filter.setFramesPerPeroid(frames);
	for (fpp_t f = 0; f < frames; ++f)
	{
		const float decayPerCycle = n->frequency() * m_feedbackDecayModel.value();
		const float impGain = 0.5f / sqrt(decayPerCycle);
		const float addImpulse = m_impulseAddModel.value() - 1.f; // Compensate ringbuf
		const float fbLevel = 1.f - 1.f / decayPerCycle;
		// TODO make this support fractional delay
		proc->ringBuf.read(workingBuffer + offset + f, -1000.f / (float)n->frequency(), 1);
		proc->filter.applyFilter(workingBuffer[offset + f]);

		// TODO impulse pan/filter
		if (n->isReleaseStarted() && !proc->env.released)
		{
			proc->env.release();
		}

		float impulse = impGain * proc->env.tick() * Oscillator::noiseSample(f);

		workingBuffer[offset + f][0] = workingBuffer[offset + f][0] * fbLevel + impulse;
		workingBuffer[offset + f][1] = workingBuffer[offset + f][1] * fbLevel + impulse;
		proc->ringBuf.write(workingBuffer + offset + f, 0, 1);
		proc->ringBuf.movePosition(1);

		// Dry impulse
		workingBuffer[offset + f][0] += addImpulse * impulse;
		workingBuffer[offset + f][1] += addImpulse * impulse;
	}
	// FIXME this shouldn't be here
	applyRelease(workingBuffer, n);

	instrumentTrack()->processAudioBuffer(workingBuffer, frames + offset, n);
}

f_cnt_t FBSynthInstrument::desiredReleaseFrames() const
{
	return 4 * Engine::audioEngine()->processingSampleRate();
}

void FBSynthInstrument::deleteNotePluginData(NotePlayHandle* n)
{
	delete static_cast<FBSynthProcessor*>(n->m_pluginData);
	n->m_pluginData = nullptr;
}

QString FBSynthInstrument::nodeName() const
{
	return fbsynth_plugin_descriptor.name;
}

gui::PluginView* FBSynthInstrument::instantiateView(QWidget* parent)
{
	return new gui::FBSynthInstrumentView(this, parent);
}

extern "C"
{

//needed for getting plugin out of shared lib
PLUGIN_EXPORT Plugin * lmms_plugin_main(Model* parent, void*)
{
	return new FBSynthInstrument(static_cast<InstrumentTrack*>(parent));
}

}

} // namespace lmms
