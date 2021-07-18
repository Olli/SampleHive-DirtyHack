/* SampleHive
 * Copyright (C) 2021  Apoorv Singh
 * A simple, modern audio sample browser/manager for GNU/Linux.
 *
 * This file is a part of SampleHive
 *
 * SampleHive is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>

#include "Tags.hpp"

Tags::Tags(const std::string& filename)
    : m_Filepath(filename)
{

}

Tags::~Tags()
{

}

AudioInfo Tags::GetAudioInfo()
{
    wxString artist, album, genre, title, comment;
    int channels = 0, length = 0, sample_rate = 0, bitrate = 0;

    TagLib::FileRef f (static_cast<const char*>(m_Filepath.c_str()), true, TagLib::AudioProperties::ReadStyle::Average);

    if (!f.isNull() && f.tag() && f.audioProperties())
    {
        TagLib::Tag* tag = f.tag();
        TagLib::AudioProperties* properties = f.audioProperties();

        TagLib::String Title = tag->title();
        TagLib::String Artist = tag->artist();
        TagLib::String Album = tag->album();
        TagLib::String Genre = tag->genre();
        TagLib::String Comment = tag->comment();

        int seconds = properties->length() % 60;
        int minutes = (properties->length() - seconds) / 60;

        bitrate = properties->bitrate();
        channels = properties->channels();
        length = properties->lengthInMilliseconds();
        int length_sec = properties->lengthInSeconds();
        sample_rate = properties->sampleRate();

        title = wxString::FromUTF8(Title.toCString(true));
        artist = wxString::FromUTF8(Artist.toCString(true));
        album = wxString::FromUTF8(Album.toCString(true));
        genre = wxString::FromUTF8(Genre.toCString(true));
        comment = wxString::FromUTF8(Comment.toCString(true));

        bValid = true;
    }
    else
    {
        bValid = false;
    }

    return { title, artist, album, genre, comment, channels, length, sample_rate, bitrate };
}

void Tags::SetTitle(std::string title)
{
    TagLib::FileRef f (static_cast<const char*>(m_Filepath.c_str()), true, TagLib::AudioProperties::ReadStyle::Average);

    if (!f.isNull() && f.tag() && f.audioProperties())
    {
        TagLib::Tag* tag = f.tag();

        tag->setTitle(title);
        f.save();
    }
}

void Tags::SetArtist(std::string artist)
{
    TagLib::FileRef f (static_cast<const char*>(m_Filepath.c_str()), true, TagLib::AudioProperties::ReadStyle::Average);

    if (!f.isNull() && f.tag() && f.audioProperties())
    {
        TagLib::Tag* tag = f.tag();

        tag->setArtist(artist);
        f.save();
    }
}

void Tags::SetAlbum(std::string album)
{
    TagLib::FileRef f (static_cast<const char*>(m_Filepath.c_str()), true, TagLib::AudioProperties::ReadStyle::Average);

    if (!f.isNull() && f.tag() && f.audioProperties())
    {
        TagLib::Tag* tag = f.tag();

        tag->setAlbum(album);
        f.save();
    }
}

void Tags::SetGenre(std::string genre)
{
    TagLib::FileRef f (static_cast<const char*>(m_Filepath.c_str()), true, TagLib::AudioProperties::ReadStyle::Average);

    if (!f.isNull() && f.tag() && f.audioProperties())
    {
        TagLib::Tag* tag = f.tag();

        tag->setGenre(genre);
        f.save();
    }
}

void Tags::SetComment(std::string comment)
{
    TagLib::FileRef f (static_cast<const char*>(m_Filepath.c_str()), true, TagLib::AudioProperties::ReadStyle::Average);

    if (!f.isNull() && f.tag() && f.audioProperties())
    {
        TagLib::Tag* tag = f.tag();

        tag->setComment(comment);
        f.save();
    }
}
