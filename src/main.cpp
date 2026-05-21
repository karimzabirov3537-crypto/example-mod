#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_noclipActive = false;
float g_noclipTimer = 0.0f;
int g_deathSavedCount = 0;      
bool g_isAwaitingDecision = false; 

class MyAlertDelegate : public FLAlertLayerProtocol {
public:
    PlayerObject* m_player;
    PlayLayer* m_layer;

    MyAlertDelegate(PlayerObject* p, PlayLayer* l) : m_player(p), m_layer(l) {}

    void FLAlert_Clicked(FLAlertLayer* alert, bool selectedSecondButton) override {
        g_isAwaitingDecision = false; 

        if (selectedSecondButton) {
            // Нажали "NO" -> включаем временный ноуклип и возвращаем в игру
            g_deathSavedCount++;
            g_noclipActive = true;
            g_noclipTimer = 5.0f; 

            if (m_player) {
                m_player->setOpacity(100);
            }
            
            m_layer->resume();
        } else {
            // Нажали "YES" -> сбрасываем уровень, как игра и хотела
            g_isAwaitingDecision = false;
            if (m_layer) {
                m_layer->resetLevel();
            }
        }
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    
    bool init(GJGameLevel* level, bool useIndex, bool dontUseIndex) {
        if (!PlayLayer::init(level, useIndex, dontUseIndex)) return false;
        // Сбрасываем счётчик только при полной ручной перезагрузке уровня
        if (g_deathSavedCount >= 3) {
            g_deathSavedCount = 0;
        }
        g_noclipActive = false;
        g_noclipTimer = 0.0f;
        g_isAwaitingDecision = false;
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        if (g_noclipActive) {
            g_noclipTimer -= dt;
            if (g_noclipTimer <= 0.0f) {
                g_noclipActive = false;
                if (m_player1) m_player1->setOpacity(255);
                if (m_player2) m_player2->setOpacity(255);
            }
        }
    }

    // Хукаем момент, когда игра ОФИЦИАЛЬНО запускает процесс перезапуска после смерти
    void resetLevel() {
        // Если ноуклип активен — просто продолжаем лететь
        if (g_noclipActive) return;
        if (g_isAwaitingDecision) return;

        // Если лимит 3 жизней не исчерпан — перехватываем рестарт и выводим окно!
        if (g_deathSavedCount < 3) {
            g_isAwaitingDecision = true;
            
            // Замораживаем игру
            this->pauseGame(true);
            
            auto delegate = new MyAlertDelegate(m_player1, this);
            auto alert = FLAlertLayer::create(delegate, "WASTED?", "Are you sure you want to die?", "Yes...", "No!", 300.f);
            alert->show();
            return;
        }

        // Если попытки кончились — запускаем обычный рестарт
        PlayLayer::resetLevel();
    }
};
