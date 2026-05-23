#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

std::string getSaveKey(int levelID) {
    return "platformer_save_level_" + std::to_string(levelID);
}

class CheckpointLoadDelegate : public CCObject, public FLAlertLayerProtocol {
public:
    PlayLayer* m_layer;
    
    void FLAlert_Clicked(FLAlertLayer* alert, bool btn2) override {
        if (btn2 && m_layer && m_layer->m_player1) {
            int levelID = m_layer->m_level->m_levelID;
            auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

            float posX = static_cast<float>(savedData["player_x"].as<double>().unwrapOrDefault());
            float posY = static_cast<float>(savedData["player_y"].as<double>().unwrapOrDefault());
            
            auto p1 = m_layer->m_player1;
            p1->m_position = ccp(posX, posY);

            // ВОССТАНОВЛЕНИЕ ВСЕХ РЕЖИМОВ И СОСТОЯНИЙ
            p1->m_isShip = savedData["is_ship"].as<bool>().unwrapOrDefault();
            p1->m_isBall = savedData["is_ball"].as<bool>().unwrapOrDefault();
            p1->m_isBird = savedData["is_ufo"].as<bool>().unwrapOrDefault();
            p1->m_isDart = savedData["is_wave"].as<bool>().unwrapOrDefault();
            p1->m_isRobot = savedData["is_robot"].as<bool>().unwrapOrDefault();
            p1->m_isSpider = savedData["is_spider"].as<bool>().unwrapOrDefault();
            
            // Восстановление размера и гравитации
            p1->m_isUpsideDown = savedData["is_upside_down"].as<bool>().unwrapOrDefault();
            if (savedData["is_mini"].as<bool>().unwrapOrDefault()) {
                p1->toggleMiniMode(true);
            }

            p1->resetObject();
            m_layer->createCheckpoint(); 
            
            log::info("Successfully loaded state at: {}, {}", posX, posY);
        }
    }

    static CheckpointLoadDelegate* create(PlayLayer* layer) {
        auto ret = new CheckpointLoadDelegate();
        ret->m_layer = layer;
        ret->autorelease();
        return ret;
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        bool m_hasCheckedLoad = false;
        CheckpointLoadDelegate* m_delegate = nullptr;
    };

    bool init(GJGameLevel* level, bool usePracticeMode, bool isSfxPreview) {
        if (!PlayLayer::init(level, usePracticeMode, isSfxPreview)) return false;
        
        m_fields->m_hasCheckedLoad = false;
        m_fields->m_delegate = CheckpointLoadDelegate::create(this);
        m_fields->m_delegate->retain(); 
        return true;
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();

        if (!m_fields->m_hasCheckedLoad && this->m_level && this->m_level->isPlatformer()) {
            m_fields->m_hasCheckedLoad = true;
            
            int levelID = this->m_level->m_levelID;
            auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

            if (savedData.contains("has_save") && savedData["has_save"].as<bool>().unwrapOrDefault()) {
                auto alert = FLAlertLayer::create(
                    m_fields->m_delegate,
                    "Load Save",
                    "A checkpoint save was found from your previous session. Do you want to <cg>continue</c>?",
                    "Cancel", "OK",
                    320.f
                );
                alert->show();
            }
        }
    }

    void createCheckpoint() {
        PlayLayer::createCheckpoint();

        if (this->m_level && this->m_level->isPlatformer() && this->m_player1) {
            int levelID = this->m_level->m_levelID;
            auto p1 = this->m_player1;
            
            matjson::Value save;
            save["has_save"] = true;
            save["player_x"] = p1->m_position.x;
            save["player_y"] = p1->m_position.y;

            // ЗАПИСЬ ВСЕХ ТЕКУЩИХ РЕЖИМОВ В JSON
            save["is_ship"] = p1->m_isShip;
            save["is_ball"] = p1->m_isBall;
            save["is_ufo"] = p1->m_isBird;
            save["is_wave"] = p1->m_isDart;
            save["is_robot"] = p1->m_isRobot;
            save["is_spider"] = p1->m_isSpider;
            
            // Дополнительные параметры состояния
            save["is_upside_down"] = p1->m_isUpsideDown;
            save["is_mini"] = p1->m_muffling; // Внутреннее поле Geode для состояния мини-мода

            Mod::get()->setSavedValue(getSaveKey(levelID), save);
            Mod::get()->saveData();
        }
    }

    void onDestroy() {
        if (m_fields->m_delegate) {
            m_fields->m_delegate->release();
        }
        PlayLayer::onDestroy();
    }
};
