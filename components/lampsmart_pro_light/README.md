# Componente LampSmart Pro Light

Componente customizado do ESPHome para controlar lâmpadas LampSmart Pro via Bluetooth Low Energy (BLE).

## Requisitos

- ESP32 (ESP8266 não suportado)
- ESPHome 2025.11.4+
- API com `custom_services: true` habilitado

## Configuração Mínima

```yaml
api:
  encryption:
    key: "SUA_CHAVE"
  custom_services: true  # OBRIGATÓRIO

external_components:
  - source: github://Joao-Dezan/esphome-lampsmart-pro

light:
  - platform: lampsmart_pro_light
    name: Kitchen Light
    duration: 1000
    default_transition_length: 0s
```

## Como Parear

1. **Desligue** a lâmpada completamente
2. **Ligue** a lâmpada novamente
3. **Dentro de 5 segundos**, execute o serviço no Home Assistant:
   - `esphome.<nome-dispositivo>_pair_<nome-luz>`
4. A lâmpada estará pareada!

## Problemas Conhecidos

* Apenas testado com lâmpadas Marpou Ceiling CCT
* Não suporta RGB atualmente (pode ser adicionado no futuro)

## Soluções para Problemas Comuns

### Temperaturas de cor invertidas
Se o branco quente aparece como frio e vice-versa:
```yaml
light:
  - platform: lampsmart_pro_light
    name: Minha Luz
    reversed: true
```

### Brilho mínimo muito alto
Ajuste o `min_brightness` (1-255 em hexadecimal):
```yaml
light:
  - platform: lampsmart_pro_light
    name: Minha Luz
    min_brightness: 0x3  # Valor mais baixo
```
