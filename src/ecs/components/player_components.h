#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <utils/weapon.h>

struct PlayerComponent
{
    size_t number = 0;
    std::unordered_map<WeaponType, WeaponProps> weapons;
    WeaponType currentWeapon = WeaponType::Uzi;
    glm::vec2 weaponDirection = {1, 0};
    size_t countOfGroundContacts = 0; // Flag indicating whether the player can jump.
};
