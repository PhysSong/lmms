/*
 * MidiOutInstr.h - MIDI output plugin which supports CC
 *
 * Copyright (c) 2018 Hyunjin Song <tteu.ingog@gmail.com>
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


#ifndef MIDIOUTINSTR_H
#define MIDIOUTINSTR_H

#include <QDomElement>

#include <memory>

#include "AutomatableModel.h"
#include "Instrument.h"
#include "InstrumentView.h"
#include "LcdSpinBox.h"
#include "MemoryManager.h"

namespace lmms
{

constexpr int ROWS = 8;
constexpr int COLS = 2;
constexpr int NUM_CONTROLS = ROWS * COLS;

namespace gui
{
class MidiOutInstrView;
}

class MidiOutInstr : public Instrument
{
	Q_OBJECT
public:
	MidiOutInstr(InstrumentTrack* instrumentTrack);
	virtual ~MidiOutInstr();

	virtual void playNote(NotePlayHandle*, sampleFrame*);
	virtual void deleteNotePluginData(NotePlayHandle*);


	virtual void saveSettings(QDomDocument& doc, QDomElement& parent);
	virtual void loadSettings(const QDomElement& e);

	virtual QString nodeName() const;

	virtual f_cnt_t desiredReleaseFrames() const
	{
		return 0;
	}

	virtual gui::PluginView* instantiateView(QWidget* parent);

private:
	std::unique_ptr<IntModel> m_channelsCc[NUM_CONTROLS];
	std::unique_ptr<IntModel> m_controlsCc[NUM_CONTROLS];
	std::unique_ptr<IntModel> m_valuesCc[NUM_CONTROLS];

	friend class gui::MidiOutInstrView;
};

namespace gui
{

class MidiOutInstrView : public InstrumentView
{
	Q_OBJECT
public:
	MidiOutInstrView(Instrument* instrument, QWidget* parent);
	QSize sizeHint() const override { return QSize(344, 364); }

	virtual ~MidiOutInstrView() {};

private:
	virtual void modelChanged();

	LcdSpinBox* m_channelsCcLcd[NUM_CONTROLS];
	LcdSpinBox* m_controlsCcLcd[NUM_CONTROLS];
	LcdSpinBox* m_valuesCcLcd[NUM_CONTROLS];

	static QPixmap* s_artwork;

};

} // namespace gui

} // namespace lmms

#endif
