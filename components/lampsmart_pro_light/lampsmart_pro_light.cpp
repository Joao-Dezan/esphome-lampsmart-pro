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
          ESP_LOGI(TAG, "Enviando comando TURN_ON para a lampada...");
          send_packet(CMD_TURN_ON, 0, 0);
          ESP_LOGI(TAG, "Comando TURN_ON enviado");
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

        ESP_LOGI(TAG, "Enviando comando DIM: cw=%d, ww=%d", cwi, wwi);
        send_packet(CMD_DIM, cwi, wwi);
        ESP_LOGI(TAG, "Comando DIM enviado");
      } else {
        if (!_is_off.has_value() || _is_off.value() != state->current_values.is_on()) {
            ESP_LOGI(TAG, "Enviando comando TURN_OFF para a lampada...");
            send_packet(CMD_TURN_OFF, 0, 0);
            ESP_LOGI(TAG, "Comando TURN_OFF enviado");
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
      ESP_LOGI(TAG, "========================================");
      ESP_LOGI(TAG, "COMANDO DE PARAMENTO RECEBIDO!");
      ESP_LOGI(TAG, "LampSmartProLight::on_pair() chamado");
      
      char *hostId = getHostDeviceIdentifier();
      ESP_LOGI(TAG, "Host ID: [0x%02X, 0x%02X]", hostId[0], hostId[1]);
      ESP_LOGI(TAG, "Group ID: 0x%02X", group_id_);
      ESP_LOGI(TAG, "Enviando comando de pareamento via BLE...");
      
      send_packet(CMD_PAIR, hostId[0], hostId[1]);
      
      ESP_LOGI(TAG, "Comando de pareamento enviado com sucesso!");
      ESP_LOGI(TAG, "");
      ESP_LOGI(TAG, "*** PROCESSO DE PARAMENTO - SIGA ESTES PASSOS ***");
      ESP_LOGI(TAG, "");
      ESP_LOGI(TAG, "METODO 1 (Recomendado):");
      ESP_LOGI(TAG, "  1. A lampada DEVE estar DESLIGADA (interruptor fisico)");
      ESP_LOGI(TAG, "  2. Aguarde 10 segundos");
      ESP_LOGI(TAG, "  3. Execute este comando de pareamento (ja foi enviado)");
      ESP_LOGI(TAG, "  4. IMEDIATAMENTE (dentro de 5 segundos), LIGUE a lampada fisicamente");
      ESP_LOGI(TAG, "  5. A lampada deve piscar ou mudar de estado se o pareamento funcionou");
      ESP_LOGI(TAG, "");
      ESP_LOGI(TAG, "METODO 2 (Alternativo - se metodo 1 nao funcionar):");
      ESP_LOGI(TAG, "  1. LIGUE a lampada fisicamente");
      ESP_LOGI(TAG, "  2. Aguarde 2 segundos");
      ESP_LOGI(TAG, "  3. Execute o comando de pareamento");
      ESP_LOGI(TAG, "  4. A lampada deve responder se o pareamento funcionou");
      ESP_LOGI(TAG, "");
      ESP_LOGI(TAG, "VERIFICACAO:");
      ESP_LOGI(TAG, "  - Tente ligar/desligar a lampada pelo Home Assistant");
      ESP_LOGI(TAG, "  - Se funcionar, o pareamento foi bem-sucedido");
      ESP_LOGI(TAG, "  - Se nao funcionar, tente novamente com o outro metodo");
      ESP_LOGI(TAG, "");
      ESP_LOGI(TAG, "DICAS:");
      ESP_LOGI(TAG, "  - O ESP32 deve estar proximo da lampada (max 2-3 metros)");
      ESP_LOGI(TAG, "  - Algumas lampadas precisam de varias tentativas");
      ESP_LOGI(TAG, "  - Aguarde pelo menos 10 segundos entre tentativas");
      ESP_LOGI(TAG, "========================================");
    }

    void LampSmartProLight::on_unpair()
    {
      ESP_LOGI(TAG, "========================================");
      ESP_LOGI(TAG, "COMANDO DE DESPARAMENTO RECEBIDO!");
      ESP_LOGI(TAG, "LampSmartProLight::on_unpair() chamado");
      
      char *hostId = getHostDeviceIdentifier();
      ESP_LOGI(TAG, "Host ID: [0x%02X, 0x%02X]", hostId[0], hostId[1]);
      ESP_LOGI(TAG, "Group ID: 0x%02X", group_id_);
      ESP_LOGI(TAG, "Enviando comando de despareamento via BLE...");
      
      send_packet(CMD_UNPAIR, hostId[0], hostId[1]);
      
      ESP_LOGI(TAG, "Comando de despareamento enviado com sucesso!");
      ESP_LOGI(TAG, "========================================");
    }

    void LampSmartProLight::send_packet(uint16_t cmd, uint8_t arg1, uint8_t arg2)
    {
      char *hostId = getHostDeviceIdentifier();
      uint8_t *packet = (uint8_t *)buildPacket(cmd, hostId[0], hostId[1], arg1, arg2, group_id_);

      // Log do comando sendo enviado
      const char* cmd_name = "";
      if (cmd == CMD_PAIR) cmd_name = "PAIR";
      else if (cmd == CMD_UNPAIR) cmd_name = "UNPAIR";
      else if (cmd == CMD_TURN_ON) cmd_name = "TURN_ON";
      else if (cmd == CMD_TURN_OFF) cmd_name = "TURN_OFF";
      else if (cmd == CMD_DIM) cmd_name = "DIM";
      else cmd_name = "UNKNOWN";
      
      ESP_LOGI(TAG, "send_packet() - Comando: %s (0x%02X)", cmd_name, cmd);
      ESP_LOGI(TAG, "  Host ID: [0x%02X, 0x%02X]", hostId[0], hostId[1]);
      ESP_LOGI(TAG, "  Args: arg1=0x%02X, arg2=0x%02X, group=0x%02X", arg1, arg2, group_id_);
      
      // Log do pacote completo (primeiros 16 bytes para debug)
      ESP_LOGD(TAG, "Pacote BLE (primeiros 16 bytes):");
      char hex_str[64];
      for (int i = 0; i < 16 && i < 31; i++) {
        sprintf(hex_str + (i * 3), "%02X ", packet[i + 1]);
      }
      ESP_LOGD(TAG, "  %s", hex_str);
      
      ESP_LOGI(TAG, "Transmitindo pacote BLE por %d ms...", tx_duration_);

      // Skip first byte (BLE packet size indicator)
      esp_err_t err1 = esp_ble_gap_config_adv_data_raw(&packet[1], 31);
      if (err1 != ESP_OK) {
        ESP_LOGE(TAG, "ERRO ao configurar dados de advertising: 0x%X", err1);
      }
      
      esp_err_t err2 = esp_ble_gap_start_advertising(&ADVERTISING_PARAMS);
      if (err2 != ESP_OK) {
        ESP_LOGE(TAG, "ERRO ao iniciar advertising: 0x%X", err2);
      } else {
        ESP_LOGI(TAG, "Advertising iniciado com sucesso");
      }
      
      delay(tx_duration_);
      
      esp_err_t err3 = esp_ble_gap_stop_advertising();
      if (err3 != ESP_OK) {
        ESP_LOGE(TAG, "ERRO ao parar advertising: 0x%X", err3);
      } else {
        ESP_LOGI(TAG, "Advertising parado");
      }
      
      ESP_LOGI(TAG, "Pacote BLE transmitido com sucesso");
    }

  } // namespace lampsmartpro
} // namespace esphome

#endif
