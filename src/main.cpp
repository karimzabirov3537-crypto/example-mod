#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
bool g_isWaitingForDecision = false;

// Класс-делегат, обрабатывающий кнопки "Да" или "Нет"
class DeathDecisionDelegate : public FLAlertLayerProtocol {
public:
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override {
        if (btn2) {
            // Игрок выбрал "Да" -> включаем бессмертие
            g_isNoclipActive = true;
            Notification::create("Saved! Noclip for 5 seconds!", NotificationIcon::Success)->show();

            // Создаем отдельный поток-таймер для отключения флага через 5 секунд
            std::thread([]() {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                geode::Loader::get()->queueInMainThread([]() {
                    g_isNoclipActive = false;
                    Notification::create("Noclip deactivated!", NotificationIcon::Warning)->show();
                });
            }).detach();

        } else {
            // Игрок выбрал "Нет" -> позволяем ему умереть
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
        // Если ноуклип уже запущен — игнорируем любой урон
        if (g_isNoclipActive) return;

        // Если окно выбора уже висит — не спамим им при повторных касаниях блоков
        if (g_isWaitingForDecision) return;
        g_isWaitingForDecision = true;

        // Открываем окно. Строки сделали максимально простыми без лишних тегов
        auto alert = FLAlertLayer::create(
            &g_deathDelegate, 
            "Are you sure?", 
            "Do you really want to die?", 
            "No", 
            "Yes",  
            300.f
        );
        alert->show();
    }
};
