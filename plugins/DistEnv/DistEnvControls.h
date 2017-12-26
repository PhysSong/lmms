/*
 * DistEnvControls.h - controls for bassboosterx -effect
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef DISTENV_CONTROLS_H
#define DISTENV_CONTROLS_H

#include "EffectControls.h"
#include "DistEnvControlDialog.h"
#include "Knob.h"


namespace lmms
{

class DistEnvEffect;


class DistEnvControls : public EffectControls
{
	Q_OBJECT
public:
	DistEnvControls(DistEnvEffect* effect);
	virtual ~DistEnvControls()
	{
	}

	virtual void saveSettings(QDomDocument & doc, QDomElement & elem);
	virtual void loadSettings(const QDomElement & elem);
	inline virtual QString nodeName() const
	{
		return "DistEnvControls";
	}

	virtual int controlCount()
	{
		return 5;
	}

	virtual gui::EffectControlDialog* createView()
	{
		return new gui::DistEnvControlDialog(this);
	}


private slots:
	void changeControl();

private:
	DistEnvEffect* m_effect;
	FloatModel m_ratioModel;
	FloatModel m_envAmountModel;
	FloatModel m_attackModel;
	FloatModel m_releaseModel;
	FloatModel m_envModeModel;
	FloatModel m_gainModel;

	friend class gui::DistEnvControlDialog;
	friend class DistEnvEffect;

};
} // namespace lmms

#endif
