#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
bool g_isWaitingForDecision = false;

class DeathDecisionDelegate : public FLAlertLayerProtocol {
public:
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override {
        if (btn2) {
            g_isNoclipActive = true;
            Notification::create("Saved! Noclip active!", NotificationIcon::Success)->show();

            std::thread([]() {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                geode::Loader::get()->queueInMainThread([]() {
                    g_isNoclipActive = false;
                    Notification::create("Noclip deactivated!", NotificationIcon::Warning)->show();
                });
            }).detach();

        } else {
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
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (g_isNoclipActive) return;
        if (g_isWaitingForDecision) return;
        g_isWaitingForDecision = true;

        // Создаем объект окна напрямую через выделение памяти, минуя макрос ::create
        auto alert = new FLAlertLayer();
        if (alert) {
            // Инициализируем напрямую через метод init
            if (alert->init(&g_deathDelegate, "Are you sure?", "Do you really want to die?", "No", "Yes", 300.f, false, nullptr, 1.f)) {
                alert->autorelease();
                alert->show();
            } else {
                delete alert;
                g_isWaitingForDecision = false;
            }
        }
    }
};
