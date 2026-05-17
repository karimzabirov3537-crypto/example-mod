#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
bool g_isWaitingForDecision = false;

// Опережающее объявление класса модификации, чтобы делегат о нём знал
class MyPlayLayer;

class DeathDecisionDelegate : public FLAlertLayerProtocol {
public:
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;
};

static DeathDecisionDelegate g_deathDelegate;

class $modify(MyPlayLayer, PlayLayer) {
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

    // Кастомный метод игрового класса для отключения ноуклипа
    void onNoclipTimer(float dt) {
        g_isNoclipActive = false;
        Notification::create("Noclip deactivated!", NotificationIcon::Warning)->show();
    }
};

// Реализация клика по кнопке: теперь мы безопасно вызываем таймер через MyPlayLayer
void DeathDecisionDelegate::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (btn2) {
        g_isNoclipActive = true;
        Notification::create("Saved! Noclip active!", NotificationIcon::Success)->show();

        // Запускаем планировщик Cocos2d на текущем объекте PlayLayer
        if (auto pl = PlayLayer::get()) {
            auto scheduler = cocos2d::CCDirector::sharedDirector()->getScheduler();
            
            // Вызываем наш метод onNoclipTimer через 5.0 секунд
            scheduler->scheduleSelector(
                schedule_selector(MyPlayLayer::onNoclipTimer), 
                pl, 
                0.0f, 
                0, 
                5.0f, 
                false
            );
        }
    } else {
        g_isNoclipActive = false;
        g_isWaitingForDecision = false;
        if (auto pl = PlayLayer::get()) {
            pl->destroyPlayer(pl->m_player1, nullptr);
        }
    }
    g_isWaitingForDecision = false;
}
