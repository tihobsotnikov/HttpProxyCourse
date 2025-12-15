#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QSplitter>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>
#include <QSqlQuery>

#include "models/Structures.h"

class AdminWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr);

private slots:
    void onSearchTextChanged(const QString& text);
    void onGenerateReportClicked();
    void onChapterSelectionChanged();
    void onSaveChangesClicked();

private:
    void setupUI();
    void setupStudentsTab();
    void setupCourseEditorTab();
    void loadCourseData();
    void updateChapterContent();
    
    // Main widgets
    QTabWidget* m_tabWidget;
    
    // Students tab widgets
    QWidget* m_studentsTab;
    QTableView* m_studentsTableView;
    QLineEdit* m_searchLineEdit;
    QPushButton* m_reportButton;
    QSqlTableModel* m_usersModel;
    QSortFilterProxyModel* m_proxyModel;
    
    // Course editor tab widgets
    QWidget* m_courseEditorTab;
    QListWidget* m_chaptersListWidget;
    QLineEdit* m_chapterTitleEdit;
    QTextEdit* m_chapterContentEdit;
    QPushButton* m_saveChangesButton;
    
    // Course data
    Course m_course;
    int m_currentChapterIndex;
};

#endif // ADMINWINDOW_H