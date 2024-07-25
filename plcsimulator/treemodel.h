#include "treeitem.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <QModelIndex>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QTimer>
#include <QVariant>

#include <memory>

class PLCMessage;
class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    void SineWaveBytePoints();
    void SineWaveIntPoints();
    void SawtoothBytePoints();
    void SawtoothIntPoints();
    void TriangleWaveBytePoints();
    void TriangleWaveIntPoints();

    enum class AccessRole
    {
        Name = Qt::UserRole,
        Type = Qt::UserRole + 1,
        Value = Qt::UserRole + 2,
        Fuzz = Qt::UserRole + 3,
        FuzzMin = Qt::UserRole + 4,
        FuzzMax = Qt::UserRole + 5,
        avg = Qt::UserRole + 6,
        selectedwave = Qt::UserRole + 7,
        FuzzInterval_s = Qt::UserRole + 8
    };

    enum class ModelType
    {
        Sending,
        Receiving
    };

    explicit TreeModel(std::shared_ptr<PLCMessage> msg,
            ModelType type,
            bool filterSpare,
            bool fuzz,
            uint fuzzInterval = 0,
            QObject* parent = nullptr);
    ~TreeModel() override;

    QVariant type(const QModelIndex& index) const;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QVariant headerData(int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QString modelType() const;
    Q_INVOKABLE QString modelId() const;
    Q_INVOKABLE bool writeData(const QModelIndex& index, const QVariant& data, const QString& role);
    Q_INVOKABLE QVariant readData(const QModelIndex& index, const QString& role);

public slots:
    void refreshView();
    void fuzzify();

private:
    void populateModel(std::shared_ptr<PLCMessage> msg, TreeItem* parent);
    qint64 maxValue(const QVariant& value);
    qint64 minValue(const QVariant& value);
    qint64 avgValue(const QVariant& value);
    qint64 waveValue(const QVariant& value);
    std::optional<QVariant> processInput(const QVariant& input, QVariant::Type, int min, int max,int avg,int wave);

    bool m_fuzz = false;
    bool m_filterSpare = false;
    int noItr_Byte = 0;
    int noItr_Int = 0;
     QStringList strlst_Byte_points,strlst_Int_points,strlstSawth_Byte_points,strlstSawth_Int_points,strlstDecay_Byte_points,strlstDecay_Int_points;

    ModelType m_type;
    QTimer m_fuzzClock;
    std::shared_ptr<PLCMessage> m_message;
    TreeItem* m_rootItem = nullptr;
    QHash<int, QByteArray> m_roleNameMapping;
    QRegularExpression m_spareRE = QRegularExpression("SPARE_.*");
};
