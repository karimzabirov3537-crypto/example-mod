#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <cstdlib>
#include <ctime>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    
    int getRandomNumber(int min, int max) {
        return min + (std::rand() % (max - min + 1));
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Запускаем стандартную смерть
        PlayLayer::destroyPlayer(player, object);

        if (player) {
            std::srand(static_cast<unsigned int>(std::time(nullptr)));

            // 1. Рандомим цвета (от 1 до 140)
            int color1 = getRandomNumber(1, 140);
            int color2 = getRandomNumber(1, 140);
            
            auto gm = GameManager::sharedState();
            player->setColor(gm->colorForIdx(color1));
            player->setSecondColor(gm->colorForIdx(color2));

            // 2. Рандомим номер фрейма
            int randomFrame = getRandomNumber(1, 150);

            // 3. Официальный метод обновления кадра из документации Geode
            player->updatePlayerFrame(randomFrame);

            // 4. Официальный метод обновления свечения и текстур из документации Geode
            player->updatePlayerGlow();
        }
    }
};

$execute {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}
