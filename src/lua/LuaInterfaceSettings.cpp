// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "defines.h" // IWYU pragma: keep
#include "LuaInterfaceSettings.h"
#include "GameServer.h"
#include "LuaServerPlayer.h"
#include "addons/const_addons.h"
#include "libutil/src/Log.h"

// Include last!
#include "DebugNew.h" // IWYU pragma: keep

inline void check(bool testValue, const std::string& error)
{
    if(!testValue)
        throw std::runtime_error(error);
}

LuaInterfaceSettings::LuaInterfaceSettings()
{
    Register(lua);
    LuaServerPlayer::Register(lua);
    lua["rttr"] = this;
}

LuaInterfaceSettings::~LuaInterfaceSettings()
{}

void LuaInterfaceSettings::Register(kaguya::State& state)
{
    state["RTTRSettings"].setClass(kaguya::ClassMetatable<LuaInterfaceSettings, LuaInterfaceBase>()
        .addMemberFunction("GetPlayerCount", &LuaInterfaceSettings::GetPlayerCount)
        .addMemberFunction("GetPlayer", &LuaInterfaceSettings::GetPlayer)
        .addMemberFunction("SetAddon", &LuaInterfaceSettings::SetAddon)
        .addMemberFunction("SetAddon", &LuaInterfaceSettings::SetBoolAddon)
        .addMemberFunction("ResetAddons", &LuaInterfaceSettings::ResetAddons)
        .addMemberFunction("SetGameSettings", &LuaInterfaceSettings::SetGameSettings)
        );

    state["AddonId"].setClass(kaguya::ClassMetatable<AddonId>());

    for(unsigned i = 0; i < AddonId::count_; ++i)
    {
        AddonId id = AddonId::type_(AddonId::values_()[i]);
        state[std::string("ADDON_") + id.toString()] = static_cast<AddonId const&>(id);
    }
}

unsigned LuaInterfaceSettings::GetPlayerCount()
{
    return GAMESERVER.GetMaxPlayerCount();
}

LuaServerPlayer LuaInterfaceSettings::GetPlayer(unsigned idx)
{
    check(idx < GetPlayerCount(), "Invalid player idx");
    return LuaServerPlayer(idx);
}

void LuaInterfaceSettings::SetAddon(AddonId id, unsigned value)
{
    GlobalGameSettings ggs = GAMESERVER.GetGGS();
    ggs.setSelection(id, value);
    GAMESERVER.ChangeGlobalGameSettings(ggs);
}

void LuaInterfaceSettings::SetBoolAddon(AddonId id, bool value)
{
    SetAddon(id, 1);
}

void LuaInterfaceSettings::ResetAddons()
{
    GlobalGameSettings ggs = GAMESERVER.GetGGS();
    for(unsigned i = 0; i < ggs.getCount(); ++i)
    {
        unsigned int status;
        const Addon* addon = ggs.getAddon(i, status);

        if(!addon)
            continue;
        ggs.setSelection(addon->getId(), addon->getDefaultStatus());
    }
    GAMESERVER.ChangeGlobalGameSettings(ggs);
}

void LuaInterfaceSettings::SetGameSettings(const kaguya::LuaTable& settings)
{
    GlobalGameSettings ggs = GAMESERVER.GetGGS();
    if(settings.getField("speed"))
    {
        GlobalGameSettings::GameSpeed speed = settings.getField("speed");
        check(unsigned(speed) <= GlobalGameSettings::GS_VERYFAST, "Speed is invalid");
        ggs.game_speed = speed;
    }
    if(settings.getField("objective"))
    {
        GlobalGameSettings::GameObjective objective = settings.getField("objective");
        check(unsigned(objective) <= GlobalGameSettings::GO_TOTALDOMINATION, "Objective is invalid");
        ggs.game_objective = objective;
    }
    if(settings.getField("startWares"))
    {
        GlobalGameSettings::StartWares wares = settings.getField("startWares");
        check(unsigned(wares) <= GlobalGameSettings::SWR_ALOT, "Start wares is invalid");
        ggs.start_wares = wares;
    }
    if(settings.getField("fow"))
    {
        GlobalGameSettings::Exploration fow = settings.getField("fow");
        check(unsigned(fow) <= GlobalGameSettings::EXP_FOGOFWARE_EXPLORED, "FoW is invalid");
        ggs.exploration = fow;
    }
    if(settings.getField("lockedTeams"))
        ggs.lock_teams = settings.getField("lockedTeams");
    if(settings.getField("teamView"))
        ggs.team_view = settings.getField("teamView");
    if(settings.getField("randomStartPosition"))
        ggs.random_location = settings.getField("randomStartPosition");
    GAMESERVER.ChangeGlobalGameSettings(ggs);
}

kaguya::LuaRef LuaInterfaceSettings::GetAllowedChanges()
{
    kaguya::LuaRef getGeneralConfig = lua["getAllowedChanges"];
    if(getGeneralConfig.type() == LUA_TFUNCTION)
    {
        kaguya::LuaRef cfg = getGeneralConfig.call<kaguya::LuaRef>();
        if(cfg.type() == LUA_TTABLE)
            return cfg;
    }
    return kaguya::LuaRef();
}

bool LuaInterfaceSettings::IsChangeAllowed(const std::string& name, const bool defaultVal/* = false*/)
{
    kaguya::LuaRef cfg = GetAllowedChanges();
    if(cfg.isNilref())
        return defaultVal;
    cfg = cfg.getField(name);
    if(cfg.typeTest<bool>())
        return cfg;
    else
        return defaultVal;
}

std::vector<AddonId> LuaInterfaceSettings::GetAllowedAddons()
{
    kaguya::LuaRef getAllowedAddons = lua["getAllowedAddons"];
    if(getAllowedAddons.type() == LUA_TFUNCTION)
    {
        kaguya::LuaRef addons = getAllowedAddons();
        if(addons.typeTest<std::vector<AddonId> >())
            return addons;
        else
            LOG.lprintf("Invalid type returned by getAllowedAddons");
    }
    return std::vector<AddonId>();
}

void LuaInterfaceSettings::EventSettingsReady()
{
    kaguya::LuaRef onGameFrame = lua["onSettingsReady"];
    if(onGameFrame.type() == LUA_TFUNCTION)
        onGameFrame.call<void>();

}
