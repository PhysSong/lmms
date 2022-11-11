/*
 * ControllerRackView.h - view for song's controllers
 *
 * Copyright (c) 2008-2009 Paul Giblock <drfaygo/at/gmail.com>
 * Copyright (c) 2019 Steffen Baranowsky <BaraMGB/at/freenet.de>
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

#ifndef CONTROLLER_RACK_VIEW_H
#define CONTROLLER_RACK_VIEW_H

#include <QCloseEvent>
#include <QMdiSubWindow>
#include <QWidget>

#include "lmms_basics.h"
#include "SerializingObject.h"


class QPushButton;
class QScrollArea;
class QVBoxLayout;


namespace lmms
{

class Controller;

namespace gui
{

class ControllerView;


class LMMS_EXPORT ControllerRackView : public QWidget, public SerializingObject
{
	Q_OBJECT
public:
	ControllerRackView();
	~ControllerRackView() override = default;

	void saveSettings( QDomDocument & _doc, QDomElement & _parent ) override;
	void loadSettings( const QDomElement & _this ) override;

	inline QString nodeName() const override
	{
		return "ControllerRackView";
	}

	QMdiSubWindow *subWin() const;

	bool allExpanded() const;
	bool allCollapsed() const;

public slots:
	void deleteController(lmms::gui::ControllerView * view);
	void collapsingAll();
	void expandAll();
	void onControllerAdded(lmms::Controller *);
	void onControllerRemoved(lmms::Controller *);
	void onControllerCollapsed();

	const QVector<ControllerView *> controllerViews() const;

protected:
	void closeEvent(QCloseEvent * ce) override;
	void resizeEvent(QResizeEvent *) override;
	void paintEvent(QPaintEvent *) override;

private slots:
	void addLfoController();
	void moveControllerUp(lmms::gui::ControllerView * cv);
	void moveControllerDown(lmms::gui::ControllerView * cv);

private:
	QVector<ControllerView *> m_controllerViews;
	QVector<bool> m_collapsingStateOnLoad;

	QScrollArea * m_scrollArea;
	QVBoxLayout * m_scrollAreaLayout;
	QPushButton * m_addButton;
	QMdiSubWindow * m_subWin;
};

} // namespace gui

} // namespace lmms

#endif
