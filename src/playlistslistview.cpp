/**
 *  This file is a part of ncxmms2, an XMMS2 Client.
 *
 *  Copyright (C) 2011 Pavel Kunavin <tusk.kun@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <algorithm>
#include "playlistslistview.h"
#include "lib/painter.h"

using namespace ncxmms2;

PlaylistsListView::PlaylistsListView(Xmms::Client* xmmsClient, int lines, int cols, int yPos, int xPos, Window* parent) :
	AbstractItemView(lines, cols, yPos, xPos, parent),
	m_xmmsClient(xmmsClient)
{
	m_xmmsClient->playlist.list()(Xmms::bind(&PlaylistsListView::getPlaylists, this));
	m_xmmsClient->playlist.currentActive()(Xmms::bind(&PlaylistsListView::getCurrentPlaylist, this));
	m_xmmsClient->playlist.broadcastLoaded()(Xmms::bind(&PlaylistsListView::getCurrentPlaylist, this));
	m_xmmsClient->collection.broadcastCollectionChanged()(Xmms::bind(&PlaylistsListView::handlePlaylistsChange, this));
}

bool PlaylistsListView::getPlaylists(const Xmms::List<std::string>& playlists)
{
	m_playlists.clear();
	for (Xmms::List<std::string>::const_iterator it=playlists.begin(), it_end=playlists.end(); it!=it_end; ++it)
	{
		const std::string& playlist=*it;
		if (playlist.empty() || playlist[0]=='_')
			continue;
		
		m_playlists.push_back(playlist);
	}
	
	reset();
	return true;
}

bool PlaylistsListView::getCurrentPlaylist(const std::string& playlist)
{
	auto it=std::find(m_playlists.begin(), m_playlists.end(), m_currentPlaylist);
	m_currentPlaylist=playlist;
	if (it!=m_playlists.end())
		redrawItem(it-m_playlists.begin());
	
	it=std::find(m_playlists.begin(), m_playlists.end(), m_currentPlaylist);
	if (it!=m_playlists.end())
		redrawItem(it-m_playlists.begin());
	
	return true;
}

bool PlaylistsListView::handlePlaylistsChange(const Xmms::Dict& change)
{
	if (change.get<std::string>("namespace")!=XMMS_COLLECTION_NS_PLAYLISTS)
		return true;
	
	switch (change.get<int>("type")) {
		case XMMS_COLLECTION_CHANGED_ADD:
			m_playlists.push_back(change.get<std::string>("name"));
			itemAdded();
			break;
			
		case XMMS_COLLECTION_CHANGED_RENAME:
		{
			const std::string name=change.get<std::string>("name");
			const std::string newName=change.get<std::string>("newname");
			
			if (m_currentPlaylist==name)
				m_currentPlaylist=newName;
			
			auto it=std::find(m_playlists.begin(), m_playlists.end(), name);
			if (it!=m_playlists.end()) {
				(*it)=newName;
				redrawItem(it-m_playlists.begin());
			}
			
			break;
		}
		
		case XMMS_COLLECTION_CHANGED_REMOVE:
		{
			auto it=std::find(m_playlists.begin(), m_playlists.end(), change.get<std::string>("name"));
			if (it!=m_playlists.end()) {
				m_playlists.erase(it);
				itemRemoved(it-m_playlists.begin());
			}
			
			break;
		}
		case XMMS_COLLECTION_CHANGED_UPDATE:
			break;
	}
	
	return true;
}

void PlaylistsListView::drawItem(int item)
{
	Painter painter(this);

	if (item==currentItem()) {
		painter.fillLine(itemLine(item), hasFocus() ? ColorYellow : ColorWhite);
		painter.setColor(hasFocus() ? ColorYellow : ColorWhite);
		painter.setReverse(true);
	} else {
		painter.setColor(ColorYellow );
		painter.clearLine(itemLine(item));
	}
	
	if (m_playlists[item]==m_currentPlaylist && item!=currentItem()) {
		painter.setBold(true);
	}
	
	painter.squeezedPrint(m_playlists[item], cols());
}

int PlaylistsListView::itemsCount() const
{
	return m_playlists.size();
}

std::string PlaylistsListView::playlist(int item) const
{
	return item>=0 && (std::vector<std::string>::size_type)item<m_playlists.size() 
	       ? m_playlists[item] 
	       : std::string();
}

void PlaylistsListView::itemEntered(int item)
{
	m_xmmsClient->playlist.load(m_playlists[item]);
}

void PlaylistsListView::keyPressedEvent(const KeyEvent &keyEvent)
{
	if (keyEvent.key()==KeyEvent::KeyDelete && itemsCount()>1) {
		m_xmmsClient->playlist.remove(m_playlists[currentItem()]);
	} else {
		AbstractItemView::keyPressedEvent(keyEvent);
	}
}