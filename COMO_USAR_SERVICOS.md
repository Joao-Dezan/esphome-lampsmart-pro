# Como Usar os Serviços de Pareamento

## 📌 Formato Atual dos Serviços Customizados do ESPHome

Os serviços customizados registrados via `CustomAPIDevice::register_service()` no ESPHome seguem o formato:

```
esphome.<nome-dispositivo>_<nome-servico>
```

Onde:
- `<nome-dispositivo>` = nome do dispositivo ESPHome (espaços viram `_`, tudo minúsculo)
- `<nome-servico>` = nome do serviço registrado (ex: `pair_bed_room_light`)

**Exemplo:** Para um dispositivo chamado `sensor-teste` com serviço `pair_bed_room_light`:
```
esphome.sensor_teste_pair_bed_room_light
```

## ⚠️ Problema Conhecido

Os serviços customizados do ESPHome podem não aparecer na lista suspensa de serviços do Home Assistant, **mas eles funcionam perfeitamente** se você usar o nome completo do serviço manualmente.

## ✅ Solução Rápida (Recomendada)

### Método 1: Via Developer Tools (Mais Rápido) - RECOMENDADO

1. Vá em **Configurações** → **Ferramentas de Desenvolvimento** → **Serviços**
2. No campo **"Serviço"**, digite manualmente o nome completo (não use a lista suspensa):
   ```
   esphome.sensor_teste_pair_bed_room_light
   ```
   **Importante:** Digite o nome completo, não selecione da lista suspensa, pois os serviços customizados podem não aparecer lá.
3. Clique em **"Executar"**
4. Pronto! O serviço será executado mesmo que não apareça na lista

**Dica:** Você pode verificar se o serviço existe usando o botão "Escolher entidade" ao lado do campo de serviço, mas geralmente é mais rápido digitar diretamente.

**Para desparear:**
```
esphome.sensor_teste_unpair_bed_room_light
```

### Método 2: Criar um Script (Mais Conveniente)

Adicione em `configuration.yaml` ou `scripts.yaml`:

**Formato correto para scripts.yaml:**

```yaml
parear_bed_room_light:
  alias: "Parear Bed Room Light"
  icon: mdi:lightbulb-on
  sequence:
    - service: esphome.sensor_teste_pair_bed_room_light
    - delay:
        seconds: 1
    # Opcional: notificação
    # - service: notify.mobile_app_seu_celular
    #   data:
    #     message: "Comando de pareamento enviado! Ligue a lâmpada agora."

desparear_bed_room_light:
  alias: "Desparear Bed Room Light"
  icon: mdi:lightbulb-off
  sequence:
    - service: esphome.sensor_teste_unpair_bed_room_light
```

**OU se estiver usando configuration.yaml (com a chave `script:`):**

```yaml
script:
  parear_bed_room_light:
    alias: "Parear Bed Room Light"
    icon: mdi:lightbulb-on
    sequence:
      - service: esphome.sensor_teste_pair_bed_room_light
      - delay:
          seconds: 1

  desparear_bed_room_light:
    alias: "Desparear Bed Room Light"
    icon: mdi:lightbulb-off
    sequence:
      - service: esphome.sensor_teste_unpair_bed_room_light
```

**Importante:** 
- Se usar `scripts.yaml`, **NÃO** inclua a chave `script:` no início
- Se usar `configuration.yaml`, **SIM**, inclua a chave `script:` no início
- O formato do `delay` mudou: use `delay: seconds: 1` em vez de `delay: 00:00:01`

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

Para verificar se os serviços estão funcionando:

1. **Execute o serviço** no Home Assistant (Developer Tools → Serviços)
2. **Verifique os logs do ESPHome** - você deve ver logs detalhados como:

   **Ao executar o serviço de pareamento:**
   ```
   [I][lampsmartpro: ] ========================================
   [I][lampsmartpro: ] COMANDO DE PARAMENTO RECEBIDO!
   [I][lampsmartpro: ] LampSmartProLight::on_pair() chamado
   [I][lampsmartpro: ] Host ID: [0xXX, 0xXX]
   [I][lampsmartpro: ] Group ID: 0xXX
   [I][lampsmartpro: ] Enviando comando de pareamento via BLE...
   [I][lampsmartpro: ] Comando de pareamento enviado com sucesso!
   [I][lampsmartpro: ] Aguarde 5 segundos e ligue a lampada para completar o pareamento
   [I][lampsmartpro: ] ========================================
   ```

   **Ao executar o serviço de despareamento:**
   ```
   [I][lampsmartpro: ] ========================================
   [I][lampsmartpro: ] COMANDO DE DESPARAMENTO RECEBIDO!
   [I][lampsmartpro: ] LampSmartProLight::on_unpair() chamado
   [I][lampsmartpro: ] Host ID: [0xXX, 0xXX]
   [I][lampsmartpro: ] Group ID: 0xXX
   [I][lampsmartpro: ] Enviando comando de despareamento via BLE...
   [I][lampsmartpro: ] Comando de despareamento enviado com sucesso!
   [I][lampsmartpro: ] ========================================
   ```

