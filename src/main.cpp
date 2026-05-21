#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool g_noclipActive = false;
float g_noclipTimer = 0.0f;
int g_deathSavedCount = 0;      
bool g_isAwaitingDecision = false; 
float g_levelStartAntiBugTimer = 0.0f; // Таймер защиты от багов на старте

class MyAlertDelegate : public FLAlertLayerProtocol {
public:
    PlayLayer* m_layer;
    MyAlertDelegate(PlayLayer* l) : m_layer(l) {}

    void FLAlert_Clicked(FLAlertLayer* alert, bool selectedSecondButton) override {
        g_isAwaitingDecision = false; 

        if (selectedSecondButton) {
            // Нажали "NO" -> даем ноуклип
            g_deathSavedCount++;
            g_noclipActive = true;
            g_noclipTimer = 5.0f; 

            if (m_layer && m_layer->m_player1) m_layer->m_player1->setOpacity(100);
            if (m_layer && m_layer->m_player2) m_layer->m_player2->setOpacity(100);
            
            if (m_layer) m_layer->resume();
        } else {
            // Нажали "YES" -> обычный рестарт
            if (m_layer) {
                int tempCount = g_deathSavedCount;
                g_deathSavedCount = 999; 
                m_layer->destroyPlayer(m_layer->m_player1, nullptr);
                g_deathSavedCount = tempCount; 
            }
        }
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    
    bool init(GJGameLevel* level, bool useIndex, bool dontUseIndex) {
        if (!PlayLayer::init(level, useIndex, dontUseIndex)) return false;
        g_deathSavedCount = 0;
        g_noclipActive = false;
        g_noclipTimer = 0.0f;
        g_isAwaitingDecision = false;
        g_levelStartAntiBugTimer = 0.0f; // Сбрасываем таймер при старте
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        // Тикает таймер защиты старта уровня
        g_levelStartAntiBugTimer += dt;

        if (g_noclipActive) {
            g_noclipTimer -= dt;
            if (g_noclipTimer <= 0.0f) {
                g_noclipActive = false;
                if (m_player1) m_player1->setOpacity(255);
                if (m_player2) m_player2->setOpacity(255);
            }
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // ЕСЛИ НОУКЛИП АКТИВЕН — ЛЕТИМ ДАЛЬШЕ
        if (g_noclipActive) return;
        if (g_isAwaitingDecision) return;

        // ЗАЩИТА: Если с момента старта уровня прошло меньше 0.5 секунд — игнорируем "смерть"
        if (g_levelStartAntiBugTimer < 0.5f) {
            PlayLayer::destroyPlayer(player, object);
            return;
        }

        // Если лимит 3 жизней не исчерпан — выводим окно
        if (g_deathSavedCount < 3) {
            g_isAwaitingDecision = true;
            
            this->pauseGame(true);
            
            auto delegate = new MyAlertDelegate(this);
            auto alert = FLAlertLayer::create(delegate, "WASTED?", "Are you sure you want to die?", "Yes...", "No!", 300.f);
            alert->show();
            return;
        }

        PlayLayer::destroyPlayer(player, object);
    }
};
