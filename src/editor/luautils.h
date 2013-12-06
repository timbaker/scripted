#ifndef LUAUTILS_H
#define LUAUTILS_H

#include <QList>
#include <QMap>
#include <QString>

extern "C" {

#include "lualib.h"
#include "lauxlib.h"

}

class LuaValue;

class LuaTableValue
{
public:
    LuaTableValue();
    LuaTableValue(const LuaTableValue &other);
    LuaTableValue &operator=(const LuaTableValue &other);

    ~LuaTableValue()
    {
        qDeleteAll(mKeys);
        qDeleteAll(mValues);
    }

    int size() const { return mKeys.size(); }
    QMap<QString,QString> toStringStringMap();

    QList<LuaValue*> mKeys;
    QList<LuaValue*> mValues;
};

class LuaValue
{
public:
    LuaValue() :
        mType(LUA_TNONE)
    {
    }
#if 0
    LuaValue(const LuaValue &other) :
        mType(other.mType),
        mNumberValue(other.mNumberValue),
        mStringValue(other.mStringValue),
        mBooleanValue(other.mBooleanValue),
        mTableValue(other.mTableValue)
    {
    }
#endif
    int type() const { return mType; }

    bool isString(const QString &v) const
    { return mType == LUA_TSTRING && mStringValue == v; }

    QString toString() const
    {
        switch (mType)
        {
        case LUA_TNONE: return QString();
        case LUA_TNIL: return QString();
        case LUA_TNUMBER: return QString::number(mNumberValue);
        case LUA_TBOOLEAN: return QLatin1String(mBooleanValue ? "true" : "false");
        case LUA_TSTRING: return mStringValue;
        case LUA_TTABLE: return QString();
        }
        return QString();
    }

    bool isTable() const
    { return mType == LUA_TTABLE; }

    int mType; // LUA_TNONE, LUA_TNIL, etc
    double mNumberValue;
    QString mStringValue;
    bool mBooleanValue;
    LuaTableValue mTableValue;
};

class LuaState
{
public:
    LuaState();
    ~LuaState();

    bool loadFile(const QString &fileName);
    bool loadString(const QString &str, const QString &name);
    LuaValue getGlobal(const QString &name);
    LuaValue toValue(int stackIndex);
    LuaTableValue toTableValue(int stackIndex);

    QString errorString() { return mError; }

private:
    lua_State *L;
    QString mError;
};

#endif // LUAUTILS_H
