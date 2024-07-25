#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QPair>
#include <QVariant>
#include <qdebug.h>

class TreeItem : public QObject
{
    Q_OBJECT
public:
    enum class Type
    {
        Name,
        Value,
        Fuzz,
        FuzzMin,
        FuzzMax,
        avg,
        selectedwave,
        FuzzInterval_s
    };

    explicit TreeItem(const QString& name,
            const QVariant& value,
            bool fuzz,
            const QVariant& fuzzMin,
            const QVariant& fuzzMax,
            const QVariant& avg,
            const QVariant& selectedwave,
            TreeItem* parentItem = 0);
    ~TreeItem();

    void appendChild(TreeItem* child);

    TreeItem* child(int row);
    QList<TreeItem*> children();
    int childCount() const;
    int columnCount() const;

    QVariant data(TreeItem::Type type) const;
    bool setData(TreeItem::Type type, const QVariant& data = 0);

    int row() const;
    TreeItem* parentItem();

    void resetFuzzTimer();
    int timeElapsedSinceFuzz_ms();

private:
    QList<TreeItem*> m_childItems;
    QList<QVariant> m_itemData;
    TreeItem* m_parentItem;
    QElapsedTimer m_lastFuzz;
};
