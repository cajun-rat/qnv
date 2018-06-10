#include "notes.h"
#include <QDateTime>
#include <QObject>
#include <QtDebug>
#include <sstream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace fs = boost::filesystem;

namespace pt = boost::property_tree;

NoteListWidget::NoteListWidget(Note::Ptr note)
    : QListWidgetItem(note->title()), m_note(note) {}

void NoteListWidget::updateVisibility(QString searchTerm) {
  setHidden(!m_note->body().contains(searchTerm, Qt::CaseInsensitive));
}

bool NoteListWidget::operator<(const QListWidgetItem &otherw) const {
  const NoteListWidget &other = dynamic_cast<const NoteListWidget &>(otherw);
  return other.m_note < m_note;
}

Note::Note(const std::string filepath) : m_filepath(filepath) {
  pt::ptree pt;
  if (fs::exists(filepath)) {
    read_json(filepath, pt);
    m_body = QString::fromStdString(pt.get<std::string>("content"));
    m_createdate = pt.get<double>("createdate", 0.0);
    m_modifydate = pt.get<double>("modifydate", 0.0);
  } else {
    m_createdate = m_modifydate =
        ((double)QDateTime::currentMSecsSinceEpoch()) / 1000.0;
  }
}

void Note::save(QString newBody) {
  pt::ptree pt;
  pt::ptree tags;

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
  qDebug() << "Saved to disk:" << QString::fromStdString(m_filepath);
}

QString Note::title() const {
  int titleend = m_body.indexOf('\n');
  if (titleend == -1) {
    return m_body;
  } else {
    return m_body.left(titleend);
  }
}

bool Note::operator<(const Note &other) const {
  return m_modifydate < other.m_modifydate;
}

bool operator<(const Note::Ptr &lhs, const Note::Ptr &rhs) {
  return *lhs < *rhs;
}

QDebug &operator<<(QDebug &d, const Note &note) {
  std::stringstream ss;
  d << note.title();

  ss << note.m_modifydate;
  d << QString::fromStdString(ss.str());
  return d;
}

QDebug &operator<<(QDebug &d, const Note::Ptr &note) { return d << *note; }

std::vector<Note::Ptr> readNotes(const std::string &dirpath) {
  fs::path p(dirpath);

  if (!fs::exists(p) || !fs::is_directory(p)) {
    // TODO throw
  }

  std::vector<Note::Ptr> res;

  for (fs::directory_iterator f(p); f != fs::directory_iterator(); ++f) {
    // TODO handle junk in the directory
    std::string filename(f->path().string());
    if (fs::extension(f->path()) != ".json") {
      qDebug() << "Skipping wrong extension"
               << QString::fromStdString(fs::extension(f->path()));
      continue;
    }
    try {
      res.push_back(std::make_shared<Note>(filename));
    } catch (boost::property_tree::json_parser::json_parser_error &e) {
      qDebug() << "failed to open" << QString::fromStdString(f->path().string())
               << QString::fromStdString(e.message());
    }
  }

  return res;
}
