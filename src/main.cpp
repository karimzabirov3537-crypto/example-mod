#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
float g_noclipTimer = 0.0f;
bool g_isPausingRightNow = false; // Новый маркер, чтобы не спамить паузу

class $modify(MyPlayLayer, PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);

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
        
        g_isPausingRightNow = false; // Игрок снял с паузы — сбрасываем защиту

        if (g_noclipTimer == -1.0f) {
            g_isNoclipActive = true;
            g_noclipTimer = 5.0f; 
        }
    }

    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        if (g_isNoclipActive) return;
        if (g_isPausingRightNow) return; // Если пауза УЖЕ нажимается в этом кадре — игнорируем смерть!

        if (g_noclipTimer <= 0.0f) {
            g_noclipTimer = -1.0f; 
            g_isPausingRightNow = true; // Запрещаем повторный вызов функции паузы
            
            auto dispatcher = CCDirector::sharedDirector()->getKeyboardDispatcher();
            dispatcher->dispatchKeyboardMSG(enumKeyCodes::KEY_Escape, true, false, 0.0);
            return;
        }

        PlayLayer::destroyPlayer(p0, p1);
    }
};
