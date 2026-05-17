#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_isNoclipActive = false;
bool g_isWaitingForDecision = false;

// Создаем кастомный делегат, чтобы поймать нажатие кнопок в окне
class DeathDecisionDelegate : public FLAlertLayerProtocol {
public:
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override {
        // btn2 — это вторая кнопка (обычно правая, то есть "Да")
        if (btn2) {
            // Игрок выбрал "Да" -> включаем ноуклип на 5 секунд
            g_isNoclipActive = true;
            Notification::create("Засейвлен! 5 секунд ноуклипа!", NotificationIcon::Success)->show();

            // Запускаем безопасный таймер отключения через шедулер Cocos2d
            auto scheduler = cocos2d::CCDirector::sharedDirector()->getScheduler();
            scheduler->scheduleSelector(
                schedule_selector(MyPlayLayer::disableNoclip), 
                MyPlayLayer::get(), // Получаем текущий синглтон PlayLayer
                0.0f, 0, 5.0f, false
            );
        } else {
            // Игрок выбрал "Нет" -> позволяем ему умереть
            g_isNoclipActive = false;
            if (auto pl = PlayLayer::get()) {
                // Вызываем оригинальную смерть, убрав флаг ожидания
                g_isWaitingForDecision = false;
                // Передаем заглушки, так как нам важен сам факт триггера смерти
                pl->destroyPlayer(pl->m_player1, nullptr); 
            }
        }
        g_isWaitingForDecision = false;
    }
};

// Выносим делегат в глобальную область, чтобы он не удалился из памяти
static DeathDecisionDelegate g_deathDelegate;

class $modify(MyPlayLayer, PlayLayer) {
    // Выносим функцию отключения в кастомные методы
    struct Fields {
        bool dummy; 
    };

    void disableNoclip(float dt) {
        g_isNoclipActive = false;
        Notification::create("Ноуклип кончился, аккуратно!", NotificationIcon::Warning)->show();
    }

    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        // 1. Если активен ноуклип — игнорируем урон
        if (g_isNoclipActive) return;

        // 2. Если окно уже открыто — не спамим им при повторных касаниях в этот же кадр
        if (g_isWaitingForDecision) return;

        g_isWaitingForDecision = true;

        // 3. Показываем всплывающее окно Geode/GD
        auto alert = FLAlertLayer::create(
            &g_deathDelegate, 
            "Вы точно?", 
            "Вы уверены, что хотите <cr>умереть</c>?", 
            "Нет", // Первая кнопка (btn1 = false)
            "Да",  // Вторая кнопка (btn2 = true)
            300.f
        );
        alert->show();
    }
};
