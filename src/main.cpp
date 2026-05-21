#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
float g_noclipTimer = 0.0f;
bool g_isPopupOpen = false;       // Флаг заморозки игры
int g_reviveCount = 0;            // Ограничение: 3 раза за попытку
float g_startTimer = 0.0f;        // Защита спавна

class $modify(MyPlayLayer, PlayLayer) {
    void update(float dt) {
        // Если открыто окно WASTED, полностью останавливаем обновление игры
        if (g_isPopupOpen) return;

        PlayLayer::update(dt);
        g_startTimer += dt;

        // Логика мигания/прозрачности при ноуклипе
        if (g_isNoclipActive) {
            g_noclipTimer -= dt;
            if (m_player1) m_player1->setOpacity(100);
            if (m_player2) m_player2->setOpacity(100);

            if (g_noclipTimer <= 0.0f) {
                g_isNoclipActive = false;
                if (m_player1) m_player1->setOpacity(255);
                if (m_player2) m_player2->setOpacity(255);
            }
        }
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        // Сброс всех параметров при рестарте уровня
        g_reviveCount = 0;
        g_isPopupOpen = false;
        g_isNoclipActive = false;
        g_startTimer = 0.0f;
    }

    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        // Если работает ноуклип или мы уже выбираем в меню — игнорируем урон
        if (g_isNoclipActive || g_isPopupOpen) return;

        // Защита первых 0.5 секунд после респавна
        if (g_startTimer < 0.5f) {
            PlayLayer::destroyPlayer(p0, p1);
            return;
        }

        // Если лимит в 3 спасения ещё не исчерпан, перехватываем смерть
        if (g_reviveCount < 3) {
            g_isPopupOpen = true; // Замораживаем физику мира через update()

            // Вызываем окно в главном потоке, чтобы избежать конфликтов отрисовки Cocos2d
            Loader::get()->queueInMainThread([this, p0, p1]() {
                auto popup = geode::createQuickPopup(
                    "WASTED",
                    "Are you sure want to die?",
                    "No!", "Yes!",
                    [this, p0, p1](auto, bool clickedYes) {
                        g_isPopupOpen = false; // Размораживаем обновление

                        if (clickedYes) {
                            g_reviveCount = 3; // Тратим все попытки
                            this->destroyPlayer(p0, p1); // Умираем окончательно
                        } else {
                            g_reviveCount++;
                            g_isNoclipActive = true;
                            g_noclipTimer = 5.0f; // Включаем бессмертие на 5 сек
                        }
                    }
                );

                if (popup) {
                    if (auto scene = cocos2d::CCScene::get()) {
                        scene->addChild(popup, 1050); // Максимальный приоритет видимости поверх уровня
                    }
                }
            });
            return; // Блокируем моментальный вызов оригинального экрана смерти
        }
        
        // Если 3 попытки уже потрачены, то просто умираем как обычно
        PlayLayer::destroyPlayer(p0, p1);
    }
};
