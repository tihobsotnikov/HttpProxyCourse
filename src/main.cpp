#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>

#include "core/CourseManager.h"
#include "core/CryptoUtils.h"
#include "db/DatabaseManager.h"
#include "ui/LoginDialog.h"
#include "ui/AdminWindow.h"
#include "ui/StudentWindow.h"

/**
 * @brief Запускает главное приложение системы обучения HTTP Proxy
 * @param argc количество аргументов командной строки
 * @param argv массив аргументов командной строки
 * @return код завершения приложения
 */
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    qDebug() << "=== HTTP Proxy Learning System - GUI Application ===";

    const QString ENCRYPTION_KEY = "SECRET_KEY_123";
    const QString JSON_PATH = "data/course_source.json";
    const QString BINARY_PATH = "data/course.bin";

    // Инициализация подключения к базе данных
    qDebug() << "\n1. Initializing database connection...";
    DatabaseManager& db = DatabaseManager::getInstance();

    if (!db.connectToDatabase()) {
        QMessageBox::critical(nullptr, "Ошибка базы данных",
                            QString("Не удалось подключиться к базе данных:\n%1\n\nПроверьте настройки PostgreSQL.").arg(db.getLastError()));
        return 1;
    }

    if (!db.initDatabase()) {
        QMessageBox::critical(nullptr, "Ошибка инициализации",
                            QString("Не удалось инициализировать базу данных:\n%1").arg(db.getLastError()));
        return 1;
    }

    qDebug() << "✅ Database initialized successfully";

    // Проверка существования бинарного файла курса
    qDebug() << "\n2. Checking course data...";
    QFile binaryFile(BINARY_PATH);

    if (!binaryFile.exists()) {
        qDebug() << "❌ Binary file not found:" << BINARY_PATH;

        QFile jsonFile(JSON_PATH);
        if (!jsonFile.exists()) {
            QMessageBox::critical(nullptr, "Ошибка данных курса",
                                QString("Не найден файл данных курса:\n%1\n\nПожалуйста, убедитесь, что файл существует.").arg(JSON_PATH));
            return 1;
        }

        // Конвертация JSON в бинарный формат с шифрованием
        qDebug() << "✅ JSON source file found, converting to binary...";
        Course course = CourseManager::loadCourseFromJSON(JSON_PATH);

        if (course.chapters.isEmpty()) {
            QMessageBox::critical(nullptr, "Ошибка загрузки курса", "Не удалось загрузить данные курса из JSON файла.");
            return 1;
        }

        if (!CourseManager::saveCourseToBinary(course, BINARY_PATH, ENCRYPTION_KEY)) {
            QMessageBox::critical(nullptr, "Ошибка сохранения", "Не удалось сохранить курс в бинарный формат.");
            return 1;
        }

        qDebug() << "✅ Course converted and saved to binary format";
    } else {
        qDebug() << "✅ Binary course file exists";
    }

    // Отображение диалога аутентификации
    qDebug() << "\n3. Starting authentication...";
    LoginDialog loginDialog;

    if (loginDialog.exec() != QDialog::Accepted) {
        qDebug() << "User cancelled login";
        return 0;
    }

    QString userRole = loginDialog.getRole();
    int userId = loginDialog.getUserId();
    qDebug() << "✅ User authenticated with role:" << userRole << "and ID:" << userId;

    // Запуск соответствующего интерфейса в зависимости от роли пользователя
    if (userRole == "admin") {
        qDebug() << "Launching admin interface...";
        AdminWindow adminWindow;
        adminWindow.show();
        return app.exec();
    } else if (userRole == "student") {
        qDebug() << "Launching student interface...";
        StudentWindow studentWindow(userId);
        studentWindow.show();
        return app.exec();
    } else {
        QMessageBox::warning(nullptr, "Неизвестная роль",
                           QString("Неизвестная роль пользователя: %1").arg(userRole));
        return 1;
    }
}