3. **Verifique os logs de inicialização** - ao iniciar o dispositivo, você deve ver:
   ```
   [I][lampsmartpro: ] Servicos ja registrados! Object_id usado: bed_room_light
   [I][lampsmartpro: ] Servicos esperados: pair_bed_room_light e unpair_bed_room_light
   ```

**Nota:** Os logs mostram quando o comando é **enviado** pelo ESP32. O pareamento é bem-sucedido quando você consegue controlar a lâmpada após executar o comando e ligar a lâmpada dentro de 5 segundos.

## 🔍 Troubleshooting

### Erro ao adicionar scripts no scripts.yaml

**Erro:** `extra keys not allowed` ou `required key not provided @ data['sequence']`

**Causa:** Formato incorreto do YAML ou uso incorreto da chave `script:`

**Solução:**
1. Se usar `scripts.yaml` (arquivo separado), **NÃO** inclua a chave `script:` no início:
   ```yaml
   # CORRETO para scripts.yaml
   parear_bed_room_light:
     alias: "Parear Bed Room Light"
     sequence:
       - service: esphome.sensor_teste_pair_bed_room_light
   ```

2. Se usar `configuration.yaml`, **SIM**, inclua a chave `script:`:
   ```yaml
   # CORRETO para configuration.yaml
   script:
     parear_bed_room_light:
       alias: "Parear Bed Room Light"
       sequence:
         - service: esphome.sensor_teste_pair_bed_room_light
   ```

3. Use o formato correto do `delay`:
   ```yaml
   # CORRETO
   - delay:
       seconds: 1
   
   # ERRADO (formato antigo)
   - delay: 00:00:01
   ```

### Os serviços não aparecem na lista do Home Assistant

**Isso é normal!** Os serviços customizados do ESPHome podem não aparecer na lista suspensa, mas funcionam quando chamados pelo nome completo.

**Solução:** Use o nome completo do serviço manualmente no Developer Tools.

### Como descobrir o nome exato do serviço

1. Verifique os logs do ESPHome na inicialização
2. Procure por: `Servicos esperados: pair_<object-id> e unpair_<object-id>`
3. O nome completo será: `esphome.<nome-dispositivo>_pair_<object-id>`

### O serviço não executa

1. Verifique se `custom_services: true` está configurado na API do ESPHome
2. Recompile e faça upload do firmware (não basta reiniciar)
3. Reinicie o Home Assistant ou remova e adicione novamente o dispositivo ESPHome
4. Verifique se está usando o nome completo correto do serviço

### Os comandos são enviados mas a lâmpada não responde

**Sintoma:** Os logs mostram que os comandos estão sendo enviados (TURN_ON, TURN_OFF, DIM), mas a lâmpada não reage.

**Causa:** O pareamento não foi bem-sucedido ou a lâmpada não está em modo de pareamento.

**Solução passo a passo:**

1. **Desligue a lâmpada completamente** (interruptor físico ou disjuntor)
2. **Aguarde pelo menos 10 segundos** (importante para a lâmpada resetar)
3. **Execute o serviço de pareamento** no Home Assistant:
   ```
   esphome.sensor_teste_pair_bed_room_light
   ```
4. **IMEDIATAMENTE** (dentro de 5 segundos), **ligue a lâmpada fisicamente**
5. **Observe a lâmpada:**
   - Se ela piscar ou mudar de estado, o pareamento pode ter funcionado
   - Se não acontecer nada, repita o processo

**Dicas importantes:**
- ⚠️ **Tempo é crítico:** Você tem apenas 5 segundos após executar o comando para ligar a lâmpada
- 📍 **Distância:** O ESP32 deve estar próximo à lâmpada (máximo 2-3 metros)
- 🔄 **Tente várias vezes:** Algumas lâmpadas precisam de várias tentativas
- 🔌 **Desligue completamente:** Use o interruptor físico ou disjuntor, não apenas pelo Home Assistant
- ⏱️ **Aguarde entre tentativas:** Aguarde pelo menos 10 segundos entre tentativas

**Verificação:**
- Após o pareamento, tente ligar/desligar a lâmpada pelo Home Assistant
- Se funcionar, o pareamento foi bem-sucedido
- Se não funcionar, repita o processo de pareamento

**Logs esperados quando funcionar:**
```
[I][lampsmartpro: ] Enviando comando TURN_ON para a lampada...
[I][lampsmartpro: ] Comando TURN_ON enviado
```

Se você ver esses logs mas a lâmpada não responder, o problema é o pareamento, não o envio dos comandos.

