/*
 * FBSynthInstrument.cpp - definition of FBSynthInstrument class.
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


#include "FBSynthInstrumentView.h"

#include <QGraphicsView>
#include <QLayout>
#include <QWidget>

#include "AutomatableButton.h"
#include "embed.h"
#include "Engine.h"
#include "Knob.h"
#include "Fader.h"
#include "LedCheckBox.h"
#include "PixmapButton.h"

#include "FBSynthFader.h"
#include "FBSynthParameterWidget.h"

namespace lmms
{

namespace gui
{

FBSynthInstrumentView::FBSynthInstrumentView(FBSynthInstrument * instrument, QWidget * parent) :
	InstrumentView(instrument, parent),
	m_instrument(instrument)
{
	setAutoFillBackground(true);
	QPalette pal;
	pal.setBrush(backgroundRole(), PLUGIN_NAME::getIconPixmap("artwork"));
	setPalette(pal);
	setFixedSize(400, 400);


	Knob * fbDecayKnob = new Knob(KnobType::Bright26, this);
	fbDecayKnob->setVolumeKnob(false);
	fbDecayKnob->setModel(&m_instrument->m_feedbackDecayModel);
	fbDecayKnob->move(23, 195);

	Knob * impAddKnob = new Knob(KnobType::Bright26, this);
	impAddKnob->setVolumeKnob(false);
	impAddKnob->setModel(&m_instrument->m_impulseAddModel);
	impAddKnob->move(23, 235);

	Knob * impAttackKnob = new Knob(KnobType::Bright26, this);
	impAttackKnob->setVolumeKnob(false);
	impAttackKnob->setModel(&m_instrument->m_impulseAttackModel);
	impAttackKnob->move(23, 265);

	Knob * impDecayKnob = new Knob(KnobType::Bright26, this);
	impDecayKnob->setVolumeKnob(false);
	impDecayKnob->setModel(&m_instrument->m_impulseDecayModel);
	impDecayKnob->move(23, 295);

	Knob * impSustainKnob = new Knob(KnobType::Bright26, this);
	impSustainKnob->setVolumeKnob(false);
	impSustainKnob->setModel(&m_instrument->m_impulseSustainModel);
	impSustainKnob->move(23, 325);

	Knob * impReleaseKnob = new Knob(KnobType::Bright26, this);
	impReleaseKnob->setVolumeKnob(false);
	impReleaseKnob->setModel(&m_instrument->m_impulseReleaseModel);
	impReleaseKnob->move(23, 355);

	m_parameterWidget = new FBSynthParameterWidget(this, instrument);
	m_parameterWidget->move(18, 17);

	setBand(0, &m_instrument->m_hpActiveModel, &m_instrument->m_hpFreqModel, &m_instrument->m_hpResModel, 0, QColor(255, 255, 255), tr("HP"), &m_instrument->m_hp12Model, &m_instrument->m_hp24Model, &m_instrument->m_hp48Model, 0, 0, 0);
	setBand(1, &m_instrument->m_lowShelfActiveModel, &m_instrument->m_lowShelfFreqModel, &m_instrument->m_lowShelfResModel, &m_instrument->m_lowShelfGainModel, QColor(255, 255, 255), tr("Low-shelf"), 0, 0, 0, 0, 0, 0);
	setBand(2, &m_instrument->m_para1ActiveModel, &m_instrument->m_para1FreqModel, &m_instrument->m_para1BwModel, &m_instrument->m_para1GainModel, QColor(255, 255, 255), tr("Peak 1"), 0, 0, 0, 0, 0, 0);
	setBand(3, &m_instrument->m_para2ActiveModel, &m_instrument->m_para2FreqModel, &m_instrument->m_para2BwModel, &m_instrument->m_para2GainModel, QColor(255, 255, 255), tr("Peak 2"), 0, 0, 0, 0, 0, 0);
	setBand(4, &m_instrument->m_para3ActiveModel, &m_instrument->m_para3FreqModel, &m_instrument->m_para3BwModel, &m_instrument->m_para3GainModel, QColor(255, 255, 255), tr("Peak 3"), 0, 0, 0, 0, 0, 0);
	setBand(5, &m_instrument->m_para4ActiveModel, &m_instrument->m_para4FreqModel, &m_instrument->m_para4BwModel, &m_instrument->m_para4GainModel, QColor(255, 255, 255), tr("Peak 4"), 0, 0, 0, 0, 0, 0);
	setBand(6, &m_instrument->m_highShelfActiveModel, &m_instrument->m_highShelfFreqModel, &m_instrument->m_highShelfResModel, &m_instrument->m_highShelfGainModel, QColor(255, 255, 255), tr("High-shelf"), 0, 0, 0, 0, 0, 0);
	setBand(7, &m_instrument->m_lpActiveModel, &m_instrument->m_lpFreqModel, &m_instrument->m_lpResModel, 0, QColor(255, 255, 255), tr("LP"), 0, 0, 0, &m_instrument->m_lp12Model, &m_instrument->m_lp24Model, &m_instrument->m_lp48Model);

	QPixmap * faderBg = new QPixmap(PLUGIN_NAME::getIconPixmap("faderback"));
	QPixmap * faderLeds = new QPixmap(PLUGIN_NAME::getIconPixmap("faderleds"));
	QPixmap * faderKnob = new QPixmap(PLUGIN_NAME::getIconPixmap("faderknob"));

	// Gain Fader for each Filter exepts the pass filter
	int distance = 92;
	for (int i = 1; i < m_parameterWidget->bandCount() - 1; i++)
	{
		FBSynthFader * gainFader = new FBSynthFader(m_parameterWidget->getBandModels(i)->gain, tr(""), this, faderBg, faderLeds, faderKnob);
		gainFader->move(distance, 225);
		distance += 33;
		gainFader->setMinimumHeight(80);
		gainFader->resize(gainFader->width(), 80);
		gainFader->setDisplayConversion(false);
		gainFader->setHintText(tr("Gain"), "dB");
	}

	//Control Button and Knobs for each Band
	distance = 60;
	for (int i = 0; i < m_parameterWidget->bandCount() ; i++)
	{
		Knob * resKnob = new Knob(KnobType::Bright26, this);
		resKnob->move(distance, 330);
		resKnob->setVolumeKnob(false);
		resKnob->setModel(m_parameterWidget->getBandModels(i)->res);
		if (i > 1 && i < 6) { resKnob->setHintText(tr("Bandwidth: "), tr(" Octave")); }
		else { resKnob->setHintText(tr("Resonance : "), ""); }

		Knob * FreqKnob = new Knob(KnobType::Bright26, this);
		FreqKnob->move(distance, 300);
		FreqKnob->setVolumeKnob(false);
		FreqKnob->setModel(m_parameterWidget->getBandModels(i)->freq);
		FreqKnob->setHintText(tr("Frequency:"), "Hz");

		// adds the Number Active buttons
		PixmapButton * activeButton = new PixmapButton(this, NULL);
		activeButton->setCheckable(true);
		activeButton->setModel(m_parameterWidget->getBandModels(i)->active);

		QString iconActiveFileName = "bandLabel" + QString::number(i+1);
		QString iconInactiveFileName = "bandLabel" + QString::number(i+1) + "off";
		activeButton->setActiveGraphic(PLUGIN_NAME::getIconPixmap(iconActiveFileName.toLatin1()));
		activeButton->setInactiveGraphic(PLUGIN_NAME::getIconPixmap(iconInactiveFileName.toLatin1()));
		activeButton->move(distance - 2, 208);
		activeButton->setModel(m_parameterWidget->getBandModels(i)->active);

		// Connects the knobs, Faders and buttons with the curve graphic
		QObject::connect(m_parameterWidget->getBandModels(i)->freq, SIGNAL(dataChanged()), m_parameterWidget, SLOT (updateHandle()));
		if (m_parameterWidget->getBandModels(i)->gain) QObject::connect(m_parameterWidget->getBandModels(i)->gain, SIGNAL(dataChanged()), m_parameterWidget, SLOT (updateHandle()));
		QObject::connect(m_parameterWidget->getBandModels(i)->res, SIGNAL(dataChanged()), m_parameterWidget, SLOT (updateHandle()));
		QObject::connect(m_parameterWidget->getBandModels(i)->active, SIGNAL(dataChanged()), m_parameterWidget, SLOT (updateHandle()));

		m_parameterWidget->changeHandle(i);
		distance += 33;
	}

	//hp filter type
	PixmapButton * hp12Button = new PixmapButton(this, NULL);
	hp12Button->setModel(m_parameterWidget->getBandModels(0)->hp12);
	hp12Button->setActiveGraphic(PLUGIN_NAME::getIconPixmap("12dB"));
	hp12Button->setInactiveGraphic( PLUGIN_NAME::getIconPixmap("12dBoff"));
	hp12Button->move(58, 233);
	PixmapButton * hp24Button = new PixmapButton(this, NULL);
	hp24Button->setModel(m_parameterWidget->getBandModels(0)->hp24);
	hp24Button->setActiveGraphic(PLUGIN_NAME::getIconPixmap("24dB"));
	hp24Button->setInactiveGraphic( PLUGIN_NAME::getIconPixmap("24dBoff"));

	hp24Button->move(58, 258);
	PixmapButton * hp48Button = new PixmapButton(this, NULL);
	hp48Button->setModel(m_parameterWidget->getBandModels(0)->hp48);
	hp48Button->setActiveGraphic(PLUGIN_NAME::getIconPixmap("48dB"));
	hp48Button->setInactiveGraphic( PLUGIN_NAME::getIconPixmap("48dBoff"));

	hp48Button->move(58, 283);
	//LP filter type
	PixmapButton * lp12Button = new PixmapButton(this, NULL);
	lp12Button->setModel(m_parameterWidget->getBandModels(7)->lp12);
	lp12Button->setActiveGraphic(PLUGIN_NAME::getIconPixmap("12dB"));
	lp12Button->setInactiveGraphic( PLUGIN_NAME::getIconPixmap("12dBoff"));

	lp12Button->move(289, 233);
	PixmapButton * lp24Button = new PixmapButton(this, NULL);
	lp24Button->setModel(m_parameterWidget->getBandModels(7)->lp24);
	lp24Button->setActiveGraphic(PLUGIN_NAME::getIconPixmap("24dB"));
	lp24Button->setInactiveGraphic( PLUGIN_NAME::getIconPixmap("24dBoff"));

	lp24Button->move(289, 258);

	PixmapButton * lp48Button = new PixmapButton(this, NULL);
	lp48Button->setModel(m_parameterWidget->getBandModels(7)->lp48);
	lp48Button->setActiveGraphic(PLUGIN_NAME::getIconPixmap("48dB"));
	lp48Button->setInactiveGraphic( PLUGIN_NAME::getIconPixmap("48dBoff"));

	lp48Button->move(289, 283);
	// the curve has to change its appearance
	QObject::connect(m_parameterWidget->getBandModels(0)->hp12, SIGNAL (dataChanged()), m_parameterWidget, SLOT(updateHandle()));
	QObject::connect(m_parameterWidget->getBandModels(0)->hp24, SIGNAL (dataChanged()), m_parameterWidget, SLOT(updateHandle()));
	QObject::connect(m_parameterWidget->getBandModels(0)->hp48, SIGNAL (dataChanged()), m_parameterWidget, SLOT(updateHandle()));

	QObject::connect(m_parameterWidget->getBandModels(7)->lp12, SIGNAL (dataChanged()), m_parameterWidget, SLOT(updateHandle()));
	QObject::connect(m_parameterWidget->getBandModels(7)->lp24, SIGNAL (dataChanged()), m_parameterWidget, SLOT(updateHandle()));
	QObject::connect(m_parameterWidget->getBandModels(7)->lp48, SIGNAL (dataChanged()), m_parameterWidget, SLOT(updateHandle()));

	automatableButtonGroup *lpBtnGrp = new automatableButtonGroup(this, tr("LP group"));
	lpBtnGrp->addButton(lp12Button);
	lpBtnGrp->addButton(lp24Button);
	lpBtnGrp->addButton(lp48Button);
	lpBtnGrp->setModel(&m_instrument->m_lpTypeModel, false);

	automatableButtonGroup *hpBtnGrp = new automatableButtonGroup(this, tr("HP group"));
	hpBtnGrp->addButton(hp12Button);
	hpBtnGrp->addButton(hp24Button);
	hpBtnGrp->addButton(hp48Button);
	hpBtnGrp->setModel(&m_instrument->m_hpTypeModel, false);
}


FBSynthBand* FBSynthInstrumentView::setBand(int index, BoolModel* active, FloatModel* freq, FloatModel* res, FloatModel* gain, QColor color, QString name, BoolModel* hp12, BoolModel* hp24, BoolModel* hp48, BoolModel* lp12, BoolModel* lp24, BoolModel* lp48)
{
	FBSynthBand *filterModels = m_parameterWidget->getBandModels(index);
	filterModels->active = active;
	filterModels->freq = freq;
	filterModels->res = res;
	filterModels->color = color;
	filterModels->gain = gain;
	filterModels->hp12 = hp12;
	filterModels->hp24 = hp24;
	filterModels->hp48 = hp48;
	filterModels->lp12 = lp12;
	filterModels->lp24 = lp24;
	filterModels->lp48 = lp48;
	return filterModels;
}

} // namespace gui

} // namespace lmms
