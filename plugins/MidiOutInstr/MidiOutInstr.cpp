/*
 * MidiOutInstr.cpp - MIDI output plugin which supports CC
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

#include "MidiOutInstr.h"

#include <QGridLayout>
#include <QHBoxLayout>

#include "Engine.h"
#include "InstrumentTrack.h"
#include "MidiEvent.h"
#include "NotePlayHandle.h"
#include "PixmapButton.h"
#include "Song.h"

#include "embed.h"

#include "plugin_export.h"

namespace lmms
{

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT midioutinstr_plugin_descriptor =
{
	LMMS_STRINGIFY(PLUGIN_NAME),
	"MidiOutInstr",
	QT_TRANSLATE_NOOP("pluginBrowser",
				"MIDI output plugin which supports CC"),
	"Copyright (c) 2018 Hyunjin Song <tteu.ingog@gmail.com>",
	0x0100,
	Plugin::Type::Instrument,
	new PluginPixmapLoader("logo"),
	NULL,
	NULL
} ;

}

MidiOutInstr::MidiOutInstr(InstrumentTrack* instrumentTrack) :
	Instrument(instrumentTrack,&midioutinstr_plugin_descriptor)
{
	for (int i = 0; i < NUM_CONTROLS; ++i)
	{
		m_channelsCc[i] = std::make_unique<IntModel>(0, 0, MidiChannelCount, this, tr("CH%1").arg(i));
		m_controlsCc[i] = std::make_unique<IntModel>(0, 0, MidiControllerCount - 1, this, tr("CC%1").arg(i));
		m_valuesCc[i] = std::make_unique<IntModel>(0, 0, MidiMaxControllerValue, this, tr("VAL%1").arg(i));
		QObject::connect(m_valuesCc[i].get(), &Model::dataChanged, [=] ()
		{
			const int ch = m_channelsCc[i]->value() - 1;
			const int cc = m_controlsCc[i]->value();
			const int val = m_valuesCc[i]->value();
			if (ch >= 0)
			{
				this->instrumentTrack()->processOutEvent(
						MidiEvent(MidiControlChange, ch, cc, val));
			}
		});
	}
}

MidiOutInstr::~MidiOutInstr() {}

void MidiOutInstr::saveSettings(QDomDocument& doc, QDomElement& parent)
{
	for (int i = 0; i < NUM_CONTROLS; ++i)
	{
		m_channelsCc[i]->saveSettings(doc, parent, QString("channel%1").arg(i));
		m_controlsCc[i]->saveSettings(doc, parent, QString("control%1").arg(i));
		m_valuesCc[i]->saveSettings(doc, parent, QString("value%1").arg(i));
	}
}

void MidiOutInstr::loadSettings(const QDomElement& e)
{
	for (int i = 0; i < NUM_CONTROLS; ++i)
	{
		m_channelsCc[i]->loadSettings(e, QString("channel%1").arg(i));
		m_controlsCc[i]->loadSettings(e, QString("control%1").arg(i));
		m_valuesCc[i]->loadSettings(e, QString("value%1").arg(i));
	}
}


QString MidiOutInstr::nodeName() const
{
	return midioutinstr_plugin_descriptor.name;
}

void MidiOutInstr::playNote(NotePlayHandle *, sampleFrame *) {}

void MidiOutInstr::deleteNotePluginData(NotePlayHandle *) {}

gui::PluginView* MidiOutInstr::instantiateView(QWidget* parent)
{
	return new gui::MidiOutInstrView(this, parent);
}

namespace gui
{

MidiOutInstrView::MidiOutInstrView(Instrument* instrument,
					QWidget* parent) :
	InstrumentView(instrument, parent)
{
	QGridLayout* layout = new QGridLayout(this);
	layout->setContentsMargins(0, 0, 0, 30);
	setAutoFillBackground(true);
	QPalette pal;

	pal.setBrush(backgroundRole(), PLUGIN_NAME::getIconPixmap("artwork"));
	setPalette(pal);

	for (int i = 0; i < ROWS; ++i)
	{
		for (int j = 0; j < COLS; ++j)
		{
			QHBoxLayout* subLayout = new QHBoxLayout();
			m_channelsCcLcd[i * COLS + j] = new LcdSpinBox(2, nullptr);
			m_channelsCcLcd[i * COLS + j]->addTextForValue( 0, "--" );
			subLayout->addWidget(m_channelsCcLcd[i * COLS + j]);

			m_controlsCcLcd[i * COLS + j] = new LcdSpinBox(3, nullptr);
			subLayout->addWidget(m_controlsCcLcd[i * COLS + j]);

			m_valuesCcLcd[i * COLS + j] = new LcdSpinBox(3, nullptr);
			subLayout->addWidget(m_valuesCcLcd[i * COLS + j]);

			layout->addLayout(subLayout, i, j, Qt::AlignCenter);
		}
	}
}

void MidiOutInstrView::modelChanged()
{
	MidiOutInstr* mo = castModel<MidiOutInstr>();

	for (int i = 0; i < NUM_CONTROLS; ++i)
	{
		m_channelsCcLcd[i]->setModel(mo->m_channelsCc[i].get());
		m_controlsCcLcd[i]->setModel(mo->m_controlsCc[i].get());
		m_valuesCcLcd[i]->setModel(mo->m_valuesCc[i].get());
	}
}

} // namespace gui

extern "C"
{

// necessary for getting instance out of shared lib
PLUGIN_EXPORT Plugin* lmms_plugin_main(Model * model, void*)
{
	return new MidiOutInstr(static_cast<InstrumentTrack *>(model));
}

}

} // namespace lmms
