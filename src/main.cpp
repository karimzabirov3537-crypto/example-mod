#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

std::string getSaveKey(int levelID) {
    return "platformer_save_level_" + std::to_string(levelID);
}

// Создаем отдельный класс-делегат для обработки нажатий кнопок в окне
class CheckpointLoadDelegate : public CCObject {
public:
    PlayLayer* m_layer;
    
    void onConfirm(CCObject* sender) {
        if (!m_layer) return;

        int levelID = m_layer->m_level->m_levelID;
        auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

        // В matjson для Geode v3 правильное приведение типов делается через .as<double>()
        float posX = static_cast<float>(savedData["player_x"].as<double>());
        float posY = static_cast<float>(savedData["player_y"].as<double>());
        
        if (m_layer->m_player1) {
            m_layer->m_player1->m_position = ccp(posX, posY);
        }

        m_layer->createCheckpoint(); 
        log::info("Успешно загрузили чекпоинт на позиции: {}, {}", posX, posY);
    }

    static CheckpointLoadDelegate* create(PlayLayer* layer) {
        auto ret = new CheckpointLoadDelegate();
        ret->m_layer = layer;
        ret->autorelease();
        return ret;
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    // ОФИЦИАЛЬНЫЙ СПОСОБ GEODE ДЛЯ ДЕКЛАРАЦИИ ПЕРЕМЕННЫХ
    struct Fields {
        bool m_hasCheckedLoad = false;
        CheckpointLoadDelegate* m_delegate = nullptr;
    };

    bool init(GJGameLevel* level, bool usePracticeMode, bool isSfxPreview) {
        if (!PlayLayer::init(level, usePracticeMode, isSfxPreview)) return false;
        
        m_fields->m_hasCheckedLoad = false;
        m_fields->m_delegate = CheckpointLoadDelegate::create(this);
        m_fields->m_delegate->retain(); // Удерживаем в памяти
        return true;
    }

    void showNewBest(bool p0, int p1, int p2, bool p3, bool p4, bool p5) {
        PlayLayer::showNewBest(p0, p1, p2, p3, p4, p5);

        if (!m_fields->m_hasCheckedLoad && this->m_level->isPlatformer()) {
            m_fields->m_hasCheckedLoad = true;
            
            int levelID = this->m_level->m_levelID;
            auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

            if (savedData.contains("has_save") && savedData["has_save"].as<bool>()) {
                
                // Исправленный вызов FLAlertLayer::create по правилам Geode API
                auto alert = FLAlertLayer::create(
                    m_fields->m_delegate,
                    "Загрузка",
                    "Найдено сохранение чекпоинта с прошлого сеанса. Хотите продолжить?",
                    "Отмена", "ОК",
                    300.f // Ширина окна
                );
                
                alert->show();
            }
        }
    }

    void onQuit() {
        if (this->m_level && this->m_level->isPlatformer() && this->m_checkpointArray && this->m_checkpointArray->count() > 0) {
            int levelID = this->m_level->m_levelID;
            
            matjson::Value save;
            save["has_save"] = true;
            
            if (this->m_player1) {
                save["player_x"] = this->m_player1->m_position.x;
                save["player_y"] = this->m_player1->m_position.y;
            }

            Mod::get()->setSavedValue(getSaveKey(levelID), save);
            Mod::get()->saveData();
        }

        // Освобождаем память делегата перед выходом
        if (m_fields->m_delegate) {
            m_fields->m_delegate->release();
        }

        PlayLayer::onQuit();
    }
};
