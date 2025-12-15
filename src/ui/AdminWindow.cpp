#include "ui/AdminWindow.h"
#include "db/DatabaseManager.h"
#include "core/CourseManager.h"
#include <QDateTime>

AdminWindow::AdminWindow(QWidget* parent)
    : QMainWindow(parent), m_currentChapterIndex(-1) {
    setWindowTitle("Панель администратора - HTTP Proxy Course");
    setMinimumSize(900, 600);
    resize(1200, 800);

    setupUI();
    loadCourseData();
}

void AdminWindow::setupUI() {
    // Центральный виджет с вкладками
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    setupStudentsTab();
    setupCourseEditorTab();
}

void AdminWindow::setupStudentsTab()
{
    m_studentsTab = new QWidget();
    m_tabWidget->addTab(m_studentsTab, "Студенты");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(m_studentsTab);
    
    // Title
    QLabel* titleLabel = new QLabel("Управление пользователями", m_studentsTab);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // Search and report controls
    QHBoxLayout* controlsLayout = new QHBoxLayout();
    
    QLabel* searchLabel = new QLabel("Поиск:", m_studentsTab);
    controlsLayout->addWidget(searchLabel);
    
    m_searchLineEdit = new QLineEdit(m_studentsTab);
    m_searchLineEdit->setPlaceholderText("Введите логин для поиска...");
    controlsLayout->addWidget(m_searchLineEdit);
    
    controlsLayout->addStretch();
    
    m_reportButton = new QPushButton("Создать отчет", m_studentsTab);
    m_reportButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 8px 16px; border: none; border-radius: 4px; } QPushButton:hover { background-color: #F57C00; }");
    controlsLayout->addWidget(m_reportButton);
    
    mainLayout->addLayout(controlsLayout);
    
    // Users table
    m_studentsTableView = new QTableView(m_studentsTab);
    m_studentsTableView->setAlternatingRowColors(true);
    m_studentsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_studentsTableView->setSortingEnabled(true);
    
    // Setup model and proxy for filtering
    DatabaseManager& db = DatabaseManager::getInstance();
    m_usersModel = db.getUsersModel();
    
    if (m_usersModel) {
        m_proxyModel = new QSortFilterProxyModel(this);
        m_proxyModel->setSourceModel(m_usersModel);
        m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_proxyModel->setFilterKeyColumn(1); // Filter by login column
        
        m_studentsTableView->setModel(m_proxyModel);
        
        // Hide password hash column for security
        m_studentsTableView->hideColumn(2);
        
        // Adjust column widths
        m_studentsTableView->horizontalHeader()->setStretchLastSection(true);
        m_studentsTableView->resizeColumnsToContents();
    }
    
    mainLayout->addWidget(m_studentsTableView);
    
    // Connect signals
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &AdminWindow::onSearchTextChanged);
    connect(m_reportButton, &QPushButton::clicked, this, &AdminWindow::onGenerateReportClicked);
}

void AdminWindow::setupCourseEditorTab()
{
    m_courseEditorTab = new QWidget();
    m_tabWidget->addTab(m_courseEditorTab, "Редактор курса");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(m_courseEditorTab);
    
    // Title
    QLabel* titleLabel = new QLabel("Редактирование содержимого курса", m_courseEditorTab);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // Main content area with splitter
    QSplitter* splitter = new QSplitter(Qt::Horizontal, m_courseEditorTab);
    
    // Left side - chapters list
    QWidget* leftWidget = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    
    QLabel* chaptersLabel = new QLabel("Главы курса:", leftWidget);
    chaptersLabel->setStyleSheet("font-weight: bold;");
    leftLayout->addWidget(chaptersLabel);
    
    m_chaptersListWidget = new QListWidget(leftWidget);
    m_chaptersListWidget->setMaximumWidth(300);
    leftLayout->addWidget(m_chaptersListWidget);
    
    splitter->addWidget(leftWidget);
    
    // Right side - chapter editor
    QWidget* rightWidget = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    
    // Chapter title editor
    QLabel* titleEditLabel = new QLabel("Заголовок главы:", rightWidget);
    titleEditLabel->setStyleSheet("font-weight: bold;");
    rightLayout->addWidget(titleEditLabel);
    
    m_chapterTitleEdit = new QLineEdit(rightWidget);
    m_chapterTitleEdit->setPlaceholderText("Введите заголовок главы...");
    rightLayout->addWidget(m_chapterTitleEdit);
    
    // Chapter content editor
    QLabel* contentEditLabel = new QLabel("Содержимое главы:", rightWidget);
    contentEditLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    rightLayout->addWidget(contentEditLabel);
    
    m_chapterContentEdit = new QTextEdit(rightWidget);
    m_chapterContentEdit->setPlaceholderText("Введите содержимое главы...");
    rightLayout->addWidget(m_chapterContentEdit);
    
    // Save button
    m_saveChangesButton = new QPushButton("Сохранить изменения", rightWidget);
    m_saveChangesButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 4px; font-weight: bold; } QPushButton:hover { background-color: #45a049; }");
    m_saveChangesButton->setEnabled(false);
    rightLayout->addWidget(m_saveChangesButton);
    
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(splitter);
    
    // Connect signals
    connect(m_chaptersListWidget, &QListWidget::currentRowChanged, this, &AdminWindow::onChapterSelectionChanged);
    connect(m_saveChangesButton, &QPushButton::clicked, this, &AdminWindow::onSaveChangesClicked);
}

