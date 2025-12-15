#include "StudentWindow.h"

StudentWindow::StudentWindow(int userId, QWidget *parent)
    : QMainWindow(parent)
    , m_stackedWidget(nullptr)
    , m_theoryPage(nullptr)
    , m_theoryBrowser(nullptr)
    , m_takeTestButton(nullptr)
    , m_testPage(nullptr)
    , m_questionLabel(nullptr)
    , m_answerGroup(nullptr)
    , m_answerButton(nullptr)
    , m_userId(userId)
    , m_currentChapterIndex(0)
    , m_currentQuestionIndex(0)
    , m_errorsCount(0)
{
    setWindowTitle("Система обучения HTTP Proxy - Студент");
    setMinimumSize(800, 600);
    
    setupUI();
    loadCourse();
    initializeProgress();
}

StudentWindow::~StudentWindow()
{
    // Qt handles cleanup automatically
}

void StudentWindow::setupUI()
{
    // Create main stacked widget
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);
    
    // Create theory page (page 0)
    m_theoryPage = new QWidget();
    QVBoxLayout* theoryLayout = new QVBoxLayout(m_theoryPage);
    
    // Theory content browser
    m_theoryBrowser = new QTextBrowser();
    m_theoryBrowser->setReadOnly(true);
    theoryLayout->addWidget(m_theoryBrowser);
    
    // Take test button
    m_takeTestButton = new QPushButton("Пройти тест");
    m_takeTestButton->setMinimumHeight(40);
    connect(m_takeTestButton, &QPushButton::clicked, this, &StudentWindow::onTakeTestClicked);
    
    QHBoxLayout* theoryButtonLayout = new QHBoxLayout();
    theoryButtonLayout->addStretch();
    theoryButtonLayout->addWidget(m_takeTestButton);
    theoryButtonLayout->addStretch();
    theoryLayout->addLayout(theoryButtonLayout);
    
    m_stackedWidget->addWidget(m_theoryPage); // Index 0
    
    // Create test page (page 1)
    m_testPage = new QWidget();
    QVBoxLayout* testLayout = new QVBoxLayout(m_testPage);
    
    // Question label
    m_questionLabel = new QLabel();
    m_questionLabel->setWordWrap(true);
    m_questionLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; margin: 10px; }");
    testLayout->addWidget(m_questionLabel);
    
    // Answer options group
    m_answerGroup = new QButtonGroup(this);
    
    // Create radio buttons for answers (we'll create them dynamically)
    QWidget* answersWidget = new QWidget();
    answersWidget->setObjectName("answersWidget"); // Set object name for finding later
    new QVBoxLayout(answersWidget); // Layout will be used in loadCurrentQuestion()
    
    // We'll add radio buttons dynamically in loadCurrentQuestion()
    testLayout->addWidget(answersWidget);
    testLayout->addStretch();
    
    // Answer button
    m_answerButton = new QPushButton("Ответить");
    m_answerButton->setMinimumHeight(40);
    connect(m_answerButton, &QPushButton::clicked, this, &StudentWindow::onAnswerClicked);
    
    QHBoxLayout* testButtonLayout = new QHBoxLayout();
    testButtonLayout->addStretch();
    testButtonLayout->addWidget(m_answerButton);
    testButtonLayout->addStretch();
    testLayout->addLayout(testButtonLayout);
    
    m_stackedWidget->addWidget(m_testPage); // Index 1
}

void StudentWindow::loadCourse()
{
    const QString BINARY_PATH = "data/course.bin";
    const QString ENCRYPTION_KEY = "SECRET_KEY_123";
    
    m_course = CourseManager::loadCourseFromBinary(BINARY_PATH, ENCRYPTION_KEY);
    
    if (m_course.chapters.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить данные курса!");
        close();
        return;
    }
    
    qDebug() << "Course loaded successfully with" << m_course.chapters.size() << "chapters";
}

void StudentWindow::initializeProgress()
{
    DatabaseManager& db = DatabaseManager::getInstance();
    QPair<int, QString> lastProgress = db.getLastProgress(m_userId);
    
    int lastChapterId = lastProgress.first;
    QString lastStatus = lastProgress.second;
    
    if (lastChapterId == -1) {
        // No progress found, start from beginning
        m_currentChapterIndex = 0;
    } else {
        if (lastStatus == "completed") {
            // Move to next chapter
            m_currentChapterIndex = lastChapterId + 1;
            if (m_currentChapterIndex >= m_course.chapters.size()) {
                // Course completed
                QMessageBox::information(this, "Поздравляем!", "Вы успешно завершили весь курс!");
                m_currentChapterIndex = m_course.chapters.size() - 1; // Stay on last chapter
            }
        } else {
            // Continue from the same chapter (failed or new)
            m_currentChapterIndex = lastChapterId;
        }
    }
    
    // Ensure chapter index is valid
    if (m_currentChapterIndex < 0 || m_currentChapterIndex >= m_course.chapters.size()) {
        m_currentChapterIndex = 0;
    }
    
    showTheoryPage();
}

void StudentWindow::showTheoryPage()
{
    if (m_currentChapterIndex >= m_course.chapters.size()) {
        QMessageBox::information(this, "Курс завершен", "Вы прошли все главы курса!");
        return;
    }
    
    const Chapter& currentChapter = m_course.chapters[m_currentChapterIndex];
    
    // Set window title with current chapter
    setWindowTitle(QString("Система обучения HTTP Proxy - Глава %1: %2")
                   .arg(m_currentChapterIndex + 1)
                   .arg(currentChapter.title));
    
    // Display theory content
    QString theoryContent = QString("<h2>Глава %1: %2</h2><br>%3")
                           .arg(m_currentChapterIndex + 1)
                           .arg(currentChapter.title)
                           .arg(currentChapter.content);
    
    m_theoryBrowser->setHtml(theoryContent);
    
    // Enable test button only if there are questions
    m_takeTestButton->setEnabled(!currentChapter.questions.isEmpty());
    if (currentChapter.questions.isEmpty()) {
        m_takeTestButton->setText("Нет тестов для этой главы");
    } else {
        m_takeTestButton->setText("Пройти тест");
    }
    
    // Switch to theory page
    m_stackedWidget->setCurrentIndex(0);
}

