/*
 * TripleOscillator2.cpp - powerful instrument with three oscillators
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


#include <QDomDocument>
#include <QBitmap>
#include <QPainter>

#include "TripleOscillator2.h"
#include "AutomatableButton.h"
#include "debug.h"
#include "Engine.h"
#include "InstrumentTrack.h"
#include "Mixer.h"
#include "NotePlayHandle.h"
#include "PixmapButton.h"
#include "SampleBuffer.h"
#include "ToolTip.h"

#include "embed.h"
#include "plugin_export.h"


extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT tripleoscillator2_plugin_descriptor =
{
	STRINGIFY( PLUGIN_NAME ),
	"TripleOscillator2",
	QT_TRANSLATE_NOOP( "pluginBrowser",
				"Three powerful oscillators you can modulate "
				"in several ways(with unison)" ),
	"Richard H.J. Song",
	0x0100,
	Plugin::Instrument,
	new PluginPixmapLoader( "logo" ),
	NULL,
	NULL
} ;

}



OscillatorObject::OscillatorObject( Model * _parent, int _idx ) :
	Model( _parent ),
	m_volumeModel( DefaultVolume / NUM_OF_OSCILLATORS, MinVolume,
			MaxVolume, 1.0f, this, tr( "Osc %1 volume" ).arg( _idx+1 ) ),
	m_panModel( DefaultPanning, PanningLeft, PanningRight, 1.0f, this,
			tr( "Osc %1 panning" ).arg( _idx+1 ) ),
	m_coarseModel( 0,
			-2 * KeysPerOctave, 2 * KeysPerOctave, 1.0f, this,
			tr( "Osc %1 coarse detuning" ).arg( _idx+1 ) ),
	m_fineLeftModel( 0.0f, -100.0f, 100.0f, 1.0f, this,
			tr( "Osc %1 fine detuning left" ).arg( _idx+1 ) ),
	m_fineRightModel( 0.0f, -100.0f, 100.0f, 1.0f, this,
			tr( "Osc %1 fine detuning right" ).arg( _idx + 1 ) ),
	m_phaseOffsetModel( 0.0f, 0.0f, 360.0f, 1.0f, this,
			tr( "Osc %1 phase-offset" ).arg( _idx+1 ) ),
	m_stereoPhaseDetuningModel( 0.0f, 0.0f, 360.0f, 1.0f, this,
			tr( "Osc %1 stereo phase-detuning" ).arg( _idx+1 ) ),
	m_waveShapeModel( NuOsc::SineWave, 0,
			NuOsc::NumWaveShapes-1, this,
			tr( "Osc %1 wave shape" ).arg( _idx+1 ) ),
	m_modulationAlgoModel( NuOsc::SignalMix, 0,
				NuOsc::NumModulationAlgos-1, this,
				tr( "Modulation type %1" ).arg( _idx+1 ) ),
	m_sampleBuffer( new SampleBuffer ),
	m_volumeLeft( 0.0f ),
	m_volumeRight( 0.0f ),
	m_detuningLeft( 0.0f ),
	m_detuningRight( 0.0f ),
	m_phaseOffsetLeft( 0.0f ),
	m_phaseOffsetRight( 0.0f )
{
	// Connect knobs with Oscillators' inputs
	connect( &m_volumeModel, SIGNAL( dataChanged() ),
					this, SLOT( updateVolume() ) );
	connect( &m_panModel, SIGNAL( dataChanged() ),
					this, SLOT( updateVolume() ) );
	updateVolume();

	connect( &m_coarseModel, SIGNAL( dataChanged() ),
				this, SLOT( updateDetuningLeft() ) );
	connect( &m_coarseModel, SIGNAL( dataChanged() ),
				this, SLOT( updateDetuningRight() ) );
	connect( &m_fineLeftModel, SIGNAL( dataChanged() ),
				this, SLOT( updateDetuningLeft() ) );
	connect( &m_fineRightModel, SIGNAL( dataChanged() ),
				this, SLOT( updateDetuningRight() ) );
	updateDetuningLeft();
	updateDetuningRight();

	connect( &m_phaseOffsetModel, SIGNAL( dataChanged() ),
			this, SLOT( updatePhaseOffsetLeft() ) );
	connect( &m_phaseOffsetModel, SIGNAL( dataChanged() ),
			this, SLOT( updatePhaseOffsetRight() ) );
	connect( &m_stereoPhaseDetuningModel, SIGNAL( dataChanged() ),
			this, SLOT( updatePhaseOffsetLeft() ) );
	updatePhaseOffsetLeft();
	updatePhaseOffsetRight();

}




OscillatorObject::~OscillatorObject()
{
	sharedObject::unref( m_sampleBuffer );
}




void OscillatorObject::oscUserDefWaveDblClick()
{
	QString af = m_sampleBuffer->openAndSetWaveformFile();
	if( af != "" )
	{
		// TODO:
		//ToolTip::add( m_usrWaveBtn, m_sampleBuffer->audioFile() );
	}
}




void OscillatorObject::updateVolume()
{
	if( m_panModel.value() >= 0.0f )
	{
		const float panningFactorLeft = 1.0f - m_panModel.value()
							/ (float)PanningRight;
		m_volumeLeft = panningFactorLeft * m_volumeModel.value() /
									100.0f;
		m_volumeRight = m_volumeModel.value() / 100.0f;
	}
	else
	{
		m_volumeLeft = m_volumeModel.value() / 100.0f;
		const float panningFactorRight = 1.0f + m_panModel.value()
							/ (float)PanningRight;
		m_volumeRight = panningFactorRight * m_volumeModel.value() /
									100.0f;
	}
}




void OscillatorObject::updateDetuningLeft()
{
	m_detuningLeft = powf( 2.0f, ( (float)m_coarseModel.value() * 100.0f
				+ (float)m_fineLeftModel.value() ) / 1200.0f )
				/ Engine::mixer()->processingSampleRate();
}




void OscillatorObject::updateDetuningRight()
{
	m_detuningRight = powf( 2.0f, ( (float)m_coarseModel.value() * 100.0f
				+ (float)m_fineRightModel.value() ) / 1200.0f )
				/ Engine::mixer()->processingSampleRate();
}




void OscillatorObject::updatePhaseOffsetLeft()
{
	m_phaseOffsetLeft = ( m_phaseOffsetModel.value() +
				m_stereoPhaseDetuningModel.value() ) / 360.0f;
}




void OscillatorObject::updatePhaseOffsetRight()
{
	m_phaseOffsetRight = m_phaseOffsetModel.value() / 360.0f;
}




TripleOscillator2::TripleOscillator2( InstrumentTrack * _instrument_track ) :
	Instrument( _instrument_track, &tripleoscillator2_plugin_descriptor ),
	m_unisonModel( 1.f, 1.f,(float)MAX_VOICES, 1.f, this,
				tr( "Unison voices" ) ),
	m_fspreadModel( 0.f, 0.f, 100.f, 1.f, this,
				tr( "Unison frequency spread" ) ),
	m_frandModel( 0.f, 0.f, 100.f, 1.f, this,
				tr( "Unison frequency randomness" ) ),
	m_fsameModel( false, this, tr( "Use same detune value" ) ),
	m_prandModel( 0.f, 0.f, 100.f, 1.f, this,
				tr( "Unison phase randomness" ) ),
	m_psameModel( false, this, tr( "Use same phase" ) )
{
	for( int i = 0; i < NUM_OF_OSCILLATORS; ++i )
	{
		m_osc[i] = new OscillatorObject( this, i );

	}

	connect( Engine::mixer(), SIGNAL( sampleRateChanged() ),
			this, SLOT( updateAllDetuning() ) );
}




TripleOscillator2::~TripleOscillator2()
{
}




void TripleOscillator2::saveSettings( QDomDocument & _doc, QDomElement & _this )
{
	for( int i = 0; i < NUM_OF_OSCILLATORS; ++i )
	{
		QString is = QString::number( i );
		m_osc[i]->m_volumeModel.saveSettings( _doc, _this, "vol_" + is );
		m_osc[i]->m_panModel.saveSettings( _doc, _this, "pan_" + is );
		m_osc[i]->m_coarseModel.saveSettings( _doc, _this, "coarse_"
									+ is );
		m_osc[i]->m_fineLeftModel.saveSettings( _doc, _this, "finel_" +
									is );
		m_osc[i]->m_fineRightModel.saveSettings( _doc, _this, "finer_" +
									is );
		m_osc[i]->m_phaseOffsetModel.saveSettings( _doc, _this,
							"phoffset_" + is );
		m_osc[i]->m_stereoPhaseDetuningModel.saveSettings( _doc, _this,
							"stphdetun_" + is );
		m_osc[i]->m_waveShapeModel.saveSettings( _doc, _this,
							"wavetype_" + is );
		m_osc[i]->m_modulationAlgoModel.saveSettings( _doc, _this,
					"modalgo_" + QString::number( i+1 ) );
		_this.setAttribute( "userwavefile_" + is,
					m_osc[i]->m_sampleBuffer->audioFile() );
	}
	m_unisonModel.saveSettings( _doc, _this,
					"unison_");
	m_fspreadModel.saveSettings( _doc, _this,
					"fspread_");
	m_frandModel.saveSettings( _doc, _this,
					"frand_");
	m_fsameModel.saveSettings( _doc, _this,
					"fsame_");
	m_prandModel.saveSettings( _doc, _this,
					"prand_");
	m_psameModel.saveSettings( _doc, _this,
					"psame_");
}




void TripleOscillator2::loadSettings( const QDomElement & _this )
{
	for( int i = 0; i < NUM_OF_OSCILLATORS; ++i )
	{
		const QString is = QString::number( i );
		m_osc[i]->m_volumeModel.loadSettings( _this, "vol_" + is );
		m_osc[i]->m_panModel.loadSettings( _this, "pan_" + is );
		m_osc[i]->m_coarseModel.loadSettings( _this, "coarse_" + is );
		m_osc[i]->m_fineLeftModel.loadSettings( _this, "finel_" + is );
		m_osc[i]->m_fineRightModel.loadSettings( _this, "finer_" + is );
		m_osc[i]->m_phaseOffsetModel.loadSettings( _this,
							"phoffset_" + is );
		m_osc[i]->m_stereoPhaseDetuningModel.loadSettings( _this,
							"stphdetun_" + is );
		m_osc[i]->m_waveShapeModel.loadSettings( _this, "wavetype_" +
									is );
		m_osc[i]->m_modulationAlgoModel.loadSettings( _this,
					"modalgo_" + QString::number( i+1 ) );
		m_osc[i]->m_sampleBuffer->setAudioFile( _this.attribute(
							"userwavefile_" + is ) );
	}
	m_unisonModel.loadSettings( _this,
					"unison_");
	m_fspreadModel.loadSettings( _this,
					"fspread_");
	m_frandModel.loadSettings( _this,
					"frand_");
	m_fsameModel.loadSettings( _this,
					"fsame_");
	m_prandModel.loadSettings( _this,
					"prand_");
	m_psameModel.loadSettings( _this,
					"psame_");
}




QString TripleOscillator2::nodeName() const
{
	return tripleoscillator2_plugin_descriptor.name;
}




void TripleOscillator2::playNote( NotePlayHandle * _n,
						sampleFrame * _working_buffer )
{
	if( _n->totalFramesPlayed() == 0 || _n->m_pluginData == NULL )
	{
		NuOsc * oscs_l[NUM_OF_OSCILLATORS];
		NuOsc * oscs_r[NUM_OF_OSCILLATORS];
		for( int i = NUM_OF_OSCILLATORS - 1; i >= 0; --i )
		{

			// the last oscs needs no sub-oscs...
			if( i == NUM_OF_OSCILLATORS - 1 )
			{
				oscs_l[i] = new NuOsc(
						&m_osc[i]->m_waveShapeModel,
						&m_osc[i]->m_modulationAlgoModel,
						_n->frequency(),
						m_osc[i]->m_detuningLeft,
						m_osc[i]->m_phaseOffsetLeft,
						m_osc[i]->m_volumeLeft,
						m_unisonModel.value(),
						m_fspreadModel.value(),
						m_frandModel.value(),
						m_fsameModel.value(),
						m_prandModel.value(),
						m_psameModel.value() );
				oscs_r[i] = new NuOsc(
						&m_osc[i]->m_waveShapeModel,
						&m_osc[i]->m_modulationAlgoModel,
						_n->frequency(),
						m_osc[i]->m_detuningRight,
						m_osc[i]->m_phaseOffsetRight,
						m_osc[i]->m_volumeRight,
						m_unisonModel.value(),
						m_fspreadModel.value(),
						m_frandModel.value(),
						m_fsameModel.value(),
						m_prandModel.value(),
						m_psameModel.value() );
			}
			else
			{
				oscs_l[i] = new NuOsc(
						&m_osc[i]->m_waveShapeModel,
						&m_osc[i]->m_modulationAlgoModel,
						_n->frequency(),
						m_osc[i]->m_detuningLeft,
						m_osc[i]->m_phaseOffsetLeft,
						m_osc[i]->m_volumeLeft,
						m_unisonModel.value(),
						m_fspreadModel.value(),
						m_frandModel.value(),
						m_fsameModel.value(),
						m_prandModel.value(),
						m_psameModel.value(),
						oscs_l[i + 1] );
				oscs_r[i] = new NuOsc(
						&m_osc[i]->m_waveShapeModel,
						&m_osc[i]->m_modulationAlgoModel,
						_n->frequency(),
						m_osc[i]->m_detuningRight,
						m_osc[i]->m_phaseOffsetRight,
						m_osc[i]->m_volumeRight,
						m_unisonModel.value(),
						m_fspreadModel.value(),
						m_frandModel.value(),
						m_fsameModel.value(),
						m_prandModel.value(),
						m_psameModel.value(),
						oscs_r[i + 1] );
			}

			oscs_l[i]->setUserWave( m_osc[i]->m_sampleBuffer );
			oscs_r[i]->setUserWave( m_osc[i]->m_sampleBuffer );

		}

		_n->m_pluginData = new oscPtr;
		static_cast<oscPtr *>( _n->m_pluginData )->oscLeft = oscs_l[0];
		static_cast<oscPtr *>( _n->m_pluginData )->oscRight =
								oscs_r[0];
	}

	NuOsc * osc_l = static_cast<oscPtr *>( _n->m_pluginData )->oscLeft;
	NuOsc * osc_r = static_cast<oscPtr *>( _n->m_pluginData )->oscRight;

	const fpp_t frames = _n->framesLeftForCurrentPeriod();
	const f_cnt_t offset = _n->noteOffset();

	osc_l->update( _working_buffer + offset, frames, 0 );
	osc_r->update( _working_buffer + offset, frames, 1 );

	applyRelease( _working_buffer, _n );

	instrumentTrack()->processAudioBuffer( _working_buffer, frames + offset, _n );
}




void TripleOscillator2::deleteNotePluginData( NotePlayHandle * _n )
{
	delete static_cast<NuOsc *>( static_cast<oscPtr *>(
						_n->m_pluginData )->oscLeft );
	delete static_cast<NuOsc *>( static_cast<oscPtr *>(
						_n->m_pluginData )->oscRight );
	delete static_cast<oscPtr *>( _n->m_pluginData );
}




PluginView * TripleOscillator2::instantiateView( QWidget * _parent )
{
	return new TripleOscillator2View( this, _parent );
}




void TripleOscillator2::updateAllDetuning()
{
	for( int i = 0; i < NUM_OF_OSCILLATORS; ++i )
	{
		m_osc[i]->updateDetuningLeft();
		m_osc[i]->updateDetuningRight();
	}
}




class TripleOsc2Knob : public Knob
{
public:
	TripleOsc2Knob( QWidget * _parent ) :
			Knob( knobBright_26, _parent )
	{
		setFixedSize( 28, 35 );
	}
};

// 82, 109


TripleOscillator2View::TripleOscillator2View( Instrument * _instrument,
							QWidget * _parent ) :
	InstrumentView( _instrument, _parent )
{;
	setAutoFillBackground( true );
	QPalette pal;
	pal.setBrush( backgroundRole(),
				PLUGIN_NAME::getIconPixmap( "artwork" ) );
	setPalette( pal );

	const int mod_x = 66;
	const int mod_y = 61;
	const int mod_h = 17;
	const int osc_y = 65 + mod_h * NUM_OF_OSCILLATORS;
	const int osc_h = 52;
	const int ctkb_y = 25;
	const int ctbx_y = 11;

	// TODO: clean rewrite using layouts and all that...
	for( int i = 0; i < NUM_OF_OSCILLATORS - 1; ++i ){
		PixmapButton * pm_osc_btn = new PixmapButton( this, NULL );
		pm_osc_btn->move( mod_x, mod_y + mod_h * i );
		pm_osc_btn->setActiveGraphic( PLUGIN_NAME::getIconPixmap(
									"pm_active" ) );
		pm_osc_btn->setInactiveGraphic( PLUGIN_NAME::getIconPixmap(
								"pm_inactive" ) );
		ToolTip::add( pm_osc_btn, tr( "Use phase modulation for "
						"modulating oscillator %1 with "
						"oscillator %2" ).arg(i+1).arg(i+2) );

		PixmapButton * am_osc_btn = new PixmapButton( this, NULL );
		am_osc_btn->move( mod_x + 35, mod_y + mod_h * i );
		am_osc_btn->setActiveGraphic( PLUGIN_NAME::getIconPixmap(
									"am_active" ) );
		am_osc_btn->setInactiveGraphic( PLUGIN_NAME::getIconPixmap(
								"am_inactive" ) );
		ToolTip::add( am_osc_btn, tr( "Use amplitude modulation for "
						"modulating oscillator %1 with "
						"oscillator %2" ).arg(i+1).arg(i+2) );

		PixmapButton * mix_osc_btn = new PixmapButton( this, NULL );
		mix_osc_btn->move( mod_x + 70, mod_y + mod_h * i );
		mix_osc_btn->setActiveGraphic( PLUGIN_NAME::getIconPixmap(
								"mix_active" ) );
		mix_osc_btn->setInactiveGraphic( PLUGIN_NAME::getIconPixmap(
								"mix_inactive" ) );
		ToolTip::add( mix_osc_btn, tr( "Mix output of oscillator %1 & %2" ).arg(i+1).arg(i+2) );

		PixmapButton * sync_osc_btn = new PixmapButton( this, NULL );
		sync_osc_btn->move( mod_x + 105, mod_y + mod_h * i );
		sync_osc_btn->setActiveGraphic( PLUGIN_NAME::getIconPixmap(
								"sync_active" ) );
		sync_osc_btn->setInactiveGraphic( PLUGIN_NAME::getIconPixmap(
								"sync_inactive" ) );
		ToolTip::add( sync_osc_btn, tr( "Synchronize oscillator %1 with "
								"oscillator %2" ).arg(i+1).arg(i+2) );

		PixmapButton * fm_osc_btn = new PixmapButton( this, NULL );
		fm_osc_btn->move( mod_x + 140, mod_y + mod_h * i );
		fm_osc_btn->setActiveGraphic( PLUGIN_NAME::getIconPixmap(
									"fm_active" ) );
		fm_osc_btn->setInactiveGraphic( PLUGIN_NAME::getIconPixmap(
								"fm_inactive" ) );
		ToolTip::add( fm_osc_btn, tr( "Use frequency modulation for "
						"modulating oscillator %1 with "
						"oscillator %2" ).arg(i+1).arg(i+2) );

		m_modBtnGrp[i] = new automatableButtonGroup( this );
		m_modBtnGrp[i]->addButton( pm_osc_btn );
		m_modBtnGrp[i]->addButton( am_osc_btn );
		m_modBtnGrp[i]->addButton( mix_osc_btn );
		m_modBtnGrp[i]->addButton( sync_osc_btn );
		m_modBtnGrp[i]->addButton( fm_osc_btn );
	}


	for( int i = 0; i < NUM_OF_OSCILLATORS; ++i )
	{
		int knob_y = osc_y + i * osc_h;

		// setup volume-knob
		Knob * vk = new Knob( knobBright_26, this );
		vk->setVolumeKnob( true );
		vk->setFixedSize( 28, 35 );
		vk->move( 6, knob_y );
		vk->setHintText( tr( "Osc %1 volume:" ).arg(
							 i+1 ), "%" );
		vk->setWhatsThis(
			tr( "With this knob you can set the volume of "
				"oscillator %1. When setting a value of 0 the "
				"oscillator is turned off. Otherwise you can "
				"hear the oscillator as loud as you set it "
				"here.").arg( i+1 ) );

		// setup panning-knob
		Knob * pk = new TripleOsc2Knob( this );
		pk->move( 35, knob_y );
		pk->setHintText( tr("Osc %1 panning:").arg( i + 1 ), "" );
		pk->setWhatsThis(
			tr( "With this knob you can set the panning of the "
				"oscillator %1. A value of -100 means 100% "
				"left and a value of 100 moves oscillator-"
				"output right.").arg( i+1 ) );

		// setup coarse-knob
		Knob * ck = new TripleOsc2Knob( this );
		ck->move( 82, knob_y );
		ck->setHintText( tr( "Osc %1 coarse detuning:" ).arg( i + 1 )
						 , " " + tr( "semitones" ) );
		ck->setWhatsThis(
			tr( "With this knob you can set the coarse detuning of "
				"oscillator %1. You can detune the oscillator "
				"24 semitones (2 octaves) up and down. This is "
				"useful for creating sounds with a chord." ).
				arg( i + 1 ) );


		// setup knob for left fine-detuning
		Knob * flk = new TripleOsc2Knob( this );
		flk->move( 111, knob_y );
		flk->setHintText( tr( "Osc %1 fine detuning left:" ).
						  arg( i + 1 ),
							" " + tr( "cents" ) );
		flk->setWhatsThis(
			tr( "With this knob you can set the fine detuning of "
				"oscillator %1 for the left channel. The fine-"
				"detuning is ranged between -100 cents and "
				"+100 cents. This is useful for creating "
				"\"fat\" sounds." ).arg( i + 1 ) );

		// setup knob for right fine-detuning
		Knob * frk = new TripleOsc2Knob( this );
		frk->move( 140, knob_y );
		frk->setHintText( tr( "Osc %1 fine detuning right:" ).
						  arg( i + 1 ),
							" " + tr( "cents" ) );
		frk->setWhatsThis(
			tr( "With this knob you can set the fine detuning of "
				"oscillator %1 for the right channel. The "
				"fine-detuning is ranged between -100 cents "
				"and +100 cents. This is useful for creating "
				"\"fat\" sounds." ).arg( i+1 ) );


		// setup phase-offset-knob
		Knob * pok = new TripleOsc2Knob( this );
		pok->move( 188, knob_y );
		pok->setHintText( tr( "Osc %1 phase-offset:" ).
						  arg( i + 1 ),
							" " + tr( "degrees" ) );
		pok->setWhatsThis(
			tr( "With this knob you can set the phase-offset of "
				"oscillator %1. That means you can move the "
				"point within an oscillation where the "
				"oscillator begins to oscillate. For example "
				"if you have a sine-wave and have a phase-"
				"offset of 180 degrees the wave will first go "
				"down. It's the same with a square-wave."
				).arg( i+1 ) );

		// setup stereo-phase-detuning-knob
		Knob * spdk = new TripleOsc2Knob( this );
		spdk->move( 217, knob_y );
		spdk->setHintText( tr("Osc %1 stereo phase-detuning:" ).
						arg( i + 1 ),
							" " + tr( "degrees" ) );
		spdk->setWhatsThis(
			tr( "With this knob you can set the stereo phase-"
				"detuning of oscillator %1. The stereo phase-"
				"detuning specifies the size of the difference "
				"between the phase-offset of left and right "
				"channel. This is very good for creating wide "
				"stereo sounds." ).arg( i+1 ) );

		int btn_y = osc_y - 17 + i * osc_h;

		PixmapButton * sin_wave_btn = new PixmapButton( this, NULL );
		sin_wave_btn->move( 128, btn_y );
		sin_wave_btn->setActiveGraphic( PLUGIN_NAME::getIconPixmap(
							"sin_shape_active" ) );
		sin_wave_btn->setInactiveGraphic( PLUGIN_NAME::getIconPixmap(
							"sin_shape_inactive" ) );
		ToolTip::add( sin_wave_btn,
				tr( "Use a sine-wave for "
						"current oscillator." ) );

		PixmapButton * triangle_wave_btn =
						new PixmapButton( this, NULL );
		triangle_wave_btn->move( 143, btn_y );
		triangle_wave_btn->setActiveGraphic(
			PLUGIN_NAME::getIconPixmap( "triangle_shape_active" ) );
		triangle_wave_btn->setInactiveGraphic(
			PLUGIN_NAME::getIconPixmap( "triangle_shape_inactive" ) );
		ToolTip::add( triangle_wave_btn,
				tr( "Use a triangle-wave "
						"for current oscillator." ) );

		PixmapButton * saw_wave_btn = new PixmapButton( this, NULL );
		saw_wave_btn->move( 158, btn_y );
		saw_wave_btn->setActiveGraphic( PLUGIN_NAME::getIconPixmap(
							"saw_shape_active" ) );
		saw_wave_btn->setInactiveGraphic( PLUGIN_NAME::getIconPixmap(
							"saw_shape_inactive" ) );
		ToolTip::add( saw_wave_btn,
				tr( "Use a saw-wave for "
						"current oscillator." ) );

		PixmapButton * sqr_wave_btn = new PixmapButton( this, NULL );
		sqr_wave_btn->move( 173, btn_y );
		sqr_wave_btn->setActiveGraphic( PLUGIN_NAME::getIconPixmap(
						"square_shape_active" ) );
		sqr_wave_btn->setInactiveGraphic( PLUGIN_NAME::getIconPixmap(
						"square_shape_inactive" ) );
		ToolTip::add( sqr_wave_btn,
				tr( "Use a square-wave for "
						"current oscillator." ) );

		PixmapButton * moog_saw_wave_btn =
						new PixmapButton( this, NULL );
		moog_saw_wave_btn->move( 188, btn_y );
		moog_saw_wave_btn->setActiveGraphic(
			PLUGIN_NAME::getIconPixmap( "moog_saw_shape_active" ) );
		moog_saw_wave_btn->setInactiveGraphic(
			PLUGIN_NAME::getIconPixmap( "moog_saw_shape_inactive" ) );
		ToolTip::add( moog_saw_wave_btn,
				tr( "Use a moog-like saw-wave "
						"for current oscillator." ) );

		PixmapButton * exp_wave_btn = new PixmapButton( this, NULL );
		exp_wave_btn->move( 203, btn_y );
		exp_wave_btn->setActiveGraphic( PLUGIN_NAME::getIconPixmap(
							"exp_shape_active" ) );
		exp_wave_btn->setInactiveGraphic( PLUGIN_NAME::getIconPixmap(
							"exp_shape_inactive" ) );
		ToolTip::add( exp_wave_btn,
				tr( "Use an exponential "
					"wave for current oscillator." ) );

		PixmapButton * white_noise_btn = new PixmapButton( this, NULL );
		white_noise_btn->move( 218, btn_y );
		white_noise_btn->setActiveGraphic(
			PLUGIN_NAME::getIconPixmap( "white_noise_shape_active" ) );
		white_noise_btn->setInactiveGraphic(
			PLUGIN_NAME::getIconPixmap( "white_noise_shape_inactive" ) );
		ToolTip::add( white_noise_btn,
				tr( "Use white-noise for "
						"current oscillator." ) );

		PixmapButton * uwb = new PixmapButton( this, NULL );
		uwb->move( 233, btn_y );
		uwb->setActiveGraphic( PLUGIN_NAME::getIconPixmap(
							"usr_shape_active" ) );
		uwb->setInactiveGraphic( PLUGIN_NAME::getIconPixmap(
							"usr_shape_inactive" ) );
		ToolTip::add( uwb, tr( "Use a user-defined "
				"waveform for current oscillator." ) );

		automatableButtonGroup * wsbg =
			new automatableButtonGroup( this );

		wsbg->addButton( sin_wave_btn );
		wsbg->addButton( triangle_wave_btn );
		wsbg->addButton( saw_wave_btn );
		wsbg->addButton( sqr_wave_btn );
		wsbg->addButton( moog_saw_wave_btn );
		wsbg->addButton( exp_wave_btn );
		wsbg->addButton( white_noise_btn );
		wsbg->addButton( uwb );

		m_oscKnobs[i] = OscillatorKnobs( vk, pk, ck, flk, frk, pok,
							spdk, uwb, wsbg );
	}
	m_unisonKnob = new  TripleOsc2Knob( this );
	m_unisonKnob->move( 6, ctkb_y );
	m_unisonKnob->setHintText( tr("Unison voices:"), "");
	m_unisonKnob->setWhatsThis(
					tr(	"With this knob you can set the number of "
					"unison voices. When setting a value of 1 "
					"unison is turned off. Otherwise you can "
					"hear the multiple unison voices as many as"
					"you set it here.") );

	m_fspreadKnob = new  TripleOsc2Knob( this );
	m_fspreadKnob->move( 35, ctkb_y );
	m_fspreadKnob->setHintText( tr("Unison frequency sqread:"), " " + tr( "cents") );
	m_fspreadKnob->setWhatsThis(
					tr(	"With this knob you can set the amount of "
					"unison frequency spread. When setting a value of 0 "
					"frequency spread is turned off. Otherwise you can "
					"hear the multiple unison voices with different frequency "
					"as many as you set it here.") );

	m_frandKnob = new  TripleOsc2Knob( this );
	m_frandKnob->move( 82, ctkb_y );
	m_frandKnob->setHintText( tr("Unison frequency randomness:"), "%");
	m_frandKnob->setWhatsThis(
					tr(	"With this knob you can set the amount of "
					"frequency randomness. When setting a value of 0 "
					"frequency randomizing is turned off.") );
	m_fsameLed = new LedCheckBox( "", this );
	m_fsameLed->move( 11, ctbx_y );

	m_prandKnob = new  TripleOsc2Knob( this );
	m_prandKnob->move( 111, ctkb_y );
	m_prandKnob->setHintText( tr("Unison phase randomness:"), "%");
	m_prandKnob->setWhatsThis(
					tr(	"With this knob you can set the amount of "
					"phase randomness. When setting a value of 0 "
					"phase randomizing is turned off.") );
	m_psameLed = new LedCheckBox( "", this );
	m_psameLed->move( 116, ctbx_y );
	//TODO: change WhatsThis

}




TripleOscillator2View::~TripleOscillator2View()
{
}




void TripleOscillator2View::modelChanged()
{
	TripleOscillator2 * t = castModel<TripleOscillator2>();
	for( int i = 0; i < NUM_OF_OSCILLATORS - 1; ++i )
		m_modBtnGrp[i]->setModel( &t->m_osc[i]->m_modulationAlgoModel );

	for( int i = 0; i < NUM_OF_OSCILLATORS; ++i )
	{
		m_oscKnobs[i].m_volKnob->setModel(
					&t->m_osc[i]->m_volumeModel );
		m_oscKnobs[i].m_panKnob->setModel(
					&t->m_osc[i]->m_panModel );
		m_oscKnobs[i].m_coarseKnob->setModel(
					&t->m_osc[i]->m_coarseModel );
		m_oscKnobs[i].m_fineLeftKnob->setModel(
					&t->m_osc[i]->m_fineLeftModel );
		m_oscKnobs[i].m_fineRightKnob->setModel(
					&t->m_osc[i]->m_fineRightModel );
		m_oscKnobs[i].m_phaseOffsetKnob->setModel(
					&t->m_osc[i]->m_phaseOffsetModel );
		m_oscKnobs[i].m_stereoPhaseDetuningKnob->setModel(
				&t->m_osc[i]->m_stereoPhaseDetuningModel );
		m_oscKnobs[i].m_waveShapeBtnGrp->setModel(
					&t->m_osc[i]->m_waveShapeModel );
		connect( m_oscKnobs[i].m_userWaveButton,
						SIGNAL( doubleClicked() ),
				t->m_osc[i], SLOT( oscUserDefWaveDblClick() ) );
	}
	m_unisonKnob->setModel( &t->m_unisonModel );
	m_fspreadKnob->setModel( &t->m_fspreadModel );
	m_frandKnob->setModel( &t->m_frandModel );
	m_fsameLed->setModel( &t->m_fsameModel );
	m_prandKnob->setModel( &t->m_prandModel );
	m_psameLed->setModel( &t->m_psameModel );
}




extern "C"
{

// necessary for getting instance out of shared lib
PLUGIN_EXPORT Plugin * lmms_plugin_main(Model* model, void *)
{
	return new TripleOscillator2(static_cast<InstrumentTrack *>(model));
}

}
