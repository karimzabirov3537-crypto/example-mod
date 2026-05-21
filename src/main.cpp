#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
float g_noclipTimer = 0.0f;
bool g_isPopupOpen = false;       // Заморозит игру, пока открыта табличка
int g_reviveCount = 0;            // Счётчик использованных спасений (макс. 3)
float g_startTimer = 0.0f;        // Таймер защиты старта

class $modify(MyPlayLayer, PlayLayer) {
    void update(float dt) {
        // Если открыто окно WASTED, полностью останавливаем обновление игры
        if (g_isPopupOpen) return;

        PlayLayer::update(dt);

        g_startTimer += dt;

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
        
        // Каждую новую попытку сбрасываем все счётчики в исходное состояние
        g_reviveCount = 0;
        g_isPopupOpen = false;
        g_isNoclipActive = false;
        g_startTimer = 0.0f;
    }

    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        if (g_isNoclipActive) return;
        if (g_isPopupOpen) return;

        // Защита первых 0.5 секунд после респавна
        if (g_startTimer < 0.5f) {
            PlayLayer::destroyPlayer(p0, p1);
            return;
        }

        // Если лимит в 3 спасения ещё не исчерпан
        if (g_reviveCount < 3) {
            g_isPopupOpen = true; // Замораживаем физику уровня

            // Создаем красивое Geode-окно с вопросом
            auto popup = geode::createQuickPopup(
                "WASTED",                     // Заголовок
                "Are you sure want to die?",  // Текст вопроса
                "No!", "Yes!",                // Кнопка 1 (false), Кнопка 2 (true)
                [this, p0, p1](auto, bool clickedYes) {
                    g_isPopupOpen = false;    // Размораживаем игру после выбора

                    if (clickedYes) {
                        // Если выбрали "Yes!" — окончательно умираем
                        g_reviveCount = 3;    // Забиваем счётчик, чтобы окно больше не всплывало
                        this->destroyPlayer(p0, p1);
                    } else {
                        // Если выбрали "No!" — активируем ноуклип на 5 секунд
                        g_reviveCount++;
                        g_isNoclipActive = true;
                        g_noclipTimer = 5.0f;
                    }
                }
            );

            // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Принудительно выводим окно поверх сцены уровня, чтобы оно стало видимым
            if (popup) {
                if (auto scene = cocos2d::CCScene::get()) {
                    scene->addChild(popup, 500); // 500 гарантирует отображение на самом верхнем слое
                }
            }
            return;
        }

        // Если 3 попытки уже потрачены, то просто умираем как обычно
        PlayLayer::destroyPlayer(p0, p1);
    }
};
