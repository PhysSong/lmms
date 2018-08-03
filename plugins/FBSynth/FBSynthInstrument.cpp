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

#include "Engine.h"
#include "InstrumentTrack.h"
#include "NotePlayHandle.h"
#include "Oscillator.h"
#include "RingBuffer.h"
#include "interpolation.h"
#include "lmms_math.h"

#include "embed.h"
#include "plugin_export.h"

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT fbsynth_plugin_descriptor =
{
	STRINGIFY(PLUGIN_NAME),
	"FBSynth",
	QT_TRANSLATE_NOOP("pluginBrowser", "A native FBSynth plugin"),
	"Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>",
	0x0100,
	Plugin::Instrument,
	new PluginPixmapLoader("logo"),
	NULL,
	NULL
} ;

}

// Plugin data

class FBSynthProcessor
{
	MM_OPERATORS
public:
	enum EnvelopeState {Attack, Decay, Sustain, Release};
	FBSynthProcessor(EnvelopeState envState, float envValue);
	~FBSynthProcessor() = default;
	void process(sampleFrame* workingBuffer, f_cnt_t frames, float fbLevel, float freq);
private:
	RingBuffer m_ringBuf;
	EnvelopeState m_envState;
	float m_envValue;
};


FBSynthProcessor::FBSynthProcessor(EnvelopeState envState, float envValue) :
	m_ringBuf(2000.0f),
	m_envState(envState),
	m_envValue(envValue)
{}


void FBSynthProcessor::process(sampleFrame* workingBuffer, f_cnt_t frames, float fbLevel, float freq)
{

}


FBSynthInstrument::FBSynthInstrument(InstrumentTrack* instrumentTrack) :
	Instrument(instrumentTrack, &fbsynth_plugin_descriptor),
	m_feedbackDecayModel(0.5, 0.1, 2.0, 0.01, this, tr("Decay time")),
	m_impulseAttackModel(0.05, 0.0, 2.0, 0.01, this, tr("Impulse Attack")),
	m_impulseDecayModel(0.5, 0.1, 2.0, 0.01, this, tr("Impulse Decay")),
	m_impulseSustainModel(0.5, 0.0, 1.0, 0.01, this, tr("Impulse Sustain")),
	m_impulseReleaseModel(0.3, 0.0, 2.0, 0.01, this, tr("Impulse Release")),
	m_lowShelfGainModel(0.0, -18, 18, 0.001, this, tr("Low-shelf gain")),
	m_para1GainModel(0.0, -18, 18, 0.001, this, tr("Peak 1 gain")),
	m_para2GainModel(0.0, -18, 18, 0.001, this, tr("Peak 2 gain")),
	m_para3GainModel(0.0, -18, 18, 0.001, this, tr("Peak 3 gain")),
	m_para4GainModel(0.0, -18, 18, 0.001, this, tr("Peak 4 gain")),
	m_highShelfGainModel(0.0, -18, 18, 0.001, this, tr("High-shelf gain")),
	m_hpResModel(0.707, 0.003, 10.0, 0.001, this, tr("HP res")),
	m_lowShelfResModel(1.4, 0.55, 10.0, 0.001, this, tr("Low-shelf res")),
	m_para1BwModel(0.3, 0.1, 4, 0.001, this, tr("Peak 1 BW")),
	m_para2BwModel(0.3, 0.1, 4, 0.001, this, tr("Peak 2 BW")),
	m_para3BwModel(0.3, 0.1, 4, 0.001, this, tr("Peak 3 BW")),
	m_para4BwModel(0.3, 0.1, 4, 0.001, this, tr("Peak 4 BW")),
	m_highShelfResModel(1.4, 0.55, 10.0, 0.001, this, tr("High-shelf res")),
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

