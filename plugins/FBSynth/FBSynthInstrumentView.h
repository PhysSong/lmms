/*
 * FBSynthInstrument.h - definition of FBSynthInstrument class.
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

#ifndef FBSYNTHINSTRUMENTVIEW_H
#define FBSYNTHINSTRUMENTVIEW_H

#include <QLabel>
#include <QPushButton>

#include "FBSynthInstrument.h"
#include "InstrumentView.h"

namespace lmms
{

class BoolModel;
class FloatModel;

class FBSynthBand;

namespace gui
{

class FBSynthParameterWidget;

class FBSynthInstrumentView : public InstrumentView
{
	Q_OBJECT
public:
	FBSynthInstrumentView(FBSynthInstrument * instrument, QWidget * parent);
	QSize sizeHint() const override { return QSize(384, 404); }
	virtual ~FBSynthInstrumentView() = default;


private:
	FBSynthInstrument * m_instrument;
	FBSynthParameterWidget * m_parameterWidget;

	FBSynthBand *setBand(int index, BoolModel *active, FloatModel *freq, FloatModel *res, FloatModel *gain, QColor color, QString name, BoolModel *hp12, BoolModel *hp24, BoolModel *hp48, BoolModel *lp12, BoolModel *lp24, BoolModel *lp48);

	int m_originalHeight;
};

} // namespace gui

} // namespace lmms

#endif // FBSYNTHINSTRUMENTVIEW_H
