#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
float g_noclipTimer = 0.0f;

class $modify(MyPlayLayer, PlayLayer) {
    // Используем безопасный игровой цикл для отсчета 5 секунд ноуклипа
    void update(float dt) {
        PlayLayer::update(dt);

        if (g_isNoclipActive) {
            g_noclipTimer -= dt;

            // Пока ноуклип активен, делаем иконки игрока прозрачными (50%)
            if (m_player1) m_player1->setOpacity(100);
            if (m_player2) m_player2->setOpacity(100);

            if (g_noclipTimer <= 0.0f) {
                g_isNoclipActive = false;

                // Время вышло – возвращаем обычную полную непрозрачность (100%)
                if (m_player1) m_player1->setOpacity(255);
                if (m_player2) m_player2->setOpacity(255);
            }
        }
    }

    // Хукаем продолжение игры после паузы
    void resumeAndStopPause() {
        PlayLayer::resume();

        // Если игра была поставлена на паузу нашей смертью – активируем ноуклип
        if (g_noclipTimer == -1.0f) {
            g_isNoclipActive = true;
            g_noclipTimer = 5.0f; // 5 секунд бессмертия
        }
    }

    void destroyPlayer(PlayerObject* p0, GameObject* p1) {
        // Если ноуклип уже активен – летим дальше
        if (g_isNoclipActive) return;

        // Вместо смерти принудительно включаем стандартную игровую паузу
        if (g_noclipTimer <= 0.0f) {
            g_noclipTimer = -1.0f; // Ставим маркер, что пауза вызвана смертью
            
            // Вызываем стандартное меню паузы через Geode API
            this->onPause(nullptr); 
            return;
        }

        // Если маркер сброшен – обычная смерть
        PlayLayer::destroyPlayer(p0, p1);
    }
};
