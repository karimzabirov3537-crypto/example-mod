#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
bool g_alreadyAsking = false;

class DeathQuestionDelegate : public FLAlertLayerProtocol {
    PlayLayer* m_layer;
public:
    DeathQuestionDelegate(PlayLayer* layer) : m_layer(layer) {}

    void FLAlert_Clicked(FLAlertLayer* alert, bool btn2) override {
        g_alreadyAsking = false;
        
        if (btn2) { // "No" button pressed
            g_isNoclipActive = true;
            m_layer->resume();

            // 5 seconds timer to disable noclip
            geode::Loader::get()->queueInMainThread([this]() {
                auto scheduler = CCScheduler::sharedScheduler();
                scheduler->scheduleSelector(
                    schedule_selector(DeathQuestionDelegate::disableNoclip),
                    this, 5.0f, 0, 0.0f, false
                );
            });
            
            Notification::create("Noclip active for 5 seconds!", NotificationIcon::Success)->show();
        } else { // "Yes" button pressed
            g_isNoclipActive = false;
            m_layer->resetLevel(); 
        }
        delete this; // Memory cleanup
    }

    void disableNoclip(float dt) {
        g_isNoclipActive = false;
        Notification::create("Noclip deactivated!", NotificationIcon::Warning)->show();
        CCScheduler::sharedScheduler()->unscheduleSelector(schedule_selector(DeathQuestionDelegate::disableNoclip), this);
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (g_isNoclipActive) return; 
        if (g_alreadyAsking) return; 
        g_alreadyAsking = true;

        this->pauseGame(true);

        auto delegate = new DeathQuestionDelegate(this);
        auto alert = FLAlertLayer::create(
            delegate,
            "Hold on...", 
            "Are you <cr>sure</c> you want to die?", 
            "Yes", "No", 
            300.0f
        );
        alert->show();
    }
};



