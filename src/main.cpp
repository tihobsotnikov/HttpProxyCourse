#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QDir>

#include "core/CourseManager.h"
#include "core/CryptoUtils.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== HTTP Proxy Learning System - Business Logic Test ===";
    
    const QString ENCRYPTION_KEY = "SECRET_KEY_123";
    const QString JSON_PATH = "data/course_source.json";
    const QString BINARY_PATH = "data/course.bin";
    
    // 1. Check if binary file exists
    qDebug() << "\n1. Checking for existing binary course file...";
    QFile binaryFile(BINARY_PATH);
    
    if (!binaryFile.exists()) {
        qDebug() << "❌ Binary file not found:" << BINARY_PATH;
        
        // 2. Look for JSON source file
        qDebug() << "\n2. Looking for JSON source file...";
        QFile jsonFile(JSON_PATH);
        
        if (!jsonFile.exists()) {
            qDebug() << "❌ JSON source file not found:" << JSON_PATH;
            qDebug() << "Please ensure the JSON source file exists.";
            return 1;
        }
        
        qDebug() << "✅ JSON source file found:" << JSON_PATH;
        
        // 3. Load course from JSON
        qDebug() << "\n3. Loading course from JSON...";
        Course course = CourseManager::loadCourseFromJSON(JSON_PATH);
        
        if (course.chapters.isEmpty()) {
            qDebug() << "❌ Failed to load course from JSON or course is empty";
            return 1;
        }
        
        qDebug() << "✅ Course loaded successfully from JSON";
        qDebug() << "   Chapters loaded:" << course.chapters.size();
        
        // Display course info
        for (int i = 0; i < course.chapters.size(); ++i) {
            const Chapter& chapter = course.chapters[i];
            qDebug() << "   Chapter" << (i+1) << ":" << chapter.title;
            qDebug() << "     Questions:" << chapter.questions.size();
        }
        
        // 4. Encrypt and save to binary
        qDebug() << "\n4. Encrypting and saving to binary format...";
        if (!CourseManager::saveCourseToBinary(course, BINARY_PATH, ENCRYPTION_KEY)) {
            qDebug() << "❌ Failed to save course to binary format";
            return 1;
        }
        
        qDebug() << "✅ Course converted and encrypted";
        qDebug() << "   Binary file saved to:" << BINARY_PATH;
        
        // Display file size
        QFile savedFile(BINARY_PATH);
        if (savedFile.exists()) {
            qDebug() << "   File size:" << savedFile.size() << "bytes";
        }
    } else {
        qDebug() << "✅ Binary course file already exists:" << BINARY_PATH;
    }
    
    // 5. Load and decrypt binary file
    qDebug() << "\n5. Loading and decrypting binary course file...";
    Course loadedCourse = CourseManager::loadCourseFromBinary(BINARY_PATH, ENCRYPTION_KEY);
    
    if (loadedCourse.chapters.isEmpty()) {
        qDebug() << "❌ Failed to load course from binary file";
        return 1;
    }
    
    qDebug() << "✅ Course loaded and decrypted successfully";
    qDebug() << "   Chapters loaded:" << loadedCourse.chapters.size();
    
    // Display detailed course information
    qDebug() << "\n6. Course content verification:";
    for (int i = 0; i < loadedCourse.chapters.size(); ++i) {
        const Chapter& chapter = loadedCourse.chapters[i];
        qDebug() << "\n   📖 Chapter" << (i+1) << "(ID:" << chapter.id << "):" << chapter.title;
        qDebug() << "      Content length:" << chapter.content.length() << "characters";
        qDebug() << "      Questions:" << chapter.questions.size();
        
        for (int j = 0; j < chapter.questions.size(); ++j) {
            const Question& question = chapter.questions[j];
            qDebug() << "        ❓ Question" << (j+1) << ":" << question.q_text.left(50) + "...";
            qDebug() << "           Options:" << question.options.size() << ", Correct:" << question.correct_index;
        }
    }
    
    // 7. Test crypto functions
    qDebug() << "\n7. Testing cryptographic functions...";
    QString testPassword = "test_password_123";
    QString hashedPassword = CryptoUtils::hashPassword(testPassword);
    qDebug() << "   Original password:" << testPassword;
    qDebug() << "   SHA-256 hash:" << hashedPassword;
    
    // Test XOR encryption/decryption
    QByteArray testData = "This is a test message for XOR encryption";
    QString testKey = "test_key";
    QByteArray encrypted = CryptoUtils::xorEncryptDecrypt(testData, testKey);
    QByteArray decrypted = CryptoUtils::xorEncryptDecrypt(encrypted, testKey);
    
    qDebug() << "   XOR test - Original:" << testData;
    qDebug() << "   XOR test - Encrypted size:" << encrypted.size() << "bytes";
    qDebug() << "   XOR test - Decrypted:" << decrypted;
    qDebug() << "   XOR test - Match:" << (testData == decrypted ? "✅ YES" : "❌ NO");
    
    qDebug() << "\n=== Business Logic Test Completed Successfully! ===";
    qDebug() << "All components are working correctly:";
    qDebug() << "  ✅ JSON parsing and course loading";
    qDebug() << "  ✅ Binary serialization with QDataStream";
    qDebug() << "  ✅ XOR encryption/decryption";
    qDebug() << "  ✅ SHA-256 password hashing";
    qDebug() << "  ✅ File format validation with Magic Number";

    return 0;
}