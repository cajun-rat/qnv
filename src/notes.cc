#include "notes.h"

#include <QDateTime>
#include <QObject>
#include <QtDebug>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <iomanip>
#include <string>
#include <utility>

namespace fs = std::filesystem;

namespace pt = boost::property_tree;

NoteListWidget::NoteListWidget(Note::Ptr note)
    : QListWidgetItem(note->title()), note_(note) {}

void NoteListWidget::UpdateVisibility(const QString &query_string) {
  QString summary;
  setHidden(!note_->Matches(query_string, &summary));
  setToolTip(summary);
}

void NoteListWidget::UpdateTitle() { setText(note_->title()); }

bool NoteListWidget::operator<(const QListWidgetItem &otherw) const {
  const auto &other = dynamic_cast<const NoteListWidget &>(otherw);
  return other.note_ < note_;
}

Note::Note(std::filesystem::path filepath) : file_path_(std::move(filepath)) {
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

bool Note::Matches(const QString &query_string, QString *summary) const {
  if (query_string.isEmpty()) {
    if (summary != nullptr) {
      // Special Case matching the empty string, and remove the tooltip
      *summary = "";
    }
    return true;
  }
  int idx = body_.indexOf(query_string, 0, Qt::CaseInsensitive);
  if (idx == -1) {
    return false;
  }
  if (summary != nullptr) {
    int start = idx - kSummaryStartOffset;
    if (start < 0) {
      start = 0;
    }
    *summary = body_.mid(start, kSummaryLength);
  }
  return true;
}

void Note::Save(const QString &newBody) {
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
  int title_end = body_.indexOf('\n');
  if (title_end == -1) {
    return body_;
  } else {
    return body_.left(title_end);
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

std::vector<Note::Ptr> ReadNotes(const std::filesystem::path &dirpath) {
  if (!fs::exists(dirpath) || !fs::is_directory(dirpath)) {
    // TODO throw
  }

  std::vector<Note::Ptr> res;

  for (fs::directory_iterator f(dirpath); f != fs::directory_iterator(); ++f) {
    std::string filename(f->path().string());
    if (f->path().extension() != ".json") {
      continue;
    }
    try {
      res.push_back(std::make_shared<Note>(*f));
    } catch (boost::property_tree::json_parser::json_parser_error &e) {
      qDebug() << "failed to open" << QString::fromStdString(f->path().string())
               << QString::fromStdString(e.message());
    }
  }

  return res;
}
