/*
 * TripleOscillator2.h - powerful instrument with three oscillators
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

#ifndef TRIPLE_OSCILLATOR_2_H
#define TRIPLE_OSCILLATOR_2_H

#include "Instrument.h"
#include "InstrumentView.h"
#include "LedCheckBox.h"
#include "Knob.h"
#include "NuOsc.h"
#include "AutomatableModel.h"

namespace lmms
{

class NotePlayHandle;
class SampleBuffer;

const int NUM_OF_OSCILLATORS = 4;

namespace gui
{
class TripleOscillator2View;
}

class OscillatorObject : public Model
{
	MM_OPERATORS
	Q_OBJECT
public:
	OscillatorObject( Model * _parent, int _idx );
	virtual ~OscillatorObject();


private:
	FloatModel m_volumeModel;
	FloatModel m_panModel;
	FloatModel m_coarseModel;
	FloatModel m_fineLeftModel;
	FloatModel m_fineRightModel;
	FloatModel m_phaseOffsetModel;
	FloatModel m_stereoPhaseDetuningModel;
	IntModel m_waveShapeModel;
	IntModel m_modulationAlgoModel;
	SampleBuffer* m_sampleBuffer;

	float m_volumeLeft;
	float m_volumeRight;

	// normalized detuning -> x/sampleRate
	double m_detuningLeft;
	double m_detuningRight;
	// normalized offset -> x/360
	double m_phaseOffsetLeft;
	double m_phaseOffsetRight;

	friend class TripleOscillator2;
	friend class gui::TripleOscillator2View;


private slots:
	void oscUserDefWaveDblClick();

	void updateVolume();
	void updateDetuningLeft();
	void updateDetuningRight();
	void updatePhaseOffsetLeft();
	void updatePhaseOffsetRight();

} ;




class TripleOscillator2 : public Instrument
{
	Q_OBJECT
public:
	TripleOscillator2( InstrumentTrack * _track );
	virtual ~TripleOscillator2();

	virtual void playNote( NotePlayHandle * _n,
						sampleFrame * _working_buffer );
	virtual void deleteNotePluginData( NotePlayHandle * _n );


	virtual void saveSettings( QDomDocument & _doc, QDomElement & _parent );
	virtual void loadSettings( const QDomElement & _this );

	virtual QString nodeName() const;

	virtual f_cnt_t desiredReleaseFrames() const
	{
		return 128;
	}

	virtual gui::PluginView * instantiateView( QWidget * _parent );


protected slots:
	void updateAllDetuning();


private:
	FloatModel m_unisonModel;
	FloatModel m_fspreadModel;
	FloatModel m_frandModel;
	BoolModel m_fsameModel;
	FloatModel m_prandModel;
	BoolModel m_psameModel;

	OscillatorObject * m_osc[NUM_OF_OSCILLATORS];

	struct oscPtr
	{
		MM_OPERATORS
		NuOsc * oscLeft;
		NuOsc * oscRight;
	} ;


	friend class gui::TripleOscillator2View;

} ;

namespace gui
{

class AutomatableButtonGroup;
class PixmapButton;

class TripleOscillator2View : public InstrumentView
{
	Q_OBJECT
public:
	TripleOscillator2View( Instrument * _instrument, QWidget * _parent );
	QSize sizeHint() const override { return QSize(254, 374); }
	virtual ~TripleOscillator2View();


private:
	virtual void modelChanged();

	automatableButtonGroup * m_mod1BtnGrp;
	automatableButtonGroup * m_mod2BtnGrp;
	automatableButtonGroup * m_mod3BtnGrp;

	automatableButtonGroup * m_modBtnGrp[NUM_OF_OSCILLATORS];

	struct OscillatorKnobs
	{
		MM_OPERATORS
		OscillatorKnobs( Knob * v,
					Knob * p,
					Knob * c,
					Knob * fl,
					Knob * fr,
					Knob * po,
					Knob * spd,
					PixmapButton * uwb,
					automatableButtonGroup * wsbg ) :
			m_volKnob( v ),
			m_panKnob( p ),
			m_coarseKnob( c ),
			m_fineLeftKnob( fl ),
			m_fineRightKnob( fr ),
			m_phaseOffsetKnob( po ),
			m_stereoPhaseDetuningKnob( spd ),
			m_userWaveButton( uwb ),
			m_waveShapeBtnGrp( wsbg )
		{
		}
		OscillatorKnobs()
		{
		}
		Knob * m_volKnob;
		Knob * m_panKnob;
		Knob * m_coarseKnob;
		Knob * m_fineLeftKnob;
		Knob * m_fineRightKnob;
		Knob * m_phaseOffsetKnob;
		Knob * m_stereoPhaseDetuningKnob;
		PixmapButton * m_userWaveButton;
		automatableButtonGroup * m_waveShapeBtnGrp;

	} ;
	Knob * m_unisonKnob;
	Knob * m_fspreadKnob;
	Knob * m_frandKnob;
	LedCheckBox * m_fsameLed;
	Knob * m_prandKnob;
	LedCheckBox * m_psameLed;
	OscillatorKnobs m_oscKnobs[NUM_OF_OSCILLATORS];

};

} // namespace gui

} // namespace lmms

#endif
