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
        PlayLayer::destroyPlayer(player, object);

        if (player) {
            std::srand(static_cast<unsigned int>(std::time(nullptr)));

            // 1. Рандомим цвета от 1 до 140
            int color1 = getRandomNumber(1, 140);
            int color2 = getRandomNumber(1, 140);
            
            auto gm = GameManager::sharedState();
            player->setColor(gm->colorForIdx(color1));
            player->setSecondColor(gm->colorForIdx(color2));

            // 2. Рандомим кадры для каждого типа транспорта отдельно (как требует Geode SDK)
            player->updatePlayerCubeFrame(getRandomNumber(1, 150));
            player->updatePlayerShipFrame(getRandomNumber(1, 150));
            player->updatePlayerBallFrame(getRandomNumber(1, 150));
            player->updatePlayerUfoFrame(getRandomNumber(1, 150));
            player->updatePlayerWaveFrame(getRandomNumber(1, 150));
            player->updatePlayerSwingFrame(getRandomNumber(1, 150));

            // 3. Исправляем баг Мегахака с Роботом и Пауком:
            // Обновляем базовый фрейм и принудительно заставляем пересоздать их детали
            player->updatePlayerRobotFrame(getRandomNumber(1, 150));
            player->updatePlayerSpiderFrame(getRandomNumber(1, 150));

            // Полностью обновляем цвета и текстуры иконки
            player->updateGlowColor();
        }
    }
};

$execute {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}
