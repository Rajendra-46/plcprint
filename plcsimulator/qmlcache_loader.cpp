#include <QtQml/qqmlprivate.h>
#include <QtCore/qdir.h>
#include <QtCore/qurl.h>

static const unsigned char qt_resource_tree[] = {
0,
0,0,0,0,2,0,0,0,6,0,0,0,1,0,0,0,
8,0,0,0,0,0,1,0,0,0,0,0,0,0,144,0,
0,0,0,0,1,0,0,0,0,0,0,0,178,0,0,0,
0,0,1,0,0,0,0,0,0,0,200,0,0,0,0,0,
1,0,0,0,0,0,0,0,64,0,0,0,0,0,1,0,
0,0,0,0,0,0,108,0,0,0,0,0,1,0,0,0,
0};
static const unsigned char qt_resource_names[] = {
0,
1,0,0,0,47,0,47,0,25,0,217,70,220,0,80,0,
76,0,67,0,84,0,114,0,101,0,101,0,83,0,101,0,
116,0,116,0,105,0,110,0,103,0,115,0,66,0,117,0,
116,0,116,0,111,0,110,0,46,0,113,0,109,0,108,0,
19,14,116,199,156,0,80,0,76,0,67,0,84,0,114,0,
101,0,101,0,67,0,104,0,101,0,99,0,107,0,66,0,
111,0,120,0,46,0,113,0,109,0,108,0,15,14,186,121,
156,0,80,0,76,0,67,0,84,0,114,0,101,0,101,0,
86,0,105,0,101,0,119,0,46,0,113,0,109,0,108,0,
14,4,175,21,156,0,82,0,79,0,83,0,69,0,108,0,
101,0,109,0,101,0,110,0,116,0,46,0,113,0,109,0,
108,0,8,8,1,90,92,0,109,0,97,0,105,0,110,0,
46,0,113,0,109,0,108,0,25,11,235,164,92,0,80,0,
76,0,67,0,70,0,117,0,122,0,122,0,83,0,101,0,
116,0,116,0,105,0,110,0,103,0,115,0,80,0,114,0,
111,0,109,0,112,0,116,0,46,0,113,0,109,0,108};
static const unsigned char qt_resource_empty_payout[] = { 0, 0, 0, 0, 0 };
QT_BEGIN_NAMESPACE
extern Q_CORE_EXPORT bool qRegisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);
QT_END_NAMESPACE
namespace QmlCacheGeneratedCode {
namespace _0x5f__PLCFuzzSettingsPrompt_qml { 
    extern const unsigned char qmlData[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), nullptr, nullptr
    };
}
namespace _0x5f__main_qml { 
    extern const unsigned char qmlData[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), nullptr, nullptr
    };
}
namespace _0x5f__ROSElement_qml { 
    extern const unsigned char qmlData[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), nullptr, nullptr
    };
}
namespace _0x5f__PLCTreeView_qml { 
    extern const unsigned char qmlData[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), nullptr, nullptr
    };
}
namespace _0x5f__PLCTreeCheckBox_qml { 
    extern const unsigned char qmlData[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), nullptr, nullptr
    };
}
namespace _0x5f__PLCTreeSettingsButton_qml { 
    extern const unsigned char qmlData[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), nullptr, nullptr
    };
}

}
namespace {
struct Registry {
    Registry();
    ~Registry();
    QHash<QString, const QQmlPrivate::CachedQmlUnit*> resourcePathToCachedUnit;
    static const QQmlPrivate::CachedQmlUnit *lookupCachedUnit(const QUrl &url);
};

Q_GLOBAL_STATIC(Registry, unitRegistry)


Registry::Registry() {
        resourcePathToCachedUnit.insert(QStringLiteral("/PLCFuzzSettingsPrompt.qml"), &QmlCacheGeneratedCode::_0x5f__PLCFuzzSettingsPrompt_qml::unit);
        resourcePathToCachedUnit.insert(QStringLiteral("/main.qml"), &QmlCacheGeneratedCode::_0x5f__main_qml::unit);
        resourcePathToCachedUnit.insert(QStringLiteral("/ROSElement.qml"), &QmlCacheGeneratedCode::_0x5f__ROSElement_qml::unit);
        resourcePathToCachedUnit.insert(QStringLiteral("/PLCTreeView.qml"), &QmlCacheGeneratedCode::_0x5f__PLCTreeView_qml::unit);
        resourcePathToCachedUnit.insert(QStringLiteral("/PLCTreeCheckBox.qml"), &QmlCacheGeneratedCode::_0x5f__PLCTreeCheckBox_qml::unit);
        resourcePathToCachedUnit.insert(QStringLiteral("/PLCTreeSettingsButton.qml"), &QmlCacheGeneratedCode::_0x5f__PLCTreeSettingsButton_qml::unit);
    QQmlPrivate::RegisterQmlUnitCacheHook registration;
    registration.version = 0;
    registration.lookupCachedQmlUnit = &lookupCachedUnit;
    QQmlPrivate::qmlregister(QQmlPrivate::QmlUnitCacheHookRegistration, &registration);
QT_PREPEND_NAMESPACE(qRegisterResourceData)(/*version*/0x01, qt_resource_tree, qt_resource_names, qt_resource_empty_payout);
}

Registry::~Registry() {
    QQmlPrivate::qmlunregister(QQmlPrivate::QmlUnitCacheHookRegistration, quintptr(&lookupCachedUnit));
}

const QQmlPrivate::CachedQmlUnit *Registry::lookupCachedUnit(const QUrl &url) {
    if (url.scheme() != QLatin1String("qrc"))
        return nullptr;
    QString resourcePath = QDir::cleanPath(url.path());
    if (resourcePath.isEmpty())
        return nullptr;
    if (!resourcePath.startsWith(QLatin1Char('/')))
        resourcePath.prepend(QLatin1Char('/'));
    return unitRegistry()->resourcePathToCachedUnit.value(resourcePath, nullptr);
}
}
int QT_MANGLE_NAMESPACE(qInitResources_qml)() {
    ::unitRegistry();
    return 1;
}
Q_CONSTRUCTOR_FUNCTION(QT_MANGLE_NAMESPACE(qInitResources_qml))
int QT_MANGLE_NAMESPACE(qCleanupResources_qml)() {
    return 1;
}
