/*
 * TimeInputDialog.h - dialog for input playback time
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

#ifndef TIMEINPUTDIALOG_H
#define TIMEINPUTDIALOG_H

#include <QDialog>
#include <QPair>

#include "Song.h"
#include "ui_timeinputdialog.h"

namespace Ui
{
class TimeInputDialog;
}
class TimeDisplayWidget;
class TimeInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimeInputDialog(QWidget *parent = nullptr);
    ~TimeInputDialog();
    void setTimeModel(int time_mode);
    void setMilliSeconds(tick_t milliseconds);
    tick_t getTicks();  // User input

private:
    enum DisplayModes
    {
        MinutesSeconds,
        BarsTicks,
        DisplayModeCount
    };
    typedef DisplayModes DisplayMode;

    Ui::TimeInputDialog *ui;
    tick_t millisecsToTicks(tick_t milliseconds, int tempo);
    tick_t totalTicks(int bars, int beats, int ticks);
    tick_t totalMilliseconds(int mins, int secs, int milli);
    int m_timemode;
    int64_t m_milliseconds;
    typedef QPair<int, int> range;
    void setSpinRange(range Major, range Minor, range Milli);
};

#endif // TIMEINPUTDIALOG_H