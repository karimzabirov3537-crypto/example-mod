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

            // 1. Рандомим цвета от 1 to 140
            int color1 = getRandomNumber(1, 140);
            int color2 = getRandomNumber(1, 140);
            
            auto gm = GameManager::sharedState();
            player->setColor(gm->colorForIdx(color1));
            player->setSecondColor(gm->colorForIdx(color2));

            // 2. Рандомим ID для всех типов иконок через GameManager, чтобы игра сама знала, какие фреймы подгружать
            int randomCube = getRandomNumber(1, 150);
            int randomShip = getRandomNumber(1, 150);
            int randomBall = getRandomNumber(1, 150);
            int randomUfo = getRandomNumber(1, 150);
            int randomWave = getRandomNumber(1, 150);
            int randomRobot = getRandomNumber(1, 150);
            int randomSpider = getRandomNumber(1, 150);
            int randomSwing = getRandomNumber(1, 150);

            // 3. Используем стандартный метод обновления кадра, который ожидает 1 аргумент
            // Передаем туда случайное число, а тип транспорта игра определит сама по текущему состоянию игрока
            player->updatePlayerFrame(randomCube);

            // 4. Главный фикс для Робота и Паука: принудительно заставляем обновить все кастомные детали
            // Метод updatePage() заставляет перерисовать абсолютно весь скелет персонажа, включая робота/паука!
            player->updatePage();
            player->updateGlowColor();
        }
    }
};

$execute {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}
