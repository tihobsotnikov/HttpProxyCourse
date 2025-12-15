#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QDataStream>

struct Question {
    QString q_text;
    QStringList options;
    int correct_index;

    Question() : correct_index(0) {}
    
    Question(const QString& text, const QStringList& opts, int correctIdx)
        : q_text(text), options(opts), correct_index(correctIdx) {}

    friend QDataStream& operator<<(QDataStream& stream, const Question& question) {
        stream << question.q_text << question.options << question.correct_index;
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, Question& question) {
        stream >> question.q_text >> question.options >> question.correct_index;
        return stream;
    }
};

struct Chapter {
    int id;
    QString title;
    QString content;
    QList<Question> questions;

    Chapter() : id(0) {}
    
    Chapter(int chapterId, const QString& chapterTitle, const QString& chapterContent)
        : id(chapterId), title(chapterTitle), content(chapterContent) {}

    friend QDataStream& operator<<(QDataStream& stream, const Chapter& chapter) {
        stream << chapter.id << chapter.title << chapter.content << chapter.questions;
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, Chapter& chapter) {
        stream >> chapter.id >> chapter.title >> chapter.content >> chapter.questions;
        return stream;
    }
};

struct Course {
    QList<Chapter> chapters;

    Course() {}

    friend QDataStream& operator<<(QDataStream& stream, const Course& course) {
        stream << course.chapters;
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, Course& course) {
        stream >> course.chapters;
        return stream;
    }
};

#endif // STRUCTURES_H