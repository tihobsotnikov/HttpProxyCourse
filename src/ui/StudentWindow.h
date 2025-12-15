#ifndef STUDENTWINDOW_H
#define STUDENTWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

#include "../models/Structures.h"
#include "../core/CourseManager.h"
#include "../db/DatabaseManager.h"

class StudentWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StudentWindow(int userId, QWidget *parent = nullptr);
    ~StudentWindow();

private slots:
    void onTakeTestClicked();
    void onAnswerClicked();

private:
    void setupUI();
    void loadCourse();
    void initializeProgress();
    void showTheoryPage();
    void showTestPage();
    void loadCurrentQuestion();
    void processAnswer(bool isCorrect);
    void resetToTheory();
    void moveToNextChapter();
    
    // UI Components
    QStackedWidget* m_stackedWidget;
    
    // Theory page (page 0)
    QWidget* m_theoryPage;
    QTextBrowser* m_theoryBrowser;
    QPushButton* m_takeTestButton;
    
    // Test page (page 1)
    QWidget* m_testPage;
    QLabel* m_questionLabel;
    QButtonGroup* m_answerGroup;
    QList<QRadioButton*> m_answerButtons;
    QPushButton* m_answerButton;
    
    // State variables
    int m_userId;
    int m_currentChapterIndex;
    int m_currentQuestionIndex;
    int m_errorsCount;
    Course m_course;
};

#endif // STUDENTWINDOW_H