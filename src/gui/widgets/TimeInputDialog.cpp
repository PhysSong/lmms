/*
 * TimeInputDialog.cpp - dialog for input playback time
 *
 * Copyright (c) 2016 liushuyu <liushuyu011/at/gmail/dot/com>
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
#include "TimeDisplayWidget.h"
#include "TimeInputDialog.h"

TimeInputDialog::TimeInputDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TimeInputDialog)
{
	ui->setupUi(this);
}

TimeInputDialog::~TimeInputDialog()
{
	delete ui;
}

void TimeInputDialog::setTimeModel(int time_mode)
{
	m_timemode = time_mode;
	switch (m_timemode)
	{
	case MinutesSeconds:
		ui->majorLabel->setText(TimeDisplayWidget::tr("MIN"));
		ui->minorLabel->setText(TimeDisplayWidget::tr("SEC"));
		ui->milliLabel->setText(TimeDisplayWidget::tr("MSEC"));
		setSpinRange(range(0, 9999), range(0, 59), range(0, 999));
		break;

	case BarsTicks:
		ui->majorLabel->setText(TimeDisplayWidget::tr("BAR"));
		ui->minorLabel->setText(TimeDisplayWidget::tr("BEAT"));
		ui->milliLabel->setText(TimeDisplayWidget::tr("TICK"));
		setSpinRange(range(1, 9999),
			range(1, Engine::getSong()->getTimeSigModel().getNumerator()),
			range(0, Engine::getSong()->ticksPerBar() / Engine::getSong()->getTimeSigModel().getNumerator() - 1));
		break;

	default:
		break;
	}
	return;
}

void TimeInputDialog::setMilliSeconds(tick_t milliseconds)
{
	m_milliseconds = ((milliseconds > 0) ? milliseconds : 0);
	tick_t ticks;
	switch (m_timemode) {
	case MinutesSeconds:
		ui->majorInput->setValue(m_milliseconds / 60000);
		ui->minorInput->setValue((m_milliseconds / 1000) % 60);
		ui->milliInput->setValue(m_milliseconds % 1000);
		break;
	case BarsTicks:
		ticks = millisecsToTicks(m_milliseconds, Engine::getSong()->getTempo());
		ui->majorInput->setValue(ticks / Engine::getSong()->ticksPerBar() + 1);
		ui->minorInput->setValue((ticks % Engine::getSong()->ticksPerBar()) /
					(Engine::getSong()->ticksPerBar() / Engine::getSong()->getTimeSigModel().getNumerator()) +1);
		ui->milliInput->setValue((ticks % Engine::getSong()->ticksPerBar()) %
					(Engine::getSong()->ticksPerBar() / Engine::getSong()->getTimeSigModel().getNumerator()));
	default:
		break;
	}
}

tick_t TimeInputDialog::millisecsToTicks(tick_t milliseconds, int tempo)
{
	return ((milliseconds * tempo * (DefaultTicksPerBar / 4)) / 60000);
}

tick_t TimeInputDialog::totalTicks(int bars, int beats, int ticks)
{
	tick_t ticksTotal = 0;
	ticksTotal += (bars - 1) * Engine::getSong()->ticksPerBar();
	ticksTotal += ((beats -1) * Engine::getSong()->ticksPerBar())
				/ Engine::getSong()->getTimeSigModel().getNumerator();
	ticksTotal += ticks;
	return ticksTotal;
}

tick_t TimeInputDialog::totalMilliseconds(int mins, int secs, int milli)
{
	return (mins * 60000 + secs * 1000 + milli);
}

void TimeInputDialog::setSpinRange(range Major, range Minor, range Milli)
{
	ui->majorInput->setRange(Major.first, Major.second);
	ui->minorInput->setRange(Minor.first, Minor.second);
	ui->milliInput->setRange(Milli.first, Milli.second);
}

tick_t TimeInputDialog::getTicks()
{
	tick_t ticks = 0;
	switch (m_timemode)
	{
	case MinutesSeconds:
		ticks = millisecsToTicks(
			totalMilliseconds(ui->majorInput->value(), ui->minorInput->value(), ui->milliInput->value()),
			Engine::getSong()->getTempo());
		return (ticks > 0 ? ticks : 0);  // Try to prevent from overflow
		break;

	case BarsTicks:
		ticks = totalTicks(ui->majorInput->value(), ui->minorInput->value(), ui->milliInput->value());
		return (ticks > 0 ? ticks : 0);

	default:
		return 0;
		break;
	}
}