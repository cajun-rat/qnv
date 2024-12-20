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
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

 public slots:
  /** Called every time the search text box is updates */
  void UpdateSearch();
  void NoteSelectionChanged(QListWidgetItem *, QListWidgetItem *);
  /** Called every time the text of the 'current_note' is changed */
  void NoteBodyChanged();
  void OnSaveNoteTimer();
  void Search();
  void ChangeNotesDirectory();

 private:
  static const int kWriteTimeoutMs = 2000;

  void SetCurrentNote(NoteListWidget *note_list_widget);
  void SaveCurrentNote();
  static std::string NotesDirectory();

  Ui::MainWindow *ui_;
  NoteListWidget *current_note_{nullptr};
  QLabel *current_note_status_;
  std::vector<NoteListWidget *> notes_;
  QTimer *writeback_timer_;
};

#endif  // QNV_MAINWINDOW_H_
