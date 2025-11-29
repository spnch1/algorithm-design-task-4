#include "databasemanager.h"
#include <QDir>
#include <QRandomGenerator>
#include <QSet>
#include <cstring>
#include <algorithm>

DatabaseManager::DatabaseManager() {
    QString path = QDir::currentPath();
    dataFilePath = path + "/database.dat";
    indexFilePath = path + "/index.idx";
    loadIndex();
}

DatabaseManager::~DatabaseManager() {
    saveIndex();
}

void DatabaseManager::loadIndex() {
    index.clear();
    QFile file(indexFilePath);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        while (!in.atEnd()) {
            IndexEntry entry;
            in >> entry.key >> entry.offset;
            index.append(entry);
        }
        file.close();
    }
    std::sort(index.begin(), index.end()); // just in case
}

void DatabaseManager::saveIndex() {
    QFile file(indexFilePath);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        for (const auto& entry : index) {
            out << entry.key << entry.offset;
        }
        file.close();
    }
}

bool DatabaseManager::add(int key, const QString& data) {
    int dummy;
    if (!search(key, dummy).isEmpty()) {
        return false;
    }

    Record record;
    record.key = key;
    record.data.fill(0);
    QByteArray bytes = data.toUtf8();
    int len = qMin((int)record.data.size() - 1, bytes.size());
    std::memcpy(record.data.data(), bytes.constData(), len);
    record.data[len] = '\0';

    QFile file(dataFilePath);
    if (!file.open(QIODevice::Append)) {
        return false;
    }
    
    qint64 offset = file.pos();
    file.write(reinterpret_cast<const char*>(&record), sizeof(Record));
    file.close();

    IndexEntry entry;
    entry.key = key;
    entry.offset = offset;
    index.append(entry);
    std::sort(index.begin(), index.end());
    
    saveIndex();
    return true;
}

QString DatabaseManager::search(int key, int& comparisons) {
    comparisons = 0;
    int left = 0;
    int right = index.size() - 1;

    while (left <= right) {
        comparisons++;
        int mid = left + (right - left) / 2;
        if (index[mid].key == key) {
            Record record;
            if (readRecord(index[mid].offset, record)) {
                return QString::fromUtf8(record.data.data());
            }
            return QString();
        }
        if (index[mid].key < key) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return QString();
}

bool DatabaseManager::remove(int key) {
    int left = 0;
    int right = index.size() - 1;
    int foundIdx = -1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (index[mid].key == key) {
            foundIdx = mid;
            break;
        }
        if (index[mid].key < key) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    if (foundIdx != -1) {
        index.removeAt(foundIdx);
        saveIndex();
        return true;
    }
    return false;
}

bool DatabaseManager::edit(int key, const QString& newData) {
    int left = 0;
    int right = index.size() - 1;
    int foundIdx = -1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (index[mid].key == key) {
            foundIdx = mid;
            break;
        }
        if (index[mid].key < key) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    if (foundIdx != -1) {
        Record record;
        record.key = key;
        record.data.fill(0);
        QByteArray bytes = newData.toUtf8();
        int len = qMin((int)record.data.size() - 1, bytes.size());
        std::memcpy(record.data.data(), bytes.constData(), len);
        record.data[len] = '\0';

        return writeRecord(index[foundIdx].offset, record);
    }
    return false;
}

void DatabaseManager::generateRandom(int count) {
    clear();
    
    QFile dataFile(dataFilePath);
    if (!dataFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        return;
    }

    index.reserve(count);
    QSet<int> usedKeys;
    usedKeys.reserve(count);

    for (int i = 0; i < count; ++i) {
        int key;
        do {
            key = QRandomGenerator::global()->bounded(2147483647); // Ensure positive int
        } while (usedKeys.contains(key));
        usedKeys.insert(key);

        Record record;
        record.key = key;
        record.data.fill(0);
        QString dataStr = QString("Data_%1").arg(key);
        QByteArray bytes = dataStr.toUtf8();
        int len = qMin((int)record.data.size() - 1, bytes.size());
        std::memcpy(record.data.data(), bytes.constData(), len);
        record.data[len] = '\0';

        qint64 offset = dataFile.pos();
        dataFile.write(reinterpret_cast<const char*>(&record), sizeof(Record));

        IndexEntry entry;
        entry.key = key;
        entry.offset = offset;
        index.append(entry);
    }
    dataFile.close();
    std::sort(index.begin(), index.end());
    saveIndex();
}

void DatabaseManager::clear() {
    index.clear();
    QFile(dataFilePath).remove();
    QFile(indexFilePath).remove();
    QFile d(dataFilePath); 
    if(d.open(QIODevice::WriteOnly)) d.close();
    QFile i(indexFilePath); 
    if(i.open(QIODevice::WriteOnly)) i.close();
}

QList<IndexEntry> DatabaseManager::getIndexTable() const {
    return index;
}

QList<Record> DatabaseManager::getDataTable() const {
    QList<Record> records;
    QFile file(dataFilePath);
    if (file.open(QIODevice::ReadOnly)) {
        while (!file.atEnd()) {
            Record r;
            if (file.read(reinterpret_cast<char*>(&r), sizeof(Record)) == sizeof(Record)) {
                records.append(r);
            }
        }
        file.close();
    }
    return records;
}

bool DatabaseManager::writeRecord(qint64 offset, const Record& record) {
    QFile file(dataFilePath);
    if (file.open(QIODevice::ReadWrite)) {
        if (file.seek(offset)) {
            file.write(reinterpret_cast<const char*>(&record), sizeof(Record));
            file.close();
            return true;
        }
        file.close();
    }
    return false;
}

bool DatabaseManager::readRecord(qint64 offset, Record& record) const {
    QFile file(dataFilePath);
    if (file.open(QIODevice::ReadOnly)) {
        if (file.seek(offset)) {
            if (file.read(reinterpret_cast<char*>(&record), sizeof(Record)) == sizeof(Record)) {
                file.close();
                return true;
            }
        }
        file.close();
    }
    return false;
}

QString DatabaseManager::getComplexity() const {
    return "Time Complexity of Search:\n"
           "  Best Case: O(1)\n"
           "  Average Case: O(log N)\n"
           "  Worst Case: O(log N)\n"
           "  Where N is the number of records.\n"
           "  (Binary Search on sorted index)";
}