void FBSynthInstrument::applyFilter(sampleFrame* inBuf, sampleFrame* outBuf, const f_cnt_t frames)
{
	const int sampleRate = Engine::mixer()->processingSampleRate();

	// setup sample exact controls
	float hpRes = m_hpResModel.value();
	float lowShelfRes = m_lowShelfResModel.value();
	float para1Bw = m_para1BwModel.value();
	float para2Bw = m_para2BwModel.value();
	float para3Bw = m_para3BwModel.value();
	float para4Bw = m_para4BwModel.value();
	float highShelfRes = m_highShelfResModel.value();
	float lpRes = m_lpResModel.value();

	float hpFreq = m_hpFreqModel.value();
	float lowShelfFreq = m_lowShelfFreqModel.value();
	float para1Freq = m_para1FreqModel.value();
	float para2Freq = m_para2FreqModel.value();
	float para3Freq = m_para3FreqModel.value();
	float para4Freq = m_para4FreqModel.value();
	float highShelfFreq = m_highShelfFreqModel.value();
	float lpFreq = m_lpFreqModel.value();


	bool hpActive = m_hpActiveModel.value();
	bool hp24Active = m_hp24Model.value();
	bool hp48Active = m_hp48Model.value();
	bool lowShelfActive = m_lowShelfActiveModel.value();
	bool para1Active = m_para1ActiveModel.value();
	bool para2Active = m_para2ActiveModel.value();
	bool para3Active = m_para3ActiveModel.value();
	bool para4Active = m_para4ActiveModel.value();
	bool highShelfActive = m_highShelfActiveModel.value();
	bool lpActive = m_lpActiveModel.value();
	bool lp24Active = m_lp24Model.value();
	bool lp48Active = m_lp48Model.value();

	float lowShelfGain = m_lowShelfGainModel.value();
	float para1Gain = m_para1GainModel.value();
	float para2Gain = m_para2GainModel.value();
	float para3Gain = m_para3GainModel.value();
	float para4Gain = m_para4GainModel.value();
	float highShelfGain = m_highShelfGainModel.value();

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

	float periodProgress = 0.0f; // percentage of period processed
	for (fpp_t f = 0; f < frames; ++f)
	{
		periodProgress = (float)f / (float)(frames-1);
		if (hpActive)
		{
			outBuf[f][0] = m_hp12.update(inBuf[f][0], 0, periodProgress);
			outBuf[f][1] = m_hp12.update(inBuf[f][1], 1, periodProgress);

			if (hp24Active || hp48Active)
			{
				outBuf[f][0] = m_hp24.update(inBuf[f][0], 0, periodProgress);
				outBuf[f][1] = m_hp24.update(inBuf[f][1], 1, periodProgress);
			}

			if (hp48Active)
			{
				outBuf[f][0] = m_hp480.update(inBuf[f][0], 0, periodProgress);
				outBuf[f][1] = m_hp480.update(inBuf[f][1], 1, periodProgress);

				outBuf[f][0] = m_hp481.update(inBuf[f][0], 0, periodProgress);
				outBuf[f][1] = m_hp481.update(inBuf[f][1], 1, periodProgress);
			}
		}

		if (lowShelfActive)
		{
			outBuf[f][0] = m_lowShelf.update(inBuf[f][0], 0, periodProgress);
			outBuf[f][1] = m_lowShelf.update(inBuf[f][1], 1, periodProgress);
		}

		if (para1Active)
		{
			outBuf[f][0] = m_para1.update(inBuf[f][0], 0, periodProgress);
			outBuf[f][1] = m_para1.update(inBuf[f][1], 1, periodProgress);
		}

		if (para2Active)
		{
			outBuf[f][0] = m_para2.update(inBuf[f][0], 0, periodProgress);
			outBuf[f][1] = m_para2.update(inBuf[f][1], 1, periodProgress);
		}

		if (para3Active)
		{
			outBuf[f][0] = m_para3.update(inBuf[f][0], 0, periodProgress);
			outBuf[f][1] = m_para3.update(inBuf[f][1], 1, periodProgress);
		}

		if (para4Active)
		{
			outBuf[f][0] = m_para4.update(inBuf[f][0], 0, periodProgress);
			outBuf[f][1] = m_para4.update(inBuf[f][1], 1, periodProgress);
		}

		if (highShelfActive)
		{
			outBuf[f][0] = m_highShelf.update(inBuf[f][0], 0, periodProgress);
			outBuf[f][1] = m_highShelf.update(inBuf[f][1], 1, periodProgress);
		}

		if (lpActive)
		{
			outBuf[f][0] = m_lp12.update(inBuf[f][0], 0, periodProgress);
			outBuf[f][1] = m_lp12.update(inBuf[f][1], 1, periodProgress);

			if (lp24Active || lp48Active)
			{
				outBuf[f][0] = m_lp24.update(inBuf[f][0], 0, periodProgress);
				outBuf[f][1] = m_lp24.update(inBuf[f][1], 1, periodProgress);
			}

			if (lp48Active)
			{
				outBuf[f][0] = m_lp480.update(inBuf[f][0], 0, periodProgress);
				outBuf[f][1] = m_lp480.update(inBuf[f][1], 1, periodProgress);

				outBuf[f][0] = m_lp481.update(inBuf[f][0], 0, periodProgress);
				outBuf[f][1] = m_lp481.update(inBuf[f][1], 1, periodProgress);
			}
		}
	}
}

