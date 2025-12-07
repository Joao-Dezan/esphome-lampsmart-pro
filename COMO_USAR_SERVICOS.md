# Como Usar os Serviços de Pareamento

## ⚠️ Problema Conhecido

Os serviços customizados do ESPHome podem não aparecer na lista de serviços do Home Assistant, **mas eles funcionam perfeitamente** se você usar o nome completo do serviço.

## ✅ Solução Rápida (Recomendada)

### Método 1: Via Developer Tools (Mais Rápido)

1. Vá em **Configurações** → **Ferramentas de Desenvolvimento** → **Serviços**
2. No campo **"Serviço"**, digite manualmente (não use a lista suspensa):
   ```
   esphome.sensor_teste_pair_bed_room_light
   ```
3. Clique em **"Executar"**
4. Pronto! O serviço será executado mesmo que não apareça na lista

**Para desparear:**
```
esphome.sensor_teste_unpair_bed_room_light
```

### Método 2: Criar um Script (Mais Conveniente)

Adicione em `configuration.yaml` ou `scripts.yaml`:

```yaml
script:
  parear_bed_room_light:
    alias: "Parear Bed Room Light"
    icon: mdi:lightbulb-on
    sequence:
      - service: esphome.sensor_teste_pair_bed_room_light
      - delay: 00:00:01
      - service: notify.mobile_app_seu_celular  # Opcional: notificação
        data:
          message: "Comando de pareamento enviado! Ligue a lâmpada agora."
  
  desparear_bed_room_light:
    alias: "Desparear Bed Room Light"
    icon: mdi:lightbulb-off
    sequence:
      - service: esphome.sensor_teste_unpair_bed_room_light
```

Depois de salvar e recarregar, você terá dois scripts disponíveis:
- `script.parear_bed_room_light`
- `script.desparear_bed_room_light`

Você pode chamar esses scripts de qualquer lugar: automações, dashboard, assistente de voz, etc.

## 📋 Nomes dos Serviços

Para seu dispositivo `sensor-teste`:

| Entidade | Serviço de Pareamento | Serviço de Despareamento |
|----------|----------------------|-------------------------|
| Bed Room Light | `esphome.sensor_teste_pair_bed_room_light` | `esphome.sensor_teste_unpair_bed_room_light` |
| Bed Room Fan | `esphome.sensor_teste_pair_bed_room_fan` | `esphome.sensor_teste_unpair_bed_room_fan` |

**Formato geral:**
- Parear: `esphome.<nome-dispositivo>_pair_<object-id>`
- Desparear: `esphome.<nome-dispositivo>_unpair_<object-id>`

Onde:
- `<nome-dispositivo>` = `sensor_teste` (espaços viram `_`, tudo minúsculo)
- `<object-id>` = `bed_room_light` ou `bed_room_fan` (espaços viram `_`, tudo minúsculo)

## 🔧 Exemplo Completo de Automação

```yaml
automation:
  - alias: "Parear Lâmpada ao Ligar Interruptor"
    description: "Pareia a lâmpada quando um interruptor é ligado"
    trigger:
      - platform: state
        entity_id: input_boolean.parear_lampada
        to: 'on'
    condition:
      - condition: state
        entity_id: light.bed_room_light
        state: 'off'
    action:
      - service: esphome.sensor_teste_pair_bed_room_light
      - delay: 00:00:01
      - service: input_boolean.turn_off
        entity_id: input_boolean.parear_lampada
      - service: notify.mobile_app_seu_celular
        data:
          title: "Pareamento"
          message: "Comando enviado! Ligue a lâmpada agora (dentro de 5 segundos)."
```

## 🎯 Processo Completo de Pareamento

1. **Crie o script** (método 2 acima) ou use o Developer Tools (método 1)
2. **Desligue a lâmpada** completamente
3. **Execute o serviço/script** de pareamento
4. **IMEDIATAMENTE** (dentro de 5 segundos), **ligue a lâmpada**
5. A lâmpada estará pareada e pronta para uso!

## ❓ Verificação

Para verificar se os serviços estão funcionando, execute o serviço e verifique os logs do ESPHome. Você deve ver:
```
[I][lampsmartpro: ] LampSmartProLight::on_pair called!
```

Isso confirma que o serviço foi executado com sucesso.