void StudentWindow::showTestPage()
{
    m_currentQuestionIndex = 0;
    m_errorsCount = 0;
    loadCurrentQuestion();
    m_stackedWidget->setCurrentIndex(1);
}

void StudentWindow::loadCurrentQuestion()
{
    if (m_currentChapterIndex >= m_course.chapters.size()) {
        return;
    }
    
    const Chapter& currentChapter = m_course.chapters[m_currentChapterIndex];
    
    if (m_currentQuestionIndex >= currentChapter.questions.size()) {
        // All questions answered correctly - test passed
        DatabaseManager& db = DatabaseManager::getInstance();
        db.saveProgress(m_userId, m_currentChapterIndex, 100, "completed");
        
        QMessageBox::information(this, "Тест пройден!", 
                                QString("Поздравляем! Вы успешно прошли тест по главе %1.")
                                .arg(m_currentChapterIndex + 1));
        
        moveToNextChapter();
        return;
    }
    
    const Question& currentQuestion = currentChapter.questions[m_currentQuestionIndex];
    
    // Clear previous radio buttons
    for (QRadioButton* button : m_answerButtons) {
        m_answerGroup->removeButton(button);
        delete button;
    }
    m_answerButtons.clear();
    
    // Set question text
    m_questionLabel->setText(QString("Вопрос %1 из %2:\n\n%3")
                            .arg(m_currentQuestionIndex + 1)
                            .arg(currentChapter.questions.size())
                            .arg(currentQuestion.q_text));
    
    // Create radio buttons for answers
    QWidget* answersWidget = m_testPage->findChild<QWidget*>("answersWidget");
    if (answersWidget) {
        QVBoxLayout* answersLayout = qobject_cast<QVBoxLayout*>(answersWidget->layout());
        if (!answersLayout) {
            answersLayout = new QVBoxLayout(answersWidget);
        }
        
        for (int i = 0; i < currentQuestion.options.size(); ++i) {
            QRadioButton* radioButton = new QRadioButton(currentQuestion.options[i]);
            m_answerButtons.append(radioButton);
            m_answerGroup->addButton(radioButton, i);
            answersLayout->addWidget(radioButton);
        }
    }
}

void StudentWindow::onTakeTestClicked()
{
    if (m_currentChapterIndex >= m_course.chapters.size()) {
        return;
    }
    
    const Chapter& currentChapter = m_course.chapters[m_currentChapterIndex];
    
    if (currentChapter.questions.isEmpty()) {
        QMessageBox::information(this, "Нет тестов", "Для этой главы нет тестовых вопросов.");
        return;
    }
    
    showTestPage();
}

void StudentWindow::onAnswerClicked()
{
    if (m_currentChapterIndex >= m_course.chapters.size()) {
        return;
    }
    
    const Chapter& currentChapter = m_course.chapters[m_currentChapterIndex];
    
    if (m_currentQuestionIndex >= currentChapter.questions.size()) {
        return;
    }
    
    // Check if an answer is selected
    int selectedAnswer = m_answerGroup->checkedId();
    if (selectedAnswer == -1) {
        QMessageBox::warning(this, "Выберите ответ", "Пожалуйста, выберите один из вариантов ответа.");
        return;
    }
    
    const Question& currentQuestion = currentChapter.questions[m_currentQuestionIndex];
    bool isCorrect = (selectedAnswer == currentQuestion.correct_index);
    
    processAnswer(isCorrect);
}

void StudentWindow::processAnswer(bool isCorrect)
{
    if (isCorrect) {
        // Correct answer - move to next question
        m_currentQuestionIndex++;
        loadCurrentQuestion();
    } else {
        // Wrong answer
        m_errorsCount++;
        
        QMessageBox::warning(this, "Неверный ответ", 
                           QString("Ответ неверный. Ошибок: %1 из 3 допустимых.")
                           .arg(m_errorsCount));
        
        if (m_errorsCount >= 3) {
            // Too many errors - reset to theory
            DatabaseManager& db = DatabaseManager::getInstance();
            db.saveProgress(m_userId, m_currentChapterIndex, 0, "fail");
            
            QMessageBox::critical(this, "Тест не пройден", 
                                "Вы допустили 3 ошибки. Изучите теорию заново.");
            
            resetToTheory();
        } else {
            // Continue with same question
            // Clear selection
            m_answerGroup->setExclusive(false);
            for (QRadioButton* button : m_answerButtons) {
                button->setChecked(false);
            }
            m_answerGroup->setExclusive(true);
        }
    }
}

void StudentWindow::resetToTheory()
{
    m_errorsCount = 0;
    m_currentQuestionIndex = 0;
    showTheoryPage();
}

void StudentWindow::moveToNextChapter()
{
    m_currentChapterIndex++;
    
    if (m_currentChapterIndex >= m_course.chapters.size()) {
        // Course completed
        QMessageBox::information(this, "Курс завершен!", 
                               "Поздравляем! Вы успешно завершили весь курс обучения HTTP Proxy!");
        m_currentChapterIndex = m_course.chapters.size() - 1; // Stay on last chapter
    }
    
    showTheoryPage();
}