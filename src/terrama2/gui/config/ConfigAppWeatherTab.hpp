#ifndef __TERRAMA2_GUI_CONFIG_CONFIGAPPWEATHERTAB_HPP__
#define __TERRAMA2_GUI_CONFIG_CONFIGAPPWEATHERTAB_HPP__

// TerraMA2
#include "ConfigAppTab.hpp"

// QT
#include <QString>

class ConfigAppWeatherTab : public ConfigAppTab
{
  Q_OBJECT

  public:
    ConfigAppWeatherTab(ConfigApp* app, Ui::ConfigAppForm* ui, terrama2::core::ApplicationController* controller);
    ~ConfigAppWeatherTab();

    void load();
    bool dataChanged();
    bool validate();
    void save();
    void discardChanges(bool restore_data);

  signals:
    void serverChanged();

    //! It could be used in server operations is inserted or server is cancelled
    void serverDone();

  private slots:
    void onEnteredWeatherTab();
    void onWeatherTabEdited();
    void onImportServer();
    void onCheckConnection();

};

#endif
