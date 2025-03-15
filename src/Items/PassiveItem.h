#ifndef PASSIVEITEM_H
#define PASSIVEITEM_H

#include "../Core/GameObjectBase.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class PassiveItem : public ETG::GameObjectBase {
public:
    explicit PassiveItem(const std::string& texturePath);
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void PlayEquipSound();
    void Equip();
private:
    sf::Sound equipSound;
    sf::SoundBuffer equipSoundBuffer;
};

#endif