void AdminWindow::loadCourseData()
{
    const QString BINARY_PATH = "data/course.bin";
    const QString ENCRYPTION_KEY = "SECRET_KEY_123";
    
    m_course = CourseManager::loadCourseFromBinary(BINARY_PATH, ENCRYPTION_KEY);
    
    if (m_course.chapters.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", 
                           "Не удалось загрузить данные курса.\nПроверьте наличие файла course.bin в папке data/");
        return;
    }
    
    // Populate chapters list
    m_chaptersListWidget->clear();
    for (int i = 0; i < m_course.chapters.size(); ++i) {
        const Chapter& chapter = m_course.chapters[i];
        QString itemText = QString("Глава %1: %2").arg(i + 1).arg(chapter.title);
        m_chaptersListWidget->addItem(itemText);
    }
    
    // Select first chapter if available
    if (!m_course.chapters.isEmpty()) {
        m_chaptersListWidget->setCurrentRow(0);
    }
}

void AdminWindow::onSearchTextChanged(const QString& text)
{
    if (m_proxyModel) {
        m_proxyModel->setFilterFixedString(text);
    }
}

void AdminWindow::onGenerateReportClicked()
{
    DatabaseManager& db = DatabaseManager::getInstance();
    QSqlQuery query = db.executeSelectQuery("SELECT login, role, created_at FROM users ORDER BY created_at DESC");
    
    if (!query.isActive()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось выполнить запрос к базе данных.");
        return;
    }
    
    QString reportContent;
    reportContent += "=== ОТЧЕТ ПО ПОЛЬЗОВАТЕЛЯМ СИСТЕМЫ ===\n";
    reportContent += QString("Дата создания: %1\n\n").arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"));
    
    int totalUsers = 0;
    int adminCount = 0;
    int studentCount = 0;
    
    while (query.next()) {
        QString login = query.value(0).toString();
        QString role = query.value(1).toString();
        QString createdAt = query.value(2).toString();
        
        reportContent += QString("Логин: %1 | Роль: %2 | Дата регистрации: %3\n")
                        .arg(login).arg(role).arg(createdAt);
        
        totalUsers++;
        if (role == "admin") {
            adminCount++;
        } else if (role == "student") {
            studentCount++;
        }
    }
    
    reportContent += "\n=== СТАТИСТИКА ===\n";
    reportContent += QString("Всего пользователей: %1\n").arg(totalUsers);
    reportContent += QString("Администраторов: %1\n").arg(adminCount);
    reportContent += QString("Студентов: %1\n").arg(studentCount);
    reportContent += "\n=== КОНЕЦ ОТЧЕТА ===\n";
    
    // Save to file
    QString fileName = QString("Report_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QFile reportFile(fileName);
    
    if (reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&reportFile);
        out << reportContent;
        reportFile.close();
        
        QMessageBox::information(this, "Отчет создан", 
                               QString("Отчет успешно сохранен в файл:\n%1\n\nВсего пользователей: %2")
                               .arg(fileName).arg(totalUsers));
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить отчет в файл.");
    }
}

void AdminWindow::onChapterSelectionChanged()
{
    int currentRow = m_chaptersListWidget->currentRow();
    if (currentRow >= 0 && currentRow < m_course.chapters.size()) {
        m_currentChapterIndex = currentRow;
        updateChapterContent();
        m_saveChangesButton->setEnabled(true);
    } else {
        m_currentChapterIndex = -1;
        m_chapterTitleEdit->clear();
        m_chapterContentEdit->clear();
        m_saveChangesButton->setEnabled(false);
    }
}

void AdminWindow::updateChapterContent()
{
    if (m_currentChapterIndex >= 0 && m_currentChapterIndex < m_course.chapters.size()) {
        const Chapter& chapter = m_course.chapters[m_currentChapterIndex];
        m_chapterTitleEdit->setText(chapter.title);
        m_chapterContentEdit->setPlainText(chapter.content);
    }
}

void AdminWindow::onSaveChangesClicked()
{
    if (m_currentChapterIndex < 0 || m_currentChapterIndex >= m_course.chapters.size()) {
        QMessageBox::warning(this, "Ошибка", "Не выбрана глава для сохранения.");
        return;
    }
    
    QString newTitle = m_chapterTitleEdit->text().trimmed();
    QString newContent = m_chapterContentEdit->toPlainText();
    
    if (newTitle.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заголовок главы не может быть пустым.");
        return;
    }
    
    // Update chapter in memory
    Chapter& chapter = m_course.chapters[m_currentChapterIndex];
    chapter.title = newTitle;
    chapter.content = newContent;
    
    // Update list widget item
    QString itemText = QString("Глава %1: %2").arg(m_currentChapterIndex + 1).arg(newTitle);
    m_chaptersListWidget->item(m_currentChapterIndex)->setText(itemText);
    
    // Save to binary file
    const QString BINARY_PATH = "data/course.bin";
    const QString ENCRYPTION_KEY = "SECRET_KEY_123";
    
    if (CourseManager::saveCourseToBinary(m_course, BINARY_PATH, ENCRYPTION_KEY)) {
        QMessageBox::information(this, "Успех", 
                               QString("Изменения в главе \"%1\" успешно сохранены!").arg(newTitle));
    } else {
        QMessageBox::critical(this, "Ошибка", 
                            "Не удалось сохранить изменения в файл.\nПроверьте права доступа к папке data/");
    }
}