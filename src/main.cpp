#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/utils/JsonValidation.hpp>

using namespace geode::prelude;

// Ключ для хранения данных в локальном сейве мода
std::string getSaveKey(int levelID) {
    return "platformer_save_level_" + std::to_string(levelID);
}

class $modify(MyPlayLayer, PlayLayer) {
    // Переменная, чтобы избежать бесконечного спавна окон
    bool m_fields_hasCheckedLoad = false;

    bool init(GJGameLevel* level, bool usePracticeMode, bool isSfxPreview) {
        if (!PlayLayer::init(level, usePracticeMode, isSfxPreview)) return false;
        
        this->m_fields_hasCheckedLoad = false;
        return true;
    }

    void showNewBest(bool p0, int p1, int p2, bool p3, bool p4, bool p5) {
        PlayLayer::showNewBest(p0, p1, p2, p3, p4, p5);

        // Проверяем сохранение при старте (метод вызывается близко к началу геймплея)
        if (!this->m_fields_hasCheckedLoad && this->m_level->isPlatformer()) {
            this->m_fields_hasCheckedLoad = true;
            
            int levelID = this->m_level->m_levelID;
            auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

            // Если сохранение существует и там есть чекпоинты
            if (savedData.contains("has_save") && savedData["has_save"].asBool()) {
                
                // Ставим игру на паузу, пока игрок думает
                this->m_isPaused = true;

                // Создаем красивое встроенное окно
                auto alert = FLAlertLayer::create(
                    "Загрузка",
                    "Найдено сохранение чекпоинта с прошлого сеанса. Хотите <cg>продолжить</c>?",
                    "Отмена", "ОК"
                );
                
                // Перехватываем нажатие кнопок через лямбду (Geode API)
                alert->m_button2Target = this;
                alert->m_button2Selector = menu_selector(MyPlayLayer::onConfirmLoadCheckpoint);
                
                alert->show();
            }
        }
    }

    // Обработчик кнопки "ОК" на загрузку
    void onConfirmLoadCheckpoint(CCObject* sender) {
        this->m_isPaused = false;
        int levelID = this->m_level->m_levelID;
        auto savedData = Mod::get()->getSavedValue<matjson::Value>(getSaveKey(levelID));

        // Восстанавливаем позицию игрока
        float posX = static_cast<float>(savedData["player_x"].asDouble());
        float posY = static_cast<float>(savedData["player_y"].asDouble());
        
        if (this->m_player1) {
            this->m_player1->m_position = ccp(posX, posY);
        }

        // Симулируем создание виртуального чекпоинта, чтобы игра знала, куда респавнить
        this->createCheckpoint(); 
        
        log::info("Успешно загрузили чекпоинт на позиции: {}, {}", posX, posY);
    }

    // Хук на выход из уровня
    void onQuit() {
        // Проверяем, что это платформер и у игрока есть хотя бы один чекпоинт
        if (this->m_level && this->m_level->isPlatformer() && this->m_checkpointArray && this->m_checkpointArray->count() > 0) {
            int levelID = this->m_level->m_levelID;
            
            matjson::Value save;
            save["has_save"] = true;
            
            // Сохраняем последние координаты игрока
            if (this->m_player1) {
                save["player_x"] = this->m_player1->m_position.x;
                save["player_y"] = this->m_player1->m_position.y;
            }

            // Записываем данные в постоянную память мода
            Mod::get()->setSavedValue(getSaveKey(levelID), save);
            Mod::get()->saveData();
            
            log::info("Сохранили платформер чекпоинт для уровня {}", levelID);
        }

        // Вызываем оригинальный выход
        PlayLayer::onQuit();
    }
};
