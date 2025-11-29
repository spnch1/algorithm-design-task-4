#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QList>
#include <QString>
#include <QList>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <array>

struct Record {
    int key;
    std::array<char, 64> data;
};

struct IndexEntry {
    int key;
    qint64 offset;

    bool operator<(const IndexEntry& other) const noexcept {
        return key < other.key;
    }
};

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    [[nodiscard]] bool add(int key, const QString& data);
    [[nodiscard]] QString search(int key, int& comparisons);
    [[nodiscard]] bool remove(int key);
    [[nodiscard]] bool edit(int key, const QString& newData);
    void generateRandom(int count);
    void clear();

    [[nodiscard]] QList<IndexEntry> getIndexTable() const;
    [[nodiscard]] QList<Record> getDataTable() const;

    [[nodiscard]] QString getComplexity() const;

private:
    QString dataFilePath;
    QString indexFilePath;
    QList<IndexEntry> index;

    void loadIndex();
    void saveIndex();
    bool writeRecord(qint64 offset, const Record& record);
    bool readRecord(qint64 offset, Record& record) const;
};

#endif // DATABASEMANAGER_H
