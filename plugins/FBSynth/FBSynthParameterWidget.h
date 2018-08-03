/*
 * FBSynthparameterwidget.cpp - defination of FBSynthParameterWidget class.
 *
 * Copyright (c) 2014 David French <dave/dot/french3/at/googlemail/dot/com>
* Copyright (c) 2015 Steffen Baranowsky <BaraMGB/at/freenet/dot/de>
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


#ifndef FBSYNTHPARAMETERWIDGET_H
#define FBSYNTHPARAMETERWIDGET_H

#include <QWidget>

#include "FBSynthCurve.h"
#include "FBSynthInstrument.h"
#include "TextFloat.h"

namespace lmms
{

class FBSynthBand
{
public :
	FBSynthBand();
	FloatModel *gain;
	FloatModel *res;
	FloatModel *freq;
	BoolModel *active;
	BoolModel *hp12;
	BoolModel *hp24;
	BoolModel *hp48;
	BoolModel *lp12;
	BoolModel *lp24;
	BoolModel *lp48;
	QColor color;
	int x;
	int y;
	QString name;
	float *peakL;
	float *peakR;
};



namespace gui
{

class FBSynthParameterWidget : public QWidget
{
	Q_OBJECT
public:
	explicit FBSynthParameterWidget(QWidget *parent = nullptr, FBSynthInstrument * instrument = nullptr);
	~FBSynthParameterWidget();
	QList<FBSynthHandle*> *m_handleList;

	const int bandCount()
	{
		return 8;
	}

	FBSynthBand* getBandModels(int i);
	void changeHandle(int i);

private:
	float m_pixelsPerUnitWidth;
	float m_pixelsPerUnitHeight;
	float m_pixelsPerOctave;
	float m_scale;
	int m_displayWidth, m_displayHeigth;
	FBSynthInstrument * m_instrument;
	FBSynthBand *m_bands;
	FBSynthHandle *m_handle;
	FBSynthCurve *m_FBSynthcurve;


private slots:
	void updateModels();
	void updateHandle();
};

} // namespace gui

} // namespace lmms

#endif // FBSYNTHPARAMETERWIDGET_H
