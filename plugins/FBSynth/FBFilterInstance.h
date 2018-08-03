/* FBFilterInstance.h - declaration of FBFilterInstance class.
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

#ifndef FBFILTERINSTANCE_H
#define FBFILTERINSTANCE_H

#include "FBSynthFilter.h"

#include "AutomatableModel.h"
#include "lmms_math.h"

namespace lmms
{

class FBSynthInstrument;

class FBFilterInstance final
{
public:
	FBFilterInstance(FBSynthInstrument *inst) : m_fpp(2), m_frameIdx(0), m_inst(inst) {}
	~FBFilterInstance() = default;

	inline void setFramesPerPeroid(fpp_t fpp)
	{
		m_fpp = fpp;
		m_frameIdx = 0;
	}
	void applyFilter(sampleFrame &buf);

private:

	fpp_t m_fpp;
	fpp_t m_frameIdx;

	FBSynthInstrument *m_inst;

	FBSynthHp12Filter m_hp12;
	FBSynthHp12Filter m_hp24;
	FBSynthHp12Filter m_hp480;
	FBSynthHp12Filter m_hp481;

	FBSynthLowShelfFilter m_lowShelf;

	FBSynthPeakFilter m_para1;
	FBSynthPeakFilter m_para2;
	FBSynthPeakFilter m_para3;
	FBSynthPeakFilter m_para4;

	FBSynthHighShelfFilter m_highShelf;

	FBSynthLp12Filter m_lp12;
	FBSynthLp12Filter m_lp24;
	FBSynthLp12Filter m_lp480;
	FBSynthLp12Filter m_lp481;
};

} // namespace lmms

#endif // FBFILTERINSTANCE_H
