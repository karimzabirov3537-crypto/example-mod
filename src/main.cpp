#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
bool g_isWaitingForDecision = false;
float g_noclipTimer = 0.0f; // Переменная для отсчета времени

class DeathDecisionDelegate : public FLAlertLayerProtocol {
public:
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override {
        if (btn2) {
            // Игрок выбрал "Yes" -> даем ноуклип и взводим таймер на 5 секунд
            g_isNoclipActive = true;
            g_noclipTimer = 5.0f;
            Notification::create("Saved! Noclip active for 5s!", NotificationIcon::Success)->show();
        } else {
            // Игрок выбрал "No" -> позволяем ему умереть
            g_isNoclipActive = false;
            g_isWaitingForDecision = false;
            if (auto pl = PlayLayer::get()) {
                pl->destroyPlayer(pl->m_player1, nullptr);
            }
        }
        g_isWaitingForDecision = false;
    }
};

static DeathDecisionDelegate g_deathDelegate;

class $modify(MyPlayLayer, PlayLayer) {
    // Хукаем игровой цикл обновления кадра, чтобы безопасно отсчитывать секунды
    void update(float dt) {
        PlayLayer::update(dt);

        // Если ноуклип активен, уменьшаем таймер на дельта-тайм кадра
        if (g_isNoclipActive) {
            g_noclipTimer -= dt;
            if (g_noclipTimer <= 0.0f) {
                g_isNoclipActive = false;
                Notification::create("Noclip deactivated!", NotificationIcon::Warning)->show();
            }
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (g_isNoclipActive) return;
        if (g_isWaitingForDecision) return;
        g_isWaitingForDecision = true;

        auto alert = FLAlertLayer::create(
            &g_deathDelegate, 
            "Are you sure?", 
            "Do you really want to die?", 
            "No", 
            "Yes", 
            300.f
        );
        
        if (alert) {
            alert->show();
        } else {
            g_isWaitingForDecision = false;
        }
    }
};
