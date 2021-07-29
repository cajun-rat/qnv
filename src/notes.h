#ifndef QNV_NOTES_H_
#define QNV_NOTES_H_

#include <QAbstractListModel>
#include <QDebug>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTextDocument>
#include <QVariant>
#include <memory>
#include <string>
#include <vector>

class Note {
 public:
  typedef std::shared_ptr<Note> Ptr;
  explicit Note(std::string filepath);
  Note(const Note &) = delete;
  Note &operator=(const Note &) = delete;

  QString title() const;
  QString body() const { return body_; }
  bool Matches(const QString &query_string,
               QString *summary_out = nullptr) const;
  void Save(const QString &newBody);
  bool operator<(const Note &other) const;
  friend QDebug &operator<<(QDebug &debug, const Note &note);

 private:
  const int kSummaryStartOffset = 5;
  const int kSummaryLength = 30;
  std::string file_path_;
  QString body_;
  double create_date_;
  double last_modified_date_;
};

QDebug &operator<<(QDebug &debug, const Note::Ptr &note);

/* Order by last modified date */
bool operator<(const Note::Ptr &lhs, const Note::Ptr &rhs);

class NoteListWidget : public QListWidgetItem {
 public:
  explicit NoteListWidget(Note::Ptr note);
  void UpdateVisibility(const QString &query_string);
  Note::Ptr note() const { return note_; }
  bool operator<(const QListWidgetItem &other) const override;

 private:
  Note::Ptr note_;
};

std::vector<Note::Ptr> ReadNotes(const std::string &dirpath);

#endif  // QNV_NOTES_H_
