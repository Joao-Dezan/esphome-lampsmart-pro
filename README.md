# esphome-lampsmart-pro
Custom ESPHome component to interface with Chinese "LampSmart Pro" Bluetooth lights.

Basically it's https://github.com/aronsky/esphome-components and https://github.com/MasterDevX/lampify combined to work with more (or different?) lights.

## Requisitos

- ESP32 (ESP8266 não é suportado, pois requer BLE)
- ESPHome 2025.11.4 ou superior
- Home Assistant (opcional, mas recomendado)

## Instalação

1. Adicione o componente como `external_component` no seu arquivo de configuração do ESPHome
2. Configure a API com `custom_services: true` (obrigatório para os serviços de pareamento)
3. Adicione as entidades de luz e/ou ventilador
4. Compile e faça o flash no dispositivo ESP32

## Configuração Completa

```yaml
esphome:
  name: sensor-teste
  platform: ESP32
  board: esp32dev

# API com custom_services habilitado (OBRIGATÓRIO para pareamento)
api:
  encryption:
    key: "SUA_CHAVE_DE_CRIPTOGRAFIA_AQUI"
  custom_services: true

wifi:
  ssid: "SEU_WIFI"
  password: "SUA_SENHA"

# Componente externo
external_components:
  - source: github://Joao-Dezan/esphome-lampsmart-pro
    # ou use: github://zt8989/esphome-lampsmart-pro

# Configuração da luz
light:
  - platform: lampsmart_pro_light
    name: Bed Room Light
    group: 1  # Use o mesmo group para luz e ventilador do mesmo dispositivo
    default_transition_length: 0s
    duration: 500  # Duração da transmissão BLE em milissegundos

# Configuração do ventilador (opcional)
fan:
  - platform: lampsmart_pro_light
    name: Bed Room Fan
    group: 1  # Mesmo group da luz
    duration: 500
```

## Como Parear uma Lâmpada

O pareamento é necessário para que o ESP32 possa controlar a lâmpada via Bluetooth. Siga estes passos:

### ⚠️ Antes de Começar - Verifique se os Serviços Estão Disponíveis

Antes de tentar parear, verifique se os serviços de pareamento estão disponíveis no Home Assistant:

1. Vá em **Configurações** → **Automações e Cenas** → **Serviços**
2. Digite `esphome` na busca
3. Você deve ver serviços como:
   - `esphome.<seu-dispositivo>_pair_<nome-da-luz>`
   - `esphome.<seu-dispositivo>_unpair_<nome-da-luz>`

**Se os serviços NÃO aparecerem:**
- Verifique se `custom_services: true` está configurado na API
- **Recompile e faça flash novamente** no dispositivo ESP32 (apenas reiniciar não é suficiente)
- Reinicie o Home Assistant ou remova e adicione novamente o dispositivo ESPHome
- Veja a seção "Solução de Problemas" abaixo para mais detalhes

### Passo 1: Preparar a Lâmpada
1. **Desligue a lâmpada** completamente (interruptor físico ou disjuntor)
2. Aguarde alguns segundos

### Passo 2: Iniciar o Pareamento
1. **Ligue a lâmpada** novamente
2. **IMEDIATAMENTE** (dentro de 5 segundos), execute o serviço de pareamento no Home Assistant:

   **Opção A - Via Interface:**
   - Vá em **Configurações** → **Automações e Cenas** → **Serviços**
   - Digite `esphome` na busca
   - Procure pelo serviço: `esphome.<nome-do-dispositivo>_pair_<nome-da-luz>`
   - Exemplo: `esphome.sensor_teste_pair_bed_room_light`
   - Clique em **Executar**

   **Opção B - Via Developer Tools:**
   - Vá em **Configurações** → **Dispositivos e Serviços**
   - Clique em **ESPHome** → Selecione seu dispositivo
   - Vá na aba **Serviços** (se disponível)
   - Execute o serviço de pareamento

   **Opção C - Via YAML (Automação/Script):**
   ```yaml
   service: esphome.sensor_teste_pair_bed_room_light
   ```

   **⚠️ IMPORTANTE:** Se os serviços não aparecem, veja a seção "Solução de Problemas" abaixo.

### Passo 3: Verificar o Pareamento
- Se o pareamento foi bem-sucedido, você poderá controlar a lâmpada normalmente
- Se não funcionar, repita o processo (desligue, aguarde, ligue e execute o serviço rapidamente)

### Dicas Importantes
- ⚠️ **Tempo é crítico**: Você tem apenas **5 segundos** após ligar a lâmpada para executar o serviço de pareamento
- 🔄 Se a lâmpada e o ventilador estão no mesmo dispositivo físico, você só precisa parear uma vez (use o mesmo `group`)
- 🔌 Se mudar o ESP32 ou reinstalar, será necessário parear novamente
- 📱 O pareamento é feito via Bluetooth Low Energy (BLE), então o ESP32 precisa estar próximo à lâmpada

## Desparear uma Lâmpada

Para desparear uma lâmpada (útil se quiser parear com outro dispositivo):

1. Execute o serviço de despareamento:
   ```yaml
   service: esphome.<nome-do-dispositivo>_unpair_<nome-da-luz>
   ```

## Opções de Configuração

### Light (Luz)

| Opção | Tipo | Padrão | Descrição |
|-------|------|--------|-----------|
| `name` | string | **obrigatório** | Nome da entidade |
| `group` | hex (0x0-0xF) | `0x0` | ID do grupo (use o mesmo para luz e ventilador do mesmo dispositivo) |
| `duration` | int | `100` | Duração da transmissão BLE em milissegundos |
| `default_transition_length` | time | `0s` | Duração padrão das transições |
| `cold_white_color_temperature` | mireds | - | Temperatura de cor do branco frio |
| `warm_white_color_temperature` | mireds | - | Temperatura de cor do branco quente |
| `constant_brightness` | boolean | `false` | Mantém brilho constante ao mudar temperatura |
| `reversed` | boolean | `false` | Inverte branco frio/quente |
| `min_brightness` | hex (0x1-0xFF) | `0x7` | Brilho mínimo (1-255) |

### Fan (Ventilador)

| Opção | Tipo | Padrão | Descrição |
|-------|------|--------|-----------|
| `name` | string | **obrigatório** | Nome da entidade |
| `group` | hex (0x0-0xF) | `0x0` | ID do grupo (use o mesmo para luz e ventilador do mesmo dispositivo) |
| `duration` | int | `100` | Duração da transmissão BLE em milissegundos |

## Solução de Problemas

### Serviços de pareamento não aparecem no Home Assistant

Se os serviços `pair_` e `unpair_` não aparecem no Home Assistant, siga estes passos:

1. **Verifique a configuração da API**:
   ```yaml
   api:
     encryption:
       key: "SUA_CHAVE"
     custom_services: true  # ← DEVE estar como true
   ```

2. **Reinicie o dispositivo ESP32**:
   - Após adicionar ou modificar `custom_services: true`, você **DEVE** recompilar e fazer flash novamente no dispositivo
   - Apenas reiniciar não é suficiente - é necessário recompilar

3. **Reinicie o Home Assistant**:
   - Após recompilar o ESP32, reinicie o Home Assistant para que ele detecte os novos serviços
   - Ou remova e adicione novamente o dispositivo ESPHome no Home Assistant

4. **Verifique os logs do ESPHome**:
   - No dashboard do ESPHome, verifique os logs para ver se há erros relacionados a `register_service`
   - Os serviços devem ser registrados durante o `setup()` do componente

5. **Verifique o nome do serviço**:
   - O nome do serviço segue o padrão: `esphome.<nome-dispositivo>_pair_<nome-entidade>`
   - Exemplo: Se o dispositivo é `sensor-teste` e a luz é `Bed Room Light`, o serviço será:
     - `esphome.sensor_teste_pair_bed_room_light`
   - Note que espaços são substituídos por underscores e tudo fica em minúsculas

6. **Procure os serviços no Developer Tools**:
   - Vá em **Configurações** → **Automações e Cenas** → **Serviços**
   - Digite `esphome` na busca
   - Você deve ver todos os serviços disponíveis, incluindo os de pareamento

### Lâmpada não responde após pareamento
- Verifique se o ESP32 está próximo à lâmpada (BLE tem alcance limitado)
- Tente desparear e parear novamente
- Verifique se a lâmpada está ligada

### Temperaturas de cor invertidas
Adicione `reversed: true` na configuração da luz:
```yaml
light:
  - platform: lampsmart_pro_light
    name: Minha Luz
    reversed: true
```

### Brilho mínimo muito alto
Ajuste o `min_brightness` (valores entre 1-255, em hexadecimal):
```yaml
light:
  - platform: lampsmart_pro_light
    name: Minha Luz
    min_brightness: 0x3  # Brilho mínimo mais baixo
```

### Erro de compilação sobre `custom_services`
Certifique-se de que a API está configurada com `custom_services: true`:
```yaml
api:
  encryption:
    key: "SUA_CHAVE"
  custom_services: true  # ← OBRIGATÓRIO
```

## Compatibilidade

- ✅ Testado com lâmpadas Marpou Ceiling CCT
- ✅ Suporta luzes CCT (Cold/Warm White)
- ❌ Não suporta RGB atualmente (pode ser adicionado no futuro)
