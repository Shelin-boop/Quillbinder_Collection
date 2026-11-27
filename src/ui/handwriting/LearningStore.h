#pragma once
// 0.99.0 (handwriting, item 112): learning the writer's hand. Only corrected lines are kept: the picture of the ink and the
// corrected text, in files on this device (the program's data folder, "handwriting-lines"). They are never sent anywhere;
// the settings have an off switch and a delete. No learning itself happens in this part: a later part reads these files.
#include <QImage>
#include <QString>

namespace qb {
namespace handwriting {

class LearningStore
{
public:
    static QString folder();
    // Keeps one corrected line (a picture file and a text file with the same number). False when learning is off or the
    // files could not be written.
    static bool add(const QImage &picture, const QString &correctedText);
    static int count();
    static int removeAll();   // returns the number of lines deleted
};

} // namespace handwriting
} // namespace qb
