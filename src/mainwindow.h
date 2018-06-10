#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QTimer>
#include "notes.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

 private:
  static const int c_writetimeoutms = 2000;

  Ui::MainWindow *m_ui;
  Note::Ptr m_currentNote;
  QLabel *m_currentNoteSaved;
  std::vector<NoteListWidget *> m_notes;
  QTimer *m_dirtywritetimeout;
  void setCurrentNote(Note::Ptr note);
  void saveCurrentNote();
  std::string notesDirectory() const;

 public slots:
  void updateSearch();
  void noteSelectionChanged(QListWidgetItem *, QListWidgetItem *);
  void noteBodyChanged();
  void saveNote();
  void searchEnterPressed();
  void changeNotesDirectory();
};

#endif  // MAINWINDOW_H
