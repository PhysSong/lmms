/*
 * NoyzrControls.h - declaration of NoyzrControl class.
 *
 * Copyright (c) 2017 Hyunjin Song <tteu.ingog/at/gmail.com>
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

#ifndef NoyzrCONTROLS_H
#define NoyzrCONTROLS_H

#include "EffectControls.h"
#include "Knob.h"
#include "NoyzrControlsDialog.h"


namespace lmms
{

class NoyzrEffect;

class NoyzrControls : public EffectControls
{
	Q_OBJECT
public:
	NoyzrControls(NoyzrEffect* effect);
	virtual ~NoyzrControls()
	{
	}
	virtual void saveSettings(QDomDocument& doc, QDomElement& de);
	virtual void loadSettings(const QDomElement& de);
	inline virtual QString nodeName() const
	{
		return "Noyzr";
	}
	virtual int controlCount(){
		return 1;
	}
	virtual gui::EffectControlDialog* createView()
	{
		return new gui::NoyzrControlsDialog(this);
	}

private:
	NoyzrEffect* m_effect;
	FloatModel m_noiseAmtModel;

	friend class gui::NoyzrControlsDialog;
	friend class NoyzrEffect;
};

} // namespace lmms

#endif // NoyzrCONTROLS_H
