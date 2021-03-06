/*
 * Copyright (c) 2013-2015 Meltytech, LLC
 * Author: Dan Dennedy <dan@dennedy.org>
 *
 * This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "playlistcommands.h"
#include "mltcontroller.h"
#include "mainwindow.h"

namespace Playlist
{

AppendCommand::AppendCommand(PlaylistModel& model, const QString& xml, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_xml(xml)
{
    setText(QObject::tr("Append playlist item %1").arg(m_model.rowCount() + 1));
}

void AppendCommand::redo()
{
    Mlt::Producer producer(MLT.profile(), "xml-string", m_xml.toUtf8().constData());
    m_model.append(producer);
}

void AppendCommand::undo()
{
    m_model.remove(m_model.rowCount() - 1);
}

InsertCommand::InsertCommand(PlaylistModel& model, const QString& xml, int row, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_xml(xml)
    , m_row(row)
{
    setText(QObject::tr("Insert playist item %1").arg(row + 1));
}

void InsertCommand::redo()
{
    Mlt::Producer producer(MLT.profile(), "xml-string", m_xml.toUtf8().constData());
    m_model.insert(producer, m_row);
}

void InsertCommand::undo()
{
    m_model.remove(m_row);
}

UpdateCommand::UpdateCommand(PlaylistModel& model, const QString& xml, int row, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_newXml(xml)
    , m_row(row)
{
    setText(QObject::tr("Update playlist item %1").arg(row + 1));
    m_oldXml = MLT.XML(m_model.playlist()->get_clip(m_row));
}

void UpdateCommand::redo()
{
    Mlt::Producer producer(MLT.profile(), "xml-string", m_newXml.toUtf8().constData());
    m_model.update(m_row, producer);
}

void UpdateCommand::undo()
{
    Mlt::Producer producer(MLT.profile(), "xml-string", m_oldXml.toUtf8().constData());
    m_model.update(m_row, producer);
}

RemoveCommand::RemoveCommand(PlaylistModel& model, int row, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_row(row)
{
    m_xml = MLT.XML(m_model.playlist()->get_clip(m_row));
    setText(QObject::tr("Remove playlist item %1").arg(row + 1));
}

void RemoveCommand::redo()
{
    m_model.remove(m_row);
}

void RemoveCommand::undo()
{
    Mlt::Producer producer(MLT.profile(), "xml-string", m_xml.toUtf8().constData());
    m_model.insert(producer, m_row);
}

ClearCommand::ClearCommand(PlaylistModel& model, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
{
    m_xml = MLT.XML(m_model.playlist());
    setText(QObject::tr("Clear playlist"));
}

void ClearCommand::redo()
{
    m_model.clear();
}

void ClearCommand::undo()
{
    m_model.close();
    Mlt::Producer* producer = new Mlt::Producer(MLT.profile(), "xml-string", m_xml.toUtf8().constData());
    if (producer->is_valid()) {
        producer->set("resource", "<playlist>");
        MAIN.open(producer);
        MLT.pause();
        MAIN.seekPlaylist(0);
    }
}

MoveCommand::MoveCommand(PlaylistModel &model, int from, int to, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_from(from)
    , m_to(to)
{
    setText(QObject::tr("Move item from %1 to %2").arg(from + 1).arg(to + 1));
}

void MoveCommand::redo()
{
    m_model.move(m_from, m_to);
}

void MoveCommand::undo()
{
    m_model.move(m_to, m_from);
}

} // namespace Playlist
