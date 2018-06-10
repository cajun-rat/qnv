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
    : QListWidgetItem(note->title()), note_(note) {}

void NoteListWidget::UpdateVisibility(QString searchTerm) {
  setHidden(!note_->body().contains(searchTerm, Qt::CaseInsensitive));
}

bool NoteListWidget::operator<(const QListWidgetItem &otherw) const {
  const NoteListWidget &other = dynamic_cast<const NoteListWidget &>(otherw);
  return other.note_ < note_;
}

Note::Note(const std::string filepath) : file_path_(filepath) {
  pt::ptree pt;
  if (fs::exists(file_path_)) {
    read_json(file_path_, pt);
    body_ = QString::fromStdString(pt.get<std::string>("content"));
    create_date_ = pt.get<double>("createdate", 0.0);
    last_modified_date_ = pt.get<double>("modifydate", 0.0);
  } else {
    create_date_ = last_modified_date_ =
        ((double)QDateTime::currentMSecsSinceEpoch()) / 1000.0;
  }
}

void Note::Save(QString newBody) {
  pt::ptree pt;
  pt::ptree tags;

  body_ = newBody;

  double now = ((double)QDateTime::currentMSecsSinceEpoch()) / 1000.0;
  last_modified_date_ = now;
  // For compatibility with nvPY
  pt.put("modifydate", last_modified_date_);
  pt.put_child("tags", tags);
  pt.put("createdate", create_date_);
  pt.put("syncdate", 0);
  pt.put("content", newBody.toStdString());
  pt.put("savedate", last_modified_date_);

  write_json(file_path_, pt);
  qDebug() << "Saved to disk:" << QString::fromStdString(file_path_);
}

QString Note::title() const {
  int titleend = body_.indexOf('\n');
  if (titleend == -1) {
    return body_;
  } else {
    return body_.left(titleend);
  }
}

bool Note::operator<(const Note &other) const {
  return last_modified_date_ < other.last_modified_date_;
}

bool operator<(const Note::Ptr &lhs, const Note::Ptr &rhs) {
  return *lhs < *rhs;
}

QDebug &operator<<(QDebug &d, const Note &note) {
  std::stringstream ss;
  d << note.title();

  ss << note.last_modified_date_;
  d << QString::fromStdString(ss.str());
  return d;
}

QDebug &operator<<(QDebug &d, const Note::Ptr &note) { return d << *note; }

std::vector<Note::Ptr> ReadNotes(const std::string &dirpath) {
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
