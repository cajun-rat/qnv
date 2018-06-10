#include "mainwindow.h"
#include <QStringListModel>
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QStandardPaths>
#include <boost/filesystem.hpp>
#include <boost/random.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <memory>
#include <vector>
#include "notes.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui_(new Ui::MainWindow),
      writeback_timer_(new QTimer) {
  ui_->setupUi(this);
  writeback_timer_->setSingleShot(true);
  writeback_timer_->setInterval(kWriteTimeoutMs);

  current_note_status_ = new QLabel("current note", ui_->statusBar);
  ui_->statusBar->addPermanentWidget(current_note_status_);
  for (Note::Ptr i : ReadNotes(NotesDirectory())) {
    NoteListWidget* w = new NoteListWidget(i);
    ui_->notes->addItem(w);
    notes_.push_back(w);  // ownership handled by Qt
  }
  connect(ui_->notes,
          SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this,
          SLOT(NoteSelectionChanged(QListWidgetItem*, QListWidgetItem*)));
  connect(ui_->search, SIGNAL(textChanged(QString)), this,
          SLOT(UpdateSearch(void)));
  connect(ui_->currentNote, SIGNAL(textChanged(void)), this,
          SLOT(NoteBodyChanged(void)));
  connect(writeback_timer_, SIGNAL(timeout()), this, SLOT(SaveNote(void)));
  // ctrl-f goes to find
  QShortcut* ctrlf = new QShortcut(QKeySequence("Ctrl+F"), this);
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

std::string MainWindow::NotesDirectory() const {
  QString defaultpath =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0] +
      "/.qnv";
  QSettings settings;

  QString qpath = settings.value("notesdir", defaultpath).toString();
  std::string path = qpath.toStdString();

  if (!boost::filesystem::exists(path)) {
    qDebug() << "Creating notes directory";
    boost::filesystem::create_directories(path);
  }
  return path;
}

void MainWindow::UpdateSearch() {
  QString t = ui_->search->text();
  bool foundOne = false;
  for (std::vector<NoteListWidget*>::iterator i = notes_.begin();
       i != notes_.end(); ++i) {
    NoteListWidget* widget = *i;
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
  QList<QListWidgetItem*> selectedItems = ui_->notes->selectedItems();
  if (selectedItems.length() > 0) {
    qDebug() << "Setting note";
    NoteListWidget* noteListWidget =
        dynamic_cast<NoteListWidget*>(selectedItems[0]);
    SetCurrentNote(noteListWidget->note());
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
    SetCurrentNote(newNote);
    NoteListWidget* w = new NoteListWidget(newNote);
    ui_->notes->addItem(w);
    notes_.push_back(w);
  }
  ui_->currentNote->setFocus();
  QTextCursor cursor = ui_->currentNote->textCursor();
  cursor.movePosition(QTextCursor::End);
  ui_->currentNote->setTextCursor(cursor);

  ui_->search->setText("");
}

void MainWindow::ChangeNotesDirectory() {
  QString dir = QFileDialog::getExistingDirectory(
      this, "Choose a directory to store notes in",
      QString::fromStdString(NotesDirectory()));
  if (!dir.isNull()) {
    QSettings settings;
    settings.setValue("notesdir", dir);
    QMessageBox::information(
        this, "Restart Required",
        "Notes directory changed. A restart of qnv is required.");
  }
}

void MainWindow::SetCurrentNote(Note::Ptr note) {
  // Write out the old note
  if (writeback_timer_->isActive()) {
    writeback_timer_->stop();
    assert(current_note_.get() != 0);
    qDebug() << "Saving dirty buffer on note switch";
    SaveCurrentNote();
  }

  ui_->currentNote->setPlainText(note->body());
  writeback_timer_
      ->stop();  // Setting the text causes the dirty flag to be be set
  current_note_status_->setText("");
  current_note_ = note;
}

void MainWindow::NoteSelectionChanged(QListWidgetItem* current,
                                      QListWidgetItem*) {
  NoteListWidget* noteListWidget = dynamic_cast<NoteListWidget*>(current);
  SetCurrentNote(noteListWidget->note());
}

void MainWindow::NoteBodyChanged() {
  current_note_status_->setText("Current note dirty");
  writeback_timer_->start();
}

void MainWindow::SaveNote() {
  // write to disk
  qDebug() << "Writing note to disk";
  assert(current_note_.get() != 0);
  SaveCurrentNote();

  current_note_status_->setText("Current note saved");
}

void MainWindow::SaveCurrentNote() {
  current_note_->Save(ui_->currentNote->document()->toPlainText());
  ui_->notes->sortItems();
}
