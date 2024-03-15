#include "weapon_props_factory.h"
#include <stdexcept>

WeaponProps WeaponPropsFactory::CreateWeaponType(WeaponType type)
{
    switch (type)
    {
    case WeaponType::Bazooka:
        return CreateBazooka();
    case WeaponType::Grenade:
        return CreateGrenade();
    case WeaponType::Uzi:
        return CreateUzi();
    case WeaponType::Shotgun:
        return CreateShotgun();
    default:
        throw std::runtime_error("Unknown weapon type.");
    }
}

std::unordered_map<WeaponType, WeaponProps> WeaponPropsFactory::CreateAllWeaponsSet()
{
    std::unordered_map<WeaponType, WeaponProps> weapons;
    weapons[WeaponType::Bazooka] = CreateBazooka();
    weapons[WeaponType::Grenade] = CreateGrenade();
    weapons[WeaponType::Uzi] = CreateUzi();
    weapons[WeaponType::Shotgun] = CreateShotgun();
    return weapons;
}

WeaponProps WeaponPropsFactory::CreateBazooka()
{
    WeaponProps props;
    props.type = WeaponType::Bazooka;
    props.bulletMass = 0.5;
    props.bulletEjectionForce = 0.1;
    props.projectileSizeWorld = {3, 3};
    props.damageRadiusWorld = 25;
    props.damageForce = 10;
    props.ammoInStorage = 10;
    props.ammoInClip = 1;
    props.clipSize = 1;
    props.reloadTime = 2.0f;
    props.fireRate = 1.0f;
    return props;
}

WeaponProps WeaponPropsFactory::CreateGrenade()
{
    WeaponProps props = CreateBazooka();
    props.type = WeaponType::Grenade;
    props.bulletMass = 0.3;
    props.bulletEjectionForce = 0.0;
    props.damageRadiusWorld = 20;
    props.damageForce *= 0.7;
    props.ammoInStorage = 5;
    props.ammoInClip = 1;
    props.clipSize = 1;
    props.reloadTime = 1.0f;
    props.fireRate = 1.0f;
    return props;
}

WeaponProps WeaponPropsFactory::CreateUzi()
{
    WeaponProps props = CreateBazooka();
    props.type = WeaponType::Uzi;
    props.bulletMass = 0.05;
    props.bulletEjectionForce = 0.05;
    props.damageRadiusWorld = 10;
    props.damageForce = 5;
    props.ammoInStorage = 120;
    props.ammoInClip = 30;
    props.clipSize = 30;
    props.reloadTime = 3.0f;
    props.fireRate = 0.1f;
    return props;
}

WeaponProps WeaponPropsFactory::CreateShotgun()
{
    WeaponProps props = CreateUzi();
    props.type = WeaponType::Shotgun;
    props.bulletMass = 0.1;
    props.bulletEjectionForce = 3;
    props.damageRadiusWorld = 20;
    props.damageForce *= 10;
    props.ammoInStorage = 60;
    props.ammoInClip = 15;
    props.clipSize = 15;
    props.reloadTime = 2.0f;
    props.fireRate = 1.0f;
    return props;
}