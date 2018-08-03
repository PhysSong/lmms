/* FBSynthfader.h - defination of FBSynthFader class.
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

#ifndef EQFADER_H
#define EQFADER_H
#include <QList>
#include <QWidget>

#include "EffectControls.h"
#include "Fader.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "TextFloat.h"

namespace lmms
{

namespace gui
{

class FBSynthFader : public Fader
{
public:
	FBSynthFader(FloatModel * model, const QString & name, QWidget * parent, QPixmap * backg, QPixmap * leds, QPixmap * knobpi) :
		Fader(model, name, parent, backg, leds, knobpi)
	{
		setMinimumSize(23, 80);
		setMaximumSize(23, 80);
		resize(23, 80);
		setPeak_L(0);
		setPeak_R(0);
	}

	FBSynthFader(FloatModel * model, const QString & name, QWidget * parent) :
		Fader(model, name, parent)
	{
		setMinimumSize(23, 116);
		setMaximumSize(23, 116);
		resize(23, 116);
		setPeak_L(0);
		setPeak_R(0);
	}

	~FBSynthFader() = default;
};

} // namespace gui

} // namespace lmms

#endif // EQFADER_H
