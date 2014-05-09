#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <Qtimer>
#include "notes.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *m_ui;
    shared_ptr<Note> m_currentNote;
    QLabel *m_currentNoteSaved;
    vector<NoteListWidget*> m_notes;
    QTimer *m_dirtywritetimeout;
    static const int c_writetimeoutms = 2000;
    void setCurrentNote(shared_ptr<Note> note);
    void saveCurrentNote();
    string notesDirectory() const;

public slots:
    void updateSearch();
    void noteSelectionChanged(QListWidgetItem*,QListWidgetItem*);
    void noteBodyChanged();
    void saveNote();
    void searchEnterPressed();
    void changeNotesDirectory();
};

#endif // MAINWINDOW_H
