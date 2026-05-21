#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
float g_noclipTimer = 0.0f;
bool g_isPausingRightNow = false;
float g_startTimer = 0.0f; // Наш таймер защиты старта

class $modify(MyPlayLayer, PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);

        // Считаем время от начала уровня
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

    void resumeAndStopPause() {
        PlayLayer::resume();

        g_isPausingRightNow = false;

        if (g_noclipTimer == -1.0f) {
            g_isNoclipActive = true;
            g_noclipTimer = 5.0f;
        }
    }

    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        if (g_isNoclipActive) return;
        if (g_isPausingRightNow) return;

        // ЗАЩИТА: Если с момента старта прошло меньше полсекунды - не о
        if (g_startTimer < 0.5f) {
            PlayLayer::destroyPlayer(p0, p1);
            return;
        }

        if (g_noclipTimer <= 0.0f) {
            g_noclipTimer = -1.0f;
            g_isPausingRightNow = true;

            // БЕЗОПАСНЫЙ СПОСОБ: Ищем кнопку паузы на UI-слое и вызываем её функцию клика напрямую
            if (auto uiLayer = this->getChildByType<UILayer>(0)) {
                if (auto pauseBtn = uiLayer->getChildByID("pause-button")) {
                    // Симулируем нажатие кнопки паузы через Cocos2d
                    if (auto btnItem = cocos2d::typeinfo_cast<cocos2d::CCMenuItem*>(pauseBtn)) {
                        btnItem->activate();
                        return;
                    }
                }
            }

            // Запасной вариант, если кнопка по ID не нашлась
            PlayLayer::pushButton(1, true); 
            PlayLayer::releaseButton(1, true);
            return;
        }

        PlayLayer::destroyPlayer(p0, p1);
    }
};
