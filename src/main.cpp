#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Глобальные переменные для контроля ноуклипа
bool g_noclipActive = false;
float g_noclipTimer = 0.0f;
int g_deathSavedCount = 0;      
bool g_isAwaitingDecision = false; 

class MyAlertDelegate : public FLAlertLayerProtocol {
public:
    PlayLayer* m_layer;

    // Передаем только слой, это на 100% защищает от вылетов при старте
    MyAlertDelegate(PlayLayer* l) : m_layer(l) {}

    void FLAlert_Clicked(FLAlertLayer* alert, bool selectedSecondButton) override {
        g_isAwaitingDecision = false; 

        if (selectedSecondButton) {
            // Нажали "NO" -> включаем временный ноуклип и возвращаем в игру
            g_deathSavedCount++;
            g_noclipActive = true;
            g_noclipTimer = 5.0f; 

            if (m_layer && m_layer->m_player1) {
                m_layer->m_player1->setOpacity(100);
            }
            if (m_layer && m_layer->m_player2) {
                m_layer->m_player2->setOpacity(100);
            }
            
            if (m_layer) m_layer->resume();
        } else {
            // Нажали "YES" -> убиваем по-честному и сбрасываем уровень
            if (m_layer) {
                m_layer->resetLevel();
            }
        }
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    
    bool init(GJGameLevel* level, bool useIndex, bool dontUseIndex) {
        if (!PlayLayer::init(level, useIndex, dontUseIndex)) return false;
        
        // Сбрасываем счётчик жизней при каждом новом входе на уровень
        g_deathSavedCount = 0;
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

    void resetLevel() {
        // НАДЕЖНАЯ ЗАЩИТА: Если кнопка паузы еще не создана — это стартовый сброс игры. Игнорируем!
        if (!this->getChildByID("pause-button")) {
            PlayLayer::resetLevel();
            return;
        }

        if (g_noclipActive) return;
        if (g_isAwaitingDecision) return;

        // Если лимит 3 жизней не израсходован — выводим наше окно
        if (g_deathSavedCount < 3) {
            g_isAwaitingDecision = true;
            
            this->pauseGame(true);
            
            auto delegate = new MyAlertDelegate(this);
            auto alert = FLAlertLayer::create(delegate, "WASTED?", "Are you sure you want to die?", "Yes...", "No!", 300.f);
            alert->show();
            return;
        }

        // Если все попытки кончились — обычный рестарт
        PlayLayer::resetLevel();
    }
};
