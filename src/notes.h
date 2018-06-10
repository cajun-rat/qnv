#ifndef NOTES_H
#define NOTES_H

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
  explicit Note(const std::string filepath);
  Note(const Note &) = delete;
  Note &operator=(const Note &) = delete;

  QString title() const;
  QString body() const { return m_body; }
  void save(QString newBody);
  bool operator<(const Note &other) const;
  friend QDebug &operator<<(QDebug &debug, const Note &note);

 private:
  std::string m_filepath;
  QString m_body;
  double m_createdate;
  double m_modifydate;
};

QDebug &operator<<(QDebug &debug, const Note::Ptr &note);
/* Order by last modified date */
bool operator<(const Note::Ptr &lhs, const Note::Ptr &rhs);

class NoteListWidget : public QListWidgetItem {
 public:
  explicit NoteListWidget(Note::Ptr note);
  void updateVisibility(QString searchTerm);
  Note::Ptr note() const { return m_note; }
  virtual bool operator<(const QListWidgetItem &other) const;

 private:
  Note::Ptr m_note;
};

std::vector<Note::Ptr> readNotes(const std::string &dirpath);

#endif  // NOTES_H
