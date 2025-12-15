#include "CourseManager.h"
#include "CryptoUtils.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDataStream>
#include <QDebug>

Course CourseManager::loadCourseFromJSON(const QString& jsonPath)
{
    Course course;
    
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open JSON file:" << jsonPath;
        return course;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return course;
    }
    
    if (!doc.isArray()) {
        qWarning() << "JSON document is not an array";
        return course;
    }
    
    QJsonArray chaptersArray = doc.array();
    
    for (const QJsonValue& chapterValue : chaptersArray) {
        if (!chapterValue.isObject()) {
            continue;
        }
        
        QJsonObject chapterObj = chapterValue.toObject();
        
        Chapter chapter;
        chapter.id = chapterObj["id"].toInt();
        chapter.title = chapterObj["title"].toString();
        chapter.content = chapterObj["content"].toString();
        
        // Parse questions
        QJsonArray questionsArray = chapterObj["questions"].toArray();
        for (const QJsonValue& questionValue : questionsArray) {
            if (!questionValue.isObject()) {
                continue;
            }
            
            QJsonObject questionObj = questionValue.toObject();
            
            Question question;
            question.q_text = questionObj["q_text"].toString();
            question.correct_index = questionObj["correct_index"].toInt();
            
            // Parse options
            QJsonArray optionsArray = questionObj["options"].toArray();
            for (const QJsonValue& optionValue : optionsArray) {
                question.options.append(optionValue.toString());
            }
            
            chapter.questions.append(question);
        }
        
        course.chapters.append(chapter);
    }
    
    return course;
}

bool CourseManager::saveCourseToBinary(const Course& course, const QString& binPath, const QString& key)
{
    QFile file(binPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open binary file for writing:" << binPath;
        return false;
    }
    
    // Serialize course to QByteArray
    QByteArray courseData;
    QDataStream courseStream(&courseData, QIODevice::WriteOnly);
    courseStream << course;
    
    // Encrypt the serialized data
    QByteArray encryptedData = CryptoUtils::xorEncryptDecrypt(courseData, key);
    
    // Write magic number and encrypted data to file
    QDataStream fileStream(&file);
    fileStream << MAGIC_NUMBER;
    fileStream << encryptedData;
    
    file.close();
    return true;
}

Course CourseManager::loadCourseFromBinary(const QString& binPath, const QString& key)
{
    Course course;
    
    QFile file(binPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open binary file for reading:" << binPath;
        return course;
    }
    
    QDataStream fileStream(&file);
    
    // Read and verify magic number
    quint32 magicNumber;
    fileStream >> magicNumber;
    
    if (magicNumber != MAGIC_NUMBER) {
        qWarning() << "Invalid file format - magic number mismatch";
        file.close();
        return course;
    }
    
    // Read encrypted data
    QByteArray encryptedData;
    fileStream >> encryptedData;
    file.close();
    
    // Decrypt the data
    QByteArray decryptedData = CryptoUtils::xorEncryptDecrypt(encryptedData, key);
    
    // Deserialize course from decrypted data
    QDataStream courseStream(&decryptedData, QIODevice::ReadOnly);
    courseStream >> course;
    
    return course;
}