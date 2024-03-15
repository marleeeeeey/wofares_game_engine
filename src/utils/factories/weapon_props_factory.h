#pragma once
#include <unordered_map>
#include <utils/weapon.h>

class WeaponPropsFactory
{
public:
    static WeaponProps CreateWeaponType(WeaponType type);
    static std::unordered_map<WeaponType, WeaponProps> CreateAllWeaponsSet();
private:
    static WeaponProps CreateBazooka();
    static WeaponProps CreateGrenade();
    static WeaponProps CreateUzi();
    static WeaponProps CreateShotgun();
};