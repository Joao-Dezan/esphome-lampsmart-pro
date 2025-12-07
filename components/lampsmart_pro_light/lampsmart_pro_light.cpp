#include "lampsmart_utils.h"
#include "lampsmart_pro_light.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32

#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#ifdef USE_ESP32_FRAMEWORK_ARDUINO
#include <esp_wifi.h>
#endif

namespace esphome
{
  namespace lampsmartpro
  {

    void LampSmartProLight::setup()
    {
      ESP_LOGI(TAG, "=== LampSmartProLight::setup() chamado ===");
      std::string object_id = this->get_object_id();
      ESP_LOGI(TAG, "setup() - object_id: '%s' (vazio: %s)", object_id.c_str(), object_id.empty() ? "SIM" : "NAO");
      
      // Tenta registrar no setup(), mas se o object_id não estiver disponível, 
      // registraremos no setup_state() ou dump_config()
      if (!object_id.empty()) {
        std::string pair_service = "pair_" + object_id;
        std::string unpair_service = "unpair_" + object_id;
        ESP_LOGI(TAG, "Registrando servicos no setup():");
        ESP_LOGI(TAG, "  - pair: %s", pair_service.c_str());
        ESP_LOGI(TAG, "  - unpair: %s", unpair_service.c_str());
        register_service(&LampSmartProLight::on_pair, pair_service);
        register_service(&LampSmartProLight::on_unpair, unpair_service);
        services_registered_ = true;
        ESP_LOGI(TAG, "Servicos customizados registrados no setup() com SUCESSO!");
      } else {
        ESP_LOGI(TAG, "object_id ainda não disponível no setup(), tentando em setup_state() ou dump_config()");
      }
    }

    void LampSmartProLight::setup_state(light::LightState *state)
    {
      ESP_LOGI(TAG, "=== LampSmartProLight::setup_state() chamado ===");
      this->light_state_ = state;
      ESP_LOGI(TAG, "setup_state() - state: %s", state ? "NOT NULL" : "NULL");
      
      // Registra os serviços se ainda não foram registrados
      if (!services_registered_ && state) {
        std::string object_id = state->get_object_id();
        std::string pair_service = "pair_" + object_id;
        std::string unpair_service = "unpair_" + object_id;
        ESP_LOGI(TAG, "Registrando servicos no setup_state():");
        ESP_LOGI(TAG, "  - object_id: %s", object_id.c_str());
        ESP_LOGI(TAG, "  - pair: %s", pair_service.c_str());
        ESP_LOGI(TAG, "  - unpair: %s", unpair_service.c_str());
        register_service(&LampSmartProLight::on_pair, pair_service);
        register_service(&LampSmartProLight::on_unpair, unpair_service);
        services_registered_ = true;
        ESP_LOGI(TAG, "Servicos customizados registrados no setup_state() com SUCESSO!");
      } else if (services_registered_) {
        ESP_LOGI(TAG, "setup_state() - Servicos ja registrados anteriormente (em setup())");
      } else {
        ESP_LOGE(TAG, "setup_state() chamado com state NULL!");
      }
    }

    light::LightTraits LampSmartProLight::get_traits()
    {
      auto traits = light::LightTraits();
      traits.set_supported_color_modes({light::ColorMode::COLD_WARM_WHITE});
      traits.set_min_mireds(this->cold_white_temperature_);
      traits.set_max_mireds(this->warm_white_temperature_);
      return traits;
    }

    char *LampSmartProLight::getHostDeviceIdentifier()
    {
      #ifdef USE_ESP32_FRAMEWORK_ARDUINO
      uint8_t hash[6];
      esp_wifi_get_mac(WIFI_IF_STA, hash);
      int crc = CRC16((char *)&hash, 6, 0);
      #else
      uint32_t hash = light_state_ ? light_state_->get_object_id_hash() : 0xcafebabe;
      int crc = CRC16((char *)&hash, 4, 0);
      #endif

      static char hostId[2];
      hostId[0] = (crc >> 8) & 255;
      hostId[1] = crc & 255;
      ESP_LOGD(TAG, "LampSmartProLight::getHostDeviceIdentifier called!hostId[0]: %02x, hostId[1]: %02x", hostId[0], hostId[1]);
      return hostId;
    }

    void LampSmartProLight::write_state(light::LightState *state)
    {
      float cwf, wwf;
      state->current_values_as_cwww(&cwf, &wwf, this->constant_brightness_);

      ESP_LOGD(TAG, "write state: remote: %s, current: %s", ONOFF(state->remote_values.is_on()), ONOFF(state->current_values.is_on()));
      if (state->current_values.is_on()) {
        if (!_is_off.has_value() || _is_off.value() != state->current_values.is_on()) {
          send_packet(CMD_TURN_ON, 0, 0);
          ESP_LOGD(TAG, "write state: send turn on");
        }
        _is_off = state->current_values.is_on();
        uint8_t cwi = (uint8_t)(0xff * cwf);
        uint8_t wwi = (uint8_t)(0xff * wwf);

        if ((cwi < min_brightness_) && (wwi < min_brightness_))
        {
          if (cwf > 0.000001)
          {
            cwi = min_brightness_;
          }

          if (wwf > 0.000001)
          {
            wwi = min_brightness_;
          }
        }

        ESP_LOGD(TAG, "LampSmartProLight::write_state called! Requested cw: %d, ww: %d", cwi, wwi);

        send_packet(CMD_DIM, cwi, wwi);
      } else {
        if (!_is_off.has_value() || _is_off.value() != state->current_values.is_on()) {
            send_packet(CMD_TURN_OFF, 0, 0);
            ESP_LOGD(TAG, "write state: send turn off");
        }
        _is_off = state->current_values.is_on();
        return;
      }
    }

    void LampSmartProLight::dump_config()
    {
      ESP_LOGI(TAG, "LampSmartProLight::dump_config() chamado");
      ESP_LOGCONFIG(TAG, "LampSmartProLight '%s'", light_state_ ? light_state_->get_name().c_str() : "");
      ESP_LOGCONFIG(TAG, "  Cold White Temperature: %f mireds", cold_white_temperature_);
      ESP_LOGCONFIG(TAG, "  Warm White Temperature: %f mireds", warm_white_temperature_);
      ESP_LOGCONFIG(TAG, "  Constant Brightness: %s", constant_brightness_ ? "true" : "false");
      ESP_LOGCONFIG(TAG, "  Minimum Brightness: %d", min_brightness_);
      ESP_LOGCONFIG(TAG, "  Transmission Duration: %d millis", tx_duration_);
      
      // Tenta registrar os serviços aqui também, caso setup() e setup_state() não sejam chamados
      ESP_LOGI(TAG, "dump_config() - services_registered_: %s", services_registered_ ? "true" : "false");
      ESP_LOGI(TAG, "dump_config() - light_state_: %s", light_state_ ? "NOT NULL" : "NULL");
      
      // Se os serviços já foram registrados, mostra qual object_id foi usado
      if (services_registered_ && light_state_) {
        std::string object_id = light_state_->get_object_id();
        ESP_LOGI(TAG, "Servicos ja registrados! Object_id usado: %s", object_id.c_str());
        ESP_LOGI(TAG, "Servicos esperados: pair_%s e unpair_%s", object_id.c_str(), object_id.c_str());
      }
      
      if (!services_registered_) {
        if (light_state_) {
          std::string object_id = light_state_->get_object_id();
          ESP_LOGI(TAG, "Tentando registrar servicos no dump_config(), object_id do LightState: %s", object_id.c_str());
          register_service(&LampSmartProLight::on_pair, "pair_" + object_id);
          register_service(&LampSmartProLight::on_unpair, "unpair_" + object_id);
          services_registered_ = true;
          ESP_LOGI(TAG, "Servicos customizados registrados no dump_config() com object_id do LightState!");
        } else {
          std::string object_id = this->get_object_id();
          ESP_LOGI(TAG, "dump_config() - object_id do componente: '%s' (vazio: %s)", object_id.c_str(), object_id.empty() ? "SIM" : "NAO");
          if (!object_id.empty()) {
            ESP_LOGI(TAG, "Tentando registrar servicos no dump_config() com object_id do componente: %s", object_id.c_str());
            register_service(&LampSmartProLight::on_pair, "pair_" + object_id);
            register_service(&LampSmartProLight::on_unpair, "unpair_" + object_id);
            services_registered_ = true;
            ESP_LOGI(TAG, "Servicos customizados registrados no dump_config() com object_id do componente!");
          } else {
            ESP_LOGE(TAG, "Nao foi possivel registrar servicos: light_state_ NULL e object_id vazio!");
          }
        }
      } else {
        ESP_LOGI(TAG, "Servicos ja foram registrados anteriormente, pulando registro no dump_config()");
      }
    }

    void LampSmartProLight::on_pair()
    {
      ESP_LOGD(TAG, "LampSmartProLight::on_pair called!");

      char *hostId = getHostDeviceIdentifier();
      send_packet(CMD_PAIR, hostId[0], hostId[1]);
    }

    void LampSmartProLight::on_unpair()
    {
      ESP_LOGD(TAG, "LampSmartProLight::on_unpair called!");

      char *hostId = getHostDeviceIdentifier();
      send_packet(CMD_UNPAIR, hostId[0], hostId[1]);
    }

    void LampSmartProLight::send_packet(uint16_t cmd, uint8_t arg1, uint8_t arg2)
    {
      char *hostId = getHostDeviceIdentifier();
      uint8_t *packet = (uint8_t *)buildPacket(cmd, hostId[0], hostId[1], arg1, arg2, group_id_);

      // Skip first byte (BLE packet size indicator)
      ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gap_config_adv_data_raw(&packet[1], 31));
      ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gap_start_advertising(&ADVERTISING_PARAMS));
      delay(tx_duration_);
      ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gap_stop_advertising());
    }

  } // namespace lampsmartpro
} // namespace esphome

#endif
