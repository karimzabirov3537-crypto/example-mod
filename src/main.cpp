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
        // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Если открыто окно, полностью замирифицируем кадр
        if (g_isPopupOpen) return;

        // Проверяем, врезался ли игрок, ЕЩЕ ДО того, как игра успела его стереть
        // m_isDead — это встроенное поле PlayLayer, определяющее факт аварии
        if (m_isDead && !g_isNoclipActive && !g_isPopupOpen && g_startTimer >= 0.5f) {
            if (g_reviveCount < 3) {
                m_isDead = false;    // Отменяем смерть в текущем кадре
                g_isPopupOpen = true; // Замораживаем физику мира

                // Вызываем окно через очередь главного потока, чтобы дать Cocos2d прогрузить UI
                Loader::get()->queueInMainThread([this]() {
                    auto popup = geode::createQuickPopup(
                        "WASTED",
                        "Are you sure want to die?",
                        "No!", "Yes!",
                        [this](auto, bool clickedYes) {
                            g_isPopupOpen = false; // Размораживаем обновление

                            if (clickedYes) {
                                g_reviveCount = 3; // Тратим все попытки
                                this->destroyPlayer(m_player1, nullptr); // Умираем окончательно
                            } else {
                                g_reviveCount++;
                                g_isNoclipActive = true;
                                g_noclipTimer = 5.0f; // Включаем бессмертие на 5 сек
                            }
                        }
                    );

                    if (popup) {
                        if (auto scene = cocos2d::CCScene::get()) {
                            scene->addChild(popup, 1050); // Максимальный приоритет видимости
                        }
                    }
                });
                return;
            }
        }

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
        // Сброс при рестарте уровня
        g_reviveCount = 0;
        g_isPopupOpen = false;
        g_isNoclipActive = false;
        g_startTimer = 0.0f;
    }

    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        // Если работает ноуклип или мы в режиме выбора — игнорируем урон
        if (g_isNoclipActive || g_isPopupOpen) return;

        // Если лимит жизней исчерпан (больше 3), отдаем управление стандартной смерти
        if (g_reviveCount >= 3) {
            PlayLayer::destroyPlayer(p0, p1);
            return;
        }
        
        // В остальных случаях блокируем деструктор, так как за него отвечает проверка в update
        if (g_startTimer < 0.5f) {
            PlayLayer::destroyPlayer(p0, p1);
        }
    }
};
