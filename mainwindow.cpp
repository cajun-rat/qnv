#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStringListModel>

#include <vector>
#include <memory>
#include "notes.h"
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

using namespace std;
using namespace boost::filesystem;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_dirtywritetimeout(new QTimer)
{
    m_ui->setupUi(this);
    m_dirtywritetimeout->setSingleShot(true);
    m_dirtywritetimeout->setInterval(c_writetimeoutms);

    m_currentNoteSaved = new QLabel("current note", m_ui->statusBar);
    m_ui->statusBar->addPermanentWidget(m_currentNoteSaved);
    for (shared_ptr<Note> i : readNotes(notesDirectory())) {
        NoteListWidget *w = new NoteListWidget(i);
        m_ui->notes->addItem(w);
        m_notes.push_back(w); // ownership handled by Qt
    }
    connect(m_ui->notes, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(noteSelectionChanged(QListWidgetItem*,QListWidgetItem*)));
    connect(m_ui->search, SIGNAL(textChanged(QString)), this, SLOT(updateSearch(void)));
    connect(m_ui->currentNote, SIGNAL(textChanged(void)), this, SLOT(noteBodyChanged(void)));
    connect(m_dirtywritetimeout, SIGNAL(timeout()), this, SLOT(saveNote(void)));
    // ctrl-f goes to find
    QShortcut* ctrlf = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(ctrlf, SIGNAL(activated()), m_ui->search, SLOT(setFocus()));

    // enter in search box
    connect(m_ui->search, SIGNAL(returnPressed()), this, SLOT(searchEnterPressed()));

    connect(m_ui->actionChange_Notes_Directory, SIGNAL(triggered()), this, SLOT(changeNotesDirectory()));


}

MainWindow::~MainWindow()
{
    delete m_ui;
    delete m_dirtywritetimeout;
}

string MainWindow::notesDirectory() const
{
    QString defaultpath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0] + "/.qnv" ;
    QSettings settings;

    QString qpath = settings.value("notesdir", defaultpath).toString();
    string path = qpath.toStdString();

    if (!exists(path)) {
        qDebug() << "Creating";
        create_directories(path);
    }
    return path;
}

void MainWindow::updateSearch()
{
    QString t = m_ui->search->text();
    bool foundOne = false;
    for (vector<NoteListWidget*>::iterator i = m_notes.begin() ; i != m_notes.end() ; ++i) {
        NoteListWidget* widget = *i;
        widget->updateVisibility(t);
        if (!foundOne && !widget->isHidden()) {
            widget->setSelected(true);
            foundOne = true;
        }
    }
    if (!foundOne) {
        m_ui->notes->clearSelection();
    }
}

void MainWindow::searchEnterPressed()
{
    // select the note (if there is one)
    QList<QListWidgetItem*> selectedItems = m_ui->notes->selectedItems();
    if (selectedItems.length() > 0) {
        qDebug() << "Setting note";
        NoteListWidget* noteListWidget = dynamic_cast<NoteListWidget*>(selectedItems[0]);
        setCurrentNote(noteListWidget->note());
    } else {
        qDebug() << "Creating a new note";
        string chars = "0123456789abcdef";
        boost::random::random_device rng;
        boost::random::uniform_int_distribution<> index_dist(0, (int) chars.size() - 1);
        string filename(notesDirectory());
        filename.append("/");
        for (int i=0; i<30;i++) {
            filename.push_back(chars[index_dist(rng)]);
        }
        filename.append(".json");

        shared_ptr<Note> newNote = make_shared<Note>(filename);
        newNote->save(m_ui->search->text());
        setCurrentNote(newNote);
        NoteListWidget *w = new NoteListWidget(newNote);
        m_ui->notes->addItem(w);
        m_notes.push_back(w);
    }
    m_ui->currentNote->setFocus();
    QTextCursor cursor = m_ui->currentNote->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_ui->currentNote->setTextCursor(cursor);

    m_ui->search->setText("");
}

void MainWindow::changeNotesDirectory() {
    QString dir = QFileDialog::getExistingDirectory(
                this,
                "Choose a directory to store notes in",
                QString::fromStdString(notesDirectory()));
    if (!dir.isNull()) {
        QSettings settings;
        settings.setValue("notesdir",dir);
        QMessageBox::information(this,"Restart Required",
                                 "Notes directory changed. A restart of qnv is required.");
    }
}

void MainWindow::setCurrentNote(shared_ptr<Note> note)
{
    // Write out the old note
    if (m_dirtywritetimeout->isActive()) {
        m_dirtywritetimeout->stop();
        assert(m_currentNote.get() != 0);
        qDebug() << "Saving dirty buffer on note switch";
        saveCurrentNote();
    }

    m_ui->currentNote->setPlainText(note->body());
    m_dirtywritetimeout->stop(); // Setting the text causes the dirty flag to be be set
    m_currentNoteSaved->setText("");
    m_currentNote = note;
}

void MainWindow::noteSelectionChanged(QListWidgetItem* current, QListWidgetItem*)
{
    NoteListWidget* noteListWidget = dynamic_cast<NoteListWidget*>(current);
    setCurrentNote(noteListWidget->note());
}


void MainWindow::noteBodyChanged()
{
    qDebug() << "Note text body changed";
    m_currentNoteSaved->setText("Current note dirty");
    m_dirtywritetimeout->start();
}

void MainWindow::saveNote()
{
    // write to disk
    qDebug() << "Writing note to disk";
    assert(m_currentNote.get() != 0);
    saveCurrentNote();

    m_currentNoteSaved->setText("Current note saved");
}

void MainWindow::saveCurrentNote()
{
    m_currentNote->save(m_ui->currentNote->document()->toPlainText());
    m_ui->notes->sortItems();
}
