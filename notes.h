#ifndef NOTES_H
#define NOTES_H

#include <string>
#include <vector>
#include <memory>
#include <QAbstractListModel>
#include <QVariant>
#include <QListWidgetItem>
#include <QListWidget>
#include <QTextDocument>
#include <QDebug>
#ifndef Q_MOC_RUN
#endif

using namespace std;

class Note {
public:
    Note(const string filepath);
    QString title() const;
    QString body() const { return m_body;}
    void save(QString newBody);
    bool operator<(const Note &other);
    //friend bool operator <(const shared_ptr<Note>&lhs, const shared_ptr<Note>&rhs);
    friend QDebug& operator <<(QDebug &debug, const Note &note);

private:
    // Non-copyable
    Note(const Note&);
    Note& operator=(const Note&);
    string m_filepath;
    QString m_body;
    double m_createdate;
    double m_modifydate;
};

QDebug& operator <<(QDebug &debug, const shared_ptr<Note> &note);
/* Order by last modified date */
bool operator <(const shared_ptr<Note>&lhs, const shared_ptr<Note>&rhs);

class NoteListWidget : public QListWidgetItem {
public:
    NoteListWidget(shared_ptr<Note> note);
    void updateVisibility(QString searchTerm);
    shared_ptr<Note> note() const { return m_note;}
    virtual bool operator <(const QListWidgetItem &other) const;
private:
    shared_ptr<Note> m_note;
};

vector<shared_ptr<Note> > readNotes(const string &dirpath);

#endif // NOTES_H
