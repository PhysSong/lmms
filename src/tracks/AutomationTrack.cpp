/*
 * AutomationTrack.cpp - AutomationTrack handles automation of objects without
 *                       a track
 *
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2006-2008 Javier Serrano Polo <jasp00/at/users.sourceforge.net>
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

#include "AutomationTrack.h"
#include "AutomationPattern.h"
#include "Engine.h"
#include "embed.h"
#include "ProjectJournal.h"
#include "StringPairDrag.h"
#include "TrackContainerView.h"
#include "TrackLabelButton.h"
#include "Song.h"
#include "TextFloat.h"
#include "BBTrackContainer.h"


AutomationTrack::AutomationTrack( TrackContainer* tc, bool _hidden, bool tempo ) :
	Track( _hidden ? HiddenAutomationTrack : ( tempo ? Track::TempoTrack : Track::AutomationTrack ), tc ),
	m_processHandle( new AutomationProcessHandle( this ) ),
	m_objects()
{
	setName( defaultName() );
}


AutomationTrack::~AutomationTrack()
{
}


ProcessHandle * AutomationTrack::getProcessHandle()
{
	return static_cast<ProcessHandle *>( m_processHandle );
}


void AutomationProcessHandle::doProcessing()
{
	int tcoNum = Engine::getSong()->playingTcoNum();
	const tick_t start = Engine::getSong()->periodStartTick();
	const tick_t end = Engine::getSong()->getTicks();
	// TickOffsetHash * ticks = Engine::getSong()->ticksThisPeriod(); // not yet...

	if( m_track->isMuted() )
	{
		return;
	}

	tcoVector tcos;
	if( tcoNum >= 0 )
	{
		TrackContentObject * tco = m_track->getTCO( tcoNum );
		tcos.push_back( tco );
	}
	else
	{
		m_track->getTCOsInRange( tcos, start, end );
	}

	for( tcoVector::iterator it = tcos.begin(); it != tcos.end(); ++it )
	{
		AutomationPattern * p = dynamic_cast<AutomationPattern *>( *it );
		if( p == NULL || ( *it )->isMuted() )
		{
			continue;
		}
		MidiTime cur_start = MidiTime( start );
		if( tcoNum < 0 )
		{
			cur_start -= p->startPosition();
		}
		p->processMidiTime( cur_start );
	}
}


void AutomationTrack::addObject( AutomatableModel * obj, bool search_dup )
{
	if( ! obj->automationEnabled() )
	{
		return;
	}
	if( search_dup )
	{
		for( objectVector::iterator it = m_objects.begin();
					it != m_objects.end(); ++it )
		{
			if( *it == obj )
			{
				TextFloat::displayMessage( obj->displayName(), tr( "Model is already connected "
												"to this track." ), embed::getIconPixmap( "automation" ), 2000 );
				return;
			}
		}
	}

	m_objects += obj;
	obj->setAutomation( this );

	connect( obj, SIGNAL( destroyed( jo_id_t ) ),
			this, SLOT( objectDestroyed( jo_id_t ) ),
						Qt::DirectConnection );

	// if the track hasn't been named with a custom name, name it with the model name
	if( name() == defaultName() )
	{
		setName( obj->fullDisplayName() );
	}

	emit dataChanged();
}


void AutomationTrack::removeObject( AutomatableModel * obj )
{
	foreach( AutomatableModel * o, m_objects )
	{
		if( o == obj )
		{
			float oldMin = getMin();
			float oldMax = getMax();

			m_objects.remove( m_objects.indexOf( obj ) );
			obj->removeAutomation();

			// if the last object was removed, scale the values of all patterns to fit the 0..1 scale
			if( m_objects.isEmpty() )
			{
				tcoVector tcos = getTCOs();
				for( tcoVector::iterator it = tcos.begin(); it != tcos.end(); ++it )
				{
					AutomationPattern * ap = dynamic_cast<AutomationPattern *>( *it );
					if( ap )
					{
						ap->scaleTimemapToFit( oldMin, oldMax );
					}
				}
			}
		}
	}
	// reset name of track if it was named after this object
	if( name() == obj->fullDisplayName() )
	{
		if( m_objects.isEmpty() )
		{
			// no more objects, default to default name
			setName( defaultName() );
		}
		else // name by next in line
		{
			setName( firstObject()->fullDisplayName() );
		}
	}
}


const AutomatableModel * AutomationTrack::firstObject() const
{
	AutomatableModel * m;
	if( !m_objects.isEmpty() && ( m = m_objects.first() ) != NULL )
	{
		return m;
	}

	static FloatModel _fm( 0, AutomationPattern::DEFAULT_MIN_VALUE, AutomationPattern::DEFAULT_MAX_VALUE, 0.001 );
	return &_fm;
}


void AutomationTrack::objectDestroyed( jo_id_t id )
{
	// TODO: distict between temporary removal (e.g. LADSPA controls
	// when switching samplerate) and real deletions
	m_idsToResolve += id;

	for( objectVector::Iterator objIt = m_objects.begin();
		objIt != m_objects.end(); objIt++ )
	{
		Q_ASSERT( !(*objIt).isNull() );
		if( (*objIt)->id() == id )
		{
			//Assign to objIt so that this loop work even break; is removed.
			objIt = m_objects.erase( objIt );
			break;
		}
	}

	emit dataChanged();
}


bool AutomationTrack::isAutomated( const AutomatableModel * m )
{
	TrackList l;
	l += Engine::getSong()->tracks();
	l += Engine::getBBTrackContainer()->tracks();

	for( TrackList::ConstIterator it = l.begin(); it != l.end(); ++it )
	{
		if( ( *it )->type() == Track::AutomationTrack ||
			( *it )->type() == Track::HiddenAutomationTrack )
		{
			AutomationTrack * at = dynamic_cast<AutomationTrack *>( *it );
			if( at )
			{
				for( objectVector::const_iterator k = at->objects()->begin(); k != at->objects()->end(); ++k )
				{
					if( *k == m )
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}


void AutomationTrack::resolveAllIDs()
{
	TrackList l = Engine::getSong()->tracks() +
				Engine::getBBTrackContainer()->tracks();
	for( TrackList::iterator it = l.begin();
							it != l.end(); ++it )
	{
		if( ( *it )->type() == Track::AutomationTrack ||
			 ( *it )->type() == Track::HiddenAutomationTrack )
		{
			AutomationTrack * at = dynamic_cast<AutomationTrack *>( *it );
			if( at )
			{
				for( QVector<jo_id_t>::Iterator k = at->m_idsToResolve.begin();
								k != at->m_idsToResolve.end(); ++k )
				{
					JournallingObject * o = Engine::projectJournal()->
													journallingObject( *k );
					if( o && dynamic_cast<AutomatableModel *>( o ) )
					{
						at->addObject( dynamic_cast<AutomatableModel *>( o ), false );
					}
				}
				at->m_idsToResolve.clear();
				at->dataChanged();
			}
		}
	}
}



bool AutomationTrack::play( const MidiTime & _start, const fpp_t _frames,
							const f_cnt_t _frame_base, int _tco_num )
{
	if( isMuted() )
	{
		return false;
	}

	tcoVector tcos;
	if( _tco_num >= 0 )
	{
		TrackContentObject * tco = getTCO( _tco_num );
		tcos.push_back( tco );
	}
	else
	{
		getTCOsInRange( tcos, _start, _start + static_cast<int>(
					_frames / Engine::framesPerTick()) );
	}

	for( tcoVector::iterator it = tcos.begin(); it != tcos.end(); ++it )
	{
		AutomationPattern * p = dynamic_cast<AutomationPattern *>( *it );
		if( p == NULL || ( *it )->isMuted() )
		{
			continue;
		}
		MidiTime cur_start = _start;
		if( _tco_num < 0 )
		{
			cur_start -= p->startPosition();
		}
		p->processMidiTime( cur_start );
	}
	return false;
}




TrackView * AutomationTrack::createView( TrackContainerView* tcv )
{
	return new AutomationTrackView( this, tcv );
}




TrackContentObject * AutomationTrack::createTCO( const MidiTime & pos )
{
	return new AutomationPattern( this, pos );
}




void AutomationTrack::saveTrackSpecificSettings( QDomDocument & _doc,
							QDomElement & _this )
{
	for( objectVector::const_iterator it = m_objects.begin();
						it != m_objects.end(); ++it )
	{
		if( *it )
		{
			QDomElement element = _doc.createElement( "object" );
			element.setAttribute( "id", ( *it )->id() );
			_this.appendChild( element );
		}
	}
}




void AutomationTrack::loadTrackSpecificSettings( const QDomElement & _this )
{
	// just in case something somehow wrent wrong...
	if( type() == HiddenAutomationTrack )
	{
		setMuted( false );
	}
	for( QDomNode node = _this.firstChild(); !node.isNull();
						node = node.nextSibling() )
	{
		QDomElement element = node.toElement();
		if( element.isNull()  )
		{
			continue;
		}
		if( element.tagName() == "object" )
		{
			m_idsToResolve << element.attribute( "id" ).toInt();
		}
	}
}





AutomationTrackView::AutomationTrackView( AutomationTrack * _at, TrackContainerView* tcv ) :
	TrackView( _at, tcv )
{
	setFixedHeight( 32 );
	TrackLabelButton * tlb = new TrackLabelButton( this, getTrackSettingsWidget() );
	tlb->setIcon( embed::getIconPixmap( "automation_track" ) );
	tlb->move( 3, 1 );
	tlb->show();
	setModel( _at );
}




AutomationTrackView::~AutomationTrackView()
{
}




void AutomationTrackView::dragEnterEvent( QDragEnterEvent * _dee )
{
	StringPairDrag::processDragEnterEvent( _dee, "automatable_model" );
}




void AutomationTrackView::dropEvent( QDropEvent * _de )
{
	QString type = StringPairDrag::decodeKey( _de );
	QString val = StringPairDrag::decodeValue( _de );
	if( type == "automatable_model" )
	{
		AutomatableModel * mod = dynamic_cast<AutomatableModel *>(
				Engine::projectJournal()->
					journallingObject( val.toInt() ) );
		if( mod != NULL )
		{
			MidiTime pos = MidiTime( trackContainerView()->
							currentPosition() +
				( _de->pos().x() -
					getTrackContentWidget()->x() ) *
						MidiTime::ticksPerTact() /
		static_cast<int>( trackContainerView()->pixelsPerTact() ) )
				.toAbsoluteTact();

			if( pos.getTicks() < 0 )
			{
				pos.setTicks( 0 );
			}

			TrackContentObject * tco = getTrack()->createTCO( pos );
			AutomationPattern * pat = dynamic_cast<AutomationPattern *>( tco );
			pat->addObject( mod );
			pat->movePosition( pos );
		}
	}

	update();
}


