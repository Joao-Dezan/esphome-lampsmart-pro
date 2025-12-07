# Troubleshooting - Problemas com Pareamento

## 🔍 Diagnóstico

Se os comandos estão sendo enviados mas a lâmpada não responde, siga este guia de troubleshooting.

## ✅ Verificações Básicas

### 1. Os comandos estão sendo enviados?

**Verifique os logs do ESPHome:**
```
[I][lampsmartpro: ] Enviando comando TURN_ON para a lampada...
[I][lampsmartpro: ] Comando TURN_ON enviado
```

Se você vê esses logs, os comandos estão sendo enviados corretamente.

### 2. O pareamento foi bem-sucedido?

**Sintoma:** Comandos são enviados mas a lâmpada não responde.

**Causa:** O pareamento não foi concluído ou a lâmpada não está em modo de pareamento.

## 🔧 Métodos de Pareamento

### Método 1: Pareamento com Lâmpada Desligada (Recomendado)

1. **Desligue a lâmpada completamente** (interruptor físico ou disjuntor)
2. **Aguarde pelo menos 10 segundos** (importante para resetar)
3. **Execute o serviço de pareamento** no Home Assistant:
   ```
   esphome.sensor_teste_pair_bed_room_light
   ```
4. **IMEDIATAMENTE** (dentro de 5 segundos), **ligue a lâmpada fisicamente**
5. **Observe a lâmpada:**
   - Se ela piscar ou mudar de estado, o pareamento pode ter funcionado
   - Teste controlando pelo Home Assistant

### Método 2: Pareamento com Lâmpada Ligada (Alternativo)

1. **Ligue a lâmpada fisicamente**
2. **Aguarde 2 segundos**
3. **Execute o serviço de pareamento** no Home Assistant:
   ```
   esphome.sensor_teste_pair_bed_room_light
   ```
4. **Observe a lâmpada:**
   - Se ela piscar ou mudar de estado, o pareamento pode ter funcionado
   - Teste controlando pelo Home Assistant

## ⚠️ Problemas Comuns

### Problema: Lâmpada não responde após pareamento

**Possíveis causas:**
1. **Timing incorreto:** Você tem apenas 5 segundos após executar o comando para ligar a lâmpada
2. **Distância:** O ESP32 está muito longe da lâmpada (máximo 2-3 metros)
3. **Lâmpada não está em modo de pareamento:** Algumas lâmpadas precisam ser desligadas por mais tempo
4. **Interferência:** Outros dispositivos BLE podem estar interferindo

**Soluções:**
- Tente várias vezes (algumas lâmpadas precisam de múltiplas tentativas)
- Aproxime o ESP32 da lâmpada (máximo 2-3 metros)
- Aguarde pelo menos 10 segundos entre tentativas
- Desligue outros dispositivos BLE próximos
- Tente ambos os métodos (com lâmpada desligada e ligada)

### Problema: Comandos não são enviados

**Verifique:**
1. Os logs mostram erros ao configurar/adicionar advertising?
2. O ESP32 está funcionando corretamente?
3. O BLE está habilitado no ESP32?

**Logs esperados:**
```
[I][lampsmartpro: ] Advertising iniciado com sucesso
[I][lampsmartpro: ] Pacote BLE transmitido com sucesso
```

Se você vê erros, verifique a configuração do BLE.

### Problema: Lâmpada responde mas para de funcionar

**Causa:** O pareamento foi perdido ou a lâmpada foi pareada com outro dispositivo.

**Solução:** Execute o processo de pareamento novamente.

## 📊 Logs de Diagnóstico

### Logs de Pareamento Bem-Sucedido

Quando o pareamento funciona, você verá:
```
[I][lampsmartpro: ] COMANDO DE PARAMENTO RECEBIDO!
[I][lampsmartpro: ] Host ID: [0x5D, 0xC8]
[I][lampsmartpro: ] Group ID: 0x01
[I][lampsmartpro: ] Advertising iniciado com sucesso
[I][lampsmartpro: ] Pacote BLE transmitido com sucesso
```

### Logs de Comando Enviado

Quando você tenta controlar a lâmpada:
```
[I][lampsmartpro: ] Enviando comando TURN_ON para a lampada...
[I][lampsmartpro: ] Advertising iniciado com sucesso
[I][lampsmartpro: ] Comando TURN_ON enviado
```

Se você vê esses logs mas a lâmpada não responde, o problema é o pareamento.

## 🎯 Checklist de Troubleshooting

- [ ] O ESP32 está próximo à lâmpada (máximo 2-3 metros)?
- [ ] A lâmpada está ligada quando você tenta controlar?
- [ ] Você seguiu o processo de pareamento corretamente?
- [ ] Aguardou pelo menos 10 segundos entre tentativas?
- [ ] Tentou ambos os métodos (lâmpada desligada e ligada)?
- [ ] Os logs mostram que os comandos estão sendo enviados?
- [ ] Não há outros dispositivos BLE interferindo?

## 💡 Dicas Finais

1. **Paciência:** Algumas lâmpadas precisam de várias tentativas
2. **Distância:** Mantenha o ESP32 próximo à lâmpada durante o pareamento
3. **Timing:** O timing é crítico - você tem apenas 5 segundos
4. **Teste:** Após o pareamento, teste ligando/desligando pelo Home Assistant
5. **Repita:** Se não funcionar, repita o processo completo

## 🔄 Se Nada Funcionar

Se após várias tentativas a lâmpada ainda não responde:

1. **Verifique o modelo da lâmpada:** Este componente pode não ser compatível com seu modelo específico
2. **Verifique a versão do firmware da lâmpada:** Algumas versões podem ter protocolos diferentes
3. **Tente com outro ESP32:** Pode ser um problema de hardware
4. **Verifique os logs completos:** Pode haver erros que não estão sendo mostrados

## 📝 Informações para Suporte

Se precisar de ajuda, forneça:
- Logs completos do ESPHome (nível DEBUG)
- Modelo exato da lâmpada
- Versão do ESPHome
- Versão do Home Assistant
- Distância entre ESP32 e lâmpada
- Quantas tentativas você já fez

