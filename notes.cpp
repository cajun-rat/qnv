#include "notes.h"
#include <QObject>
#include <string>
#include <sstream>
#include <QtDebug>
#include <QDateTime>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>

// TODO
// Display modified date in notes list

using namespace boost::filesystem;

using boost::property_tree::ptree;

NoteListWidget::NoteListWidget(shared_ptr<Note> note)
    : QListWidgetItem(note->title()),
      m_note(note) { }

void NoteListWidget::updateVisibility(QString searchTerm)
{
    setHidden(!m_note->body().contains(searchTerm, Qt::CaseInsensitive));
}

bool NoteListWidget::operator <(const QListWidgetItem &otherw) const
{
    const NoteListWidget& other = dynamic_cast<const NoteListWidget&>(otherw);
    //qDebug() << "Order called " << m_note->title() << " vs " << other.m_note->title();
    return other.m_note < m_note;
}

Note::Note(const string filepath)
    : m_filepath(filepath)
{
    ptree pt;
    if (exists(filepath)) {
        read_json(filepath, pt);
        m_body = QString::fromStdString(pt.get<string>("content"));
        m_createdate = pt.get<double>("createdate",0.0);
        m_modifydate = pt.get<double>("modifydate", 0.0);
    } else {
        m_createdate = m_modifydate = ((double)QDateTime::currentMSecsSinceEpoch()) / 1000.0;
    }
}

void Note::save(QString newBody)
{
    ptree pt;
    ptree tags;

    m_body = newBody;

    double now = ((double)QDateTime::currentMSecsSinceEpoch()) / 1000.0;
    m_modifydate = now;
    // For compatibility with nvPY
    pt.put("modifydate", m_modifydate);
    pt.put_child("tags", tags);
    pt.put("createdate", m_createdate);
    pt.put("syncdate", 0);
    pt.put("content", newBody.toStdString());
    pt.put("savedate", m_modifydate);

    write_json(m_filepath, pt);
    qDebug() << "Saved to disk:" <<  QString::fromStdString(m_filepath);
}

QString Note::title() const
{
    int titleend = m_body.indexOf('\n');
    if (titleend == -1) {
        return m_body;
    } else {
        return m_body.left(titleend);
    }
}

bool Note::operator <(const Note& other) {
    return m_modifydate < other.m_modifydate;
}

bool operator <(const shared_ptr<Note>& lhs, const shared_ptr<Note> &rhs) {
    return *lhs < *rhs;
}

QDebug& operator <<(QDebug &d, const Note &note) {
    stringstream ss;
    d << note.title();

    ss << note.m_modifydate;
    d << QString::fromStdString(ss.str());
    return d;
}

QDebug& operator <<(QDebug &d, const shared_ptr<Note> &note) {
    return d  << *note;
}


vector<shared_ptr<Note> > readNotes(const string &dirpath)
{
    path p(dirpath);

    if (! exists(p) || !is_directory(p)) {
        // TODO throw
    }

    vector<shared_ptr<Note> > res;

    for(directory_iterator f(p); f != directory_iterator(); ++f) {
        // TODO handle junk in the directory
        string filename(f->path().string());
        if (extension(f->path()) != ".json") {
            qDebug() << "Skipping wrong extension" << QString::fromStdString(extension(f->path()));
            continue;
        }
        try {
            res.push_back(make_shared<Note>(filename));
        } catch(boost::property_tree::json_parser::json_parser_error &e) {
            qDebug() << "failed to open" << QString::fromStdString(f->path().string()) << QString::fromStdString(e.message());
        }
    }

    return res;
}
