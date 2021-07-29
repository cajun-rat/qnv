#ifndef QNV_MAINWINDOW_H_
#define QNV_MAINWINDOW_H_

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
  ~MainWindow() override;

 public slots:
  void UpdateSearch();
  void NoteSelectionChanged(QListWidgetItem *, QListWidgetItem *);
  void NoteBodyChanged();
  void SaveNote();
  void Search();
  void ChangeNotesDirectory();

 private:
  static const int kWriteTimeoutMs = 2000;

  void SetCurrentNote(Note::Ptr note);
  void SaveCurrentNote();
  std::string NotesDirectory() const;

  Ui::MainWindow *ui_;
  Note::Ptr current_note_;
  QLabel *current_note_status_;
  std::vector<NoteListWidget *> notes_;
  QTimer *writeback_timer_;
};

#endif  // QNV_MAINWINDOW_H_
