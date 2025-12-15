#ifndef COURSEMANAGER_H
#define COURSEMANAGER_H

#include <QString>
#include "models/Structures.h"

class CourseManager
{
public:
    static Course loadCourseFromJSON(const QString& jsonPath);
    static bool saveCourseToBinary(const Course& course, const QString& binPath, const QString& key);
    static Course loadCourseFromBinary(const QString& binPath, const QString& key);

private:
    static const quint32 MAGIC_NUMBER = 0x434F5253; // "CORS" in hex
    CourseManager() = delete;
};

#endif // COURSEMANAGER_H