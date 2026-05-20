#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <cstdlib>
#include <ctime>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    
    // Вспомогательная функция для случайных чисел
    int getRandomNumber(int min, int max) {
        return min + (std::rand() % (max - min + 1));
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Запускаем стандартную смерть
        PlayLayer::destroyPlayer(player, object);

        if (player) {
            // Инициализируем генератор случайных чисел
            std::srand(static_cast<unsigned int>(std::time(nullptr)));

            // 1. Рандомим цвета от 1 до 140
            int color1 = getRandomNumber(1, 140);
            int color2 = getRandomNumber(1, 140);
            
            auto gm = GameManager::sharedState();
            player->setColor(gm->colorForIdx(color1));
            player->setSecondColor(gm->colorForIdx(color2));

            // 2. Рандомим основные типы транспорта (кадры 1-150)
            player->updatePlayerFrame(getRandomNumber(1, 150), IconType::Cube);
            player->updatePlayerFrame(getRandomNumber(1, 150), IconType::Ship);
            player->updatePlayerFrame(getRandomNumber(1, 150), IconType::Ball);
            player->updatePlayerFrame(getRandomNumber(1, 150), IconType::Ufo);
            player->updatePlayerFrame(getRandomNumber(1, 150), IconType::Wave);
            player->updatePlayerFrame(getRandomNumber(1, 150), IconType::Swing);

            // 3. Исправляем баг Мегахака с Роботом и Пауком
            player->updatePlayerFrame(getRandomNumber(1, 150), IconType::Robot);
            player->updatePlayerFrame(getRandomNumber(1, 150), IconType::Spider);

            // Принудительно заставляем обновиться анимационный скелет
            player->updateGlowColor();
        }
    }
};

$execute {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}
