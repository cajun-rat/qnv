#include "mainwindow.h"

#include <qdir.h>
#include <qfiledialog.h>

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QStandardPaths>
#include <QStringListModel>
#include <boost/random.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <filesystem>
#include <memory>
#include <vector>

#include "notes.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui_(new Ui::MainWindow),
      writeback_timer_(new QTimer) {
  ui_->setupUi(this);
  writeback_timer_->setSingleShot(true);
  writeback_timer_->setInterval(kWriteTimeoutMs);

  current_note_status_ = new QLabel("current note", ui_->statusBar);
  ui_->statusBar->addPermanentWidget(current_note_status_);
  for (const Note::Ptr &i : ReadNotes(NotesDirectory())) {
    auto *w = new NoteListWidget(i);
    ui_->notes->addItem(w);
    notes_.push_back(w);  // ownership handled by Qt
  }
  connect(ui_->notes,
          SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
          this,
          SLOT(NoteSelectionChanged(QListWidgetItem *, QListWidgetItem *)));
  connect(ui_->search, SIGNAL(textChanged(QString)), this,
          SLOT(UpdateSearch(void)));
  connect(ui_->currentNote, SIGNAL(textChanged(void)), this,
          SLOT(NoteBodyChanged(void)));
  connect(writeback_timer_, SIGNAL(timeout()), this,
          SLOT(OnSaveNoteTimer(void)));
  // ctrl-f goes to find
  auto *ctrlf = new QShortcut(QKeySequence("Ctrl+F"), this);
  connect(ctrlf, SIGNAL(activated()), ui_->search, SLOT(setFocus()));

  // enter in search box
  connect(ui_->search, SIGNAL(returnPressed()), this, SLOT(Search()));

  connect(ui_->actionChange_Notes_Directory, SIGNAL(triggered()), this,
          SLOT(ChangeNotesDirectory()));
}

MainWindow::~MainWindow() {
  delete ui_;
  delete writeback_timer_;
}

std::string MainWindow::NotesDirectory() {
  QString defaultpath =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0] +
      "/.qnv";
  QSettings settings;

  QString qpath = settings.value("notesdir", defaultpath).toString();
  std::filesystem::path path = qpath.toStdString();

  if (!std::filesystem::exists(path)) {
    qDebug() << "Creating notes directory";
    std::filesystem::create_directories(path);
  }
  return path;
}

void MainWindow::UpdateSearch() {
  QString t = ui_->search->text();
  bool foundOne = false;
  for (auto widget : notes_) {
    widget->UpdateVisibility(t);
    if (!foundOne && !widget->isHidden()) {
      widget->setSelected(true);
      foundOne = true;
    }
  }
  if (!foundOne) {
    ui_->notes->clearSelection();
  }
}

void MainWindow::Search() {
  // select the note (if there is one)
  QList<QListWidgetItem *> selectedItems = ui_->notes->selectedItems();
  if (selectedItems.length() > 0) {
    qDebug() << "Setting note";
    auto *noteListWidget = dynamic_cast<NoteListWidget *>(selectedItems[0]);
    SetCurrentNote(noteListWidget);
  } else {
    qDebug() << "Creating a new note";
    std::string chars = "0123456789abcdef";
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0,
                                                         (int)chars.size() - 1);
    std::string filename(NotesDirectory());
    filename.append("/");
    for (int i = 0; i < 30; i++) {
      filename.push_back(chars[index_dist(rng)]);
    }
    filename.append(".json");

    Note::Ptr newNote = std::make_shared<Note>(filename);
    newNote->Save(ui_->search->text());
    auto *noteListWidget = new NoteListWidget(newNote);
    SetCurrentNote(noteListWidget);
    ui_->notes->addItem(noteListWidget);
    notes_.push_back(noteListWidget);
  }
  ui_->currentNote->setFocus();
  QTextCursor cursor = ui_->currentNote->textCursor();
  cursor.movePosition(QTextCursor::End);
  ui_->currentNote->setTextCursor(cursor);

  ui_->search->setText("");
}

void MainWindow::ChangeNotesDirectory() {
  QFileDialog dialog(this, "Choose a directory to store notes in",
                     QString::fromStdString((NotesDirectory())));
  dialog.setFileMode(QFileDialog::Directory);
  dialog.setOption(QFileDialog::ShowDirsOnly);
  dialog.setFilter(QDir::AllDirs | QDir::Hidden);

  if (!dialog.exec()) {
    return;
  }
  auto selection = dialog.selectedFiles();
  if (selection.length() == 0) {
    return;
  }
  QSettings settings;
  settings.setValue("notesdir", selection[0]);
  QMessageBox::information(
      this, "Restart Required",
      "Notes directory changed. A restart of qnv is required.");
}

void MainWindow::SetCurrentNote(NoteListWidget *note_list_widget) {
  // Write out the old note
  if (writeback_timer_->isActive()) {
    writeback_timer_->stop();
    assert(current_note_.get() != 0);
    qDebug() << "Saving dirty buffer on note switch";
    SaveCurrentNote();
  }
  QPlainTextEdit *editor_ui = ui_->currentNote;
  if (current_note_) {
    int old_position = editor_ui->textCursor().position();
    current_note_->note()->CursorPosition(old_position);
  }

  auto note = note_list_widget->note();
  editor_ui->setPlainText(note->body());
  // Setting the text causes the dirty flag to be be set
  writeback_timer_->stop();
  current_note_status_->setText("");
  current_note_ = note_list_widget;
  QTextCursor cursor = editor_ui->textCursor();
  cursor.setPosition(note->CursorPosition(), QTextCursor::MoveMode::MoveAnchor);
  editor_ui->setTextCursor(cursor);
  editor_ui->ensureCursorVisible();
}

void MainWindow::NoteSelectionChanged(QListWidgetItem *current,
                                      QListWidgetItem *) {
  auto *noteListWidget = dynamic_cast<NoteListWidget *>(current);
  SetCurrentNote(noteListWidget);
}

void MainWindow::NoteBodyChanged() {
  current_note_status_->setText("Current note dirty");
  writeback_timer_->start();
}

void MainWindow::OnSaveNoteTimer() {
  // write to disk
  qDebug() << "Writing note to disk";
  assert(current_note_.get() != 0);
  SaveCurrentNote();
  current_note_status_->setText("Current note saved");
}

void MainWindow::SaveCurrentNote() {
  current_note_->note()->Save(ui_->currentNote->document()->toPlainText());
  current_note_->UpdateTitle();
  ui_->notes->sortItems();
}
