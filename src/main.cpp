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

            // ВОССТАНОВЛЕНИЕ РЕЖИМОВ ЧЕРЕЗ ОФИЦИАЛЬНЫЕ МЕТОДЫ ИГРЫ
            bool isShip = savedData["is_ship"].as<bool>().unwrapOrDefault();
            bool isBall = savedData["is_ball"].as<bool>().unwrapOrDefault();
            bool isUfo = savedData["is_ufo"].as<bool>().unwrapOrDefault();
            bool isWave = savedData["is_wave"].as<bool>().unwrapOrDefault();
            bool isRobot = savedData["is_robot"].as<bool>().unwrapOrDefault();
            bool isSpider = savedData["is_spider"].as<bool>().unwrapOrDefault();
            
            // Сбрасываем в куб, если все режимы отключены, иначе включаем нужный
            p1->m_isShip = isShip;
            p1->m_isBall = isBall;
            p1->m_isBird = isUfo;
            p1->m_isDart = isWave;
            p1->m_isRobot = isRobot;
            p1->m_isSpider = isSpider;

            // Восстановление гравитации
            p1->m_isUpsideDown = savedData["is_upside_down"].as<bool>().unwrapOrDefault();
            
            // Восстановление мини-мода через официальный сеттер Cocos/GD
            bool isMini = savedData["is_mini"].as<bool>().unwrapOrDefault();
            p1->m_isMini = isMini;

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

            save["is_ship"] = p1->m_isShip;
            save["is_ball"] = p1->m_isBall;
            save["is_ufo"] = p1->m_isBird;
            save["is_wave"] = p1->m_isDart;
            save["is_robot"] = p1->m_isRobot;
            save["is_spider"] = p1->m_isSpider;
            
            save["is_upside_down"] = p1->m_isUpsideDown;
            
            // Читаем состояние мини-режима через публичное свойство
            save["is_mini"] = p1->m_isMini;

            Mod::get()->setSavedValue(getSaveKey(levelID), save);
            Mod::get()->saveData();
        }
    }

    void onExit() {
        if (m_fields->m_delegate) {
            m_fields->m_delegate->release();
        }
        PlayLayer::onExit();
    }
};
