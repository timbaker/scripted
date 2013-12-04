#include "luautils.h"

extern "C" {

// see luaconf.h
// these are where print() calls go
void luai_writestring(const char *s, int len)
{
}

void luai_writeline()
{
}

} // extern "C"


LuaState::LuaState()
{
    if (L = luaL_newstate()) {
        luaL_openlibs(L);

        lua_pushboolean(L, true);
        lua_setglobal(L, "_SCRIPTED_");
    }
}

LuaState::~LuaState()
{
    if (L)
        lua_close(L);
}

bool LuaState::loadFile(const QString &fileName)
{
    int status = luaL_loadfile(L, fileName.toLatin1().data());
    if (status != LUA_OK)
        return false;

    status = lua_pcall(L, 0, 0, -1);
    if (status != LUA_OK)
        return false;

    return true;
}

LuaValue LuaState::getGlobal(const QString &name)
{
    lua_getglobal(L, name.toLatin1().data());
    LuaValue v = toValue(-1);
    lua_pop(L, 1);
    return v;
}

LuaValue LuaState::toValue(int stackIndex)
{
    if (stackIndex < 0)
        stackIndex = lua_gettop(L) - qAbs(stackIndex) + 1;
    LuaValue v;
    v.mType = lua_type(L, stackIndex);
    switch (v.mType) {
    case LUA_TNONE:
        break;
    case LUA_TNIL:
        break;
    case LUA_TNUMBER:
        v.mNumberValue = lua_tonumber(L, stackIndex);
        break;
    case LUA_TBOOLEAN:
        v.mBooleanValue = lua_toboolean(L, stackIndex);
        break;
    case LUA_TSTRING:
        v.mStringValue = QLatin1String(lua_tostring(L, stackIndex));
        break;
    case LUA_TTABLE:
        v.mTableValue = toTableValue(stackIndex);
        break;
    }
    return v;
}

LuaTableValue LuaState::toTableValue(int stackIndex)
{
    if (stackIndex < 0)
        stackIndex = lua_gettop(L) - qAbs(stackIndex) + 1;
    LuaTableValue tv;
    if (!lua_istable(L, stackIndex))
        return tv;
    lua_pushnil(L); // space for key
    while (lua_next(L, stackIndex) != 0) { // pop a key, push next key, push next value
        tv.mKeys += new LuaValue(toValue(-2));
        tv.mValues += new LuaValue(toValue(-1));
        lua_pop(L, 1); // pop value
    }
    return tv;
}

/////

LuaTableValue::LuaTableValue(const LuaTableValue &other)
{
    foreach (LuaValue *v, other.mKeys)
        mKeys += new LuaValue(*v);
    foreach (LuaValue *v, other.mValues)
        mValues += new LuaValue(*v);
}

LuaTableValue &LuaTableValue::operator=(const LuaTableValue &other)
{
    foreach (LuaValue *v, other.mKeys)
        mKeys += new LuaValue(*v);
    foreach (LuaValue *v, other.mValues)
        mValues += new LuaValue(*v);
    return *this;
}

LuaTableValue::LuaTableValue()
{

}

QMap<QString, QString> LuaTableValue::toStringStringMap()
{
    QMap<QString,QString> ret;
    for (int i = 0; i < size(); i++) {
        LuaValue *k = mKeys[i];
        LuaValue *v = mValues[i];
        ret[k->toString()] = v->toString();
    }
    return ret;
}