void FBSynthInstrument::playNote(NotePlayHandle* n, sampleFrame* workingBuffer)
{
	const int sampleRate = Engine::mixer()->processingSampleRate();
	const f_cnt_t frames = n->framesLeftForCurrentPeriod();
	const f_cnt_t offset = n->noteOffset();

	if (n->m_pluginData == nullptr)
	{
		n->m_pluginData = new RingBuffer(sampleRate * 2);
	}

	RingBuffer* const ringBuf = static_cast<RingBuffer*>(n->m_pluginData);
	for (fpp_t f = 0; f < frames; ++f)
	{
		const float fbLevel = 1.f - 1.f / n->frequency() / m_feedbackDecayModel.value();
		ringBuf->read(workingBuffer + offset + f, -1000.f / (float)n->frequency(), 1);
		if (!n->isReleaseStarted())
		{
			workingBuffer[offset + f][0] = workingBuffer[offset + f][0] * fbLevel + Oscillator::noiseSample(f);
			workingBuffer[offset + f][1] = workingBuffer[offset + f][1] * fbLevel + Oscillator::noiseSample(f);
		}
		else
		{
			workingBuffer[offset + f][0] = workingBuffer[offset + f][0] * fbLevel;
			workingBuffer[offset + f][1] = workingBuffer[offset + f][1] * fbLevel;
		}
		ringBuf->write(workingBuffer + offset + f, 0, 1);
		ringBuf->movePosition(1);
		workingBuffer[offset + f][0] *= 0.5f / sqrt(n->frequency() * m_feedbackDecayModel.value());
		workingBuffer[offset + f][1] *= 0.5f / sqrt(n->frequency() * m_feedbackDecayModel.value());
	}
	applyFilter(workingBuffer + offset, workingBuffer + offset, frames);
	applyRelease(workingBuffer, n);

	instrumentTrack()->processAudioBuffer(workingBuffer, frames + offset, n);
}

f_cnt_t FBSynthInstrument::desiredReleaseFrames() const
{
	return 4 * Engine::mixer()->processingSampleRate();
}

void FBSynthInstrument::deleteNotePluginData(NotePlayHandle* n)
{
	delete static_cast<RingBuffer*>(n->m_pluginData);
	n->m_pluginData = nullptr;
}

QString FBSynthInstrument::nodeName() const
{
	return fbsynth_plugin_descriptor.name;
}

PluginView* FBSynthInstrument::instantiateView(QWidget* parent)
{
	return new FBSynthInstrumentView(this, parent);
}

extern "C"
{

//needed for getting plugin out of shared lib
PLUGIN_EXPORT Plugin * lmms_plugin_main(Model* parent, void*)
{
	return new FBSynthInstrument(static_cast<InstrumentTrack*>(parent));
}

}
