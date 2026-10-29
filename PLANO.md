# Plano de construção — robo4

Robô quadrúpede de bancada (~20 cm) com **joelhos**: cada perna tem 2 juntas (quadril + joelho), 8 servos no total, controlados por um **Arduino Uno** com um **Sensor Shield V5** encaixado por cima.

Valores de preço são **estimativas** (out/2026, lojas brasileiras e AliExpress) — confira antes de comprar.

---

## 1. Decisões de projeto (e o porquê)

| Decisão | Escolha | Por quê |
|---|---|---|
| Juntas por perna | 2 (quadril + joelho), no mesmo plano | É o mínimo para o pé subir, avançar e descer. Andar para os lados exigiria um 3º servo (12 no total). |
| Servo | **MG90S** (engrenagem de metal) | O joelho segura o peso do corpo. O SG90 (plástico) espana os dentes com o tempo. MG90S tem ~2,2 kg·cm, cabe nos mesmos suportes. |
| Shield | **Sensor Shield V5.0** para Uno | Tem 3 pinos (S/V/G) para cada porta digital: os 8 servos plugam direto, sem protoboard. Tem borne para alimentação externa dos servos. |
| Biblioteca | `Servo.h` (vem com a IDE) | No Uno ela controla até 12 servos em qualquer pino digital. Usa o Timer1, então D9/D10 perdem o `analogWrite` (não usamos). Não precisa de driver PCA9685 para 8 servos. |
| Energia dos servos | Bateria **LiPo 2S (7,4 V)** → **UBEC 6 V / 5 A** → borne do shield | 8 MG90S puxam 4–6 A de pico. O 5 V do Uno aguenta ~0,5 A: se alimentar por ali, o Arduino reinicia ou queima. |
| Energia do Arduino | A mesma LiPo no **jack DC** do Uno (plugue P4 5,5 × 2,1 mm, centro positivo) | 7,4 V está dentro da faixa do regulador do Uno (7–12 V). GND tudo ligado junto. |
| Estrutura | Impressão 3D (PLA/PETG) ou acrílico/MDF 3 mm cortado | Qualquer um serve. O importante é medir **L1** (coxa) e **L2** (canela) eixo a eixo e colocar no código. Referência: L1 = 55 mm, L2 = 60 mm. |
| Marchas | Marcha lenta (*creep*, 1 pé no ar) e trote (2 pés diagonais) | A marcha lenta é estável sem sensor nenhum; o trote é mais rápido, mas precisa de pernas bem calibradas. |

> **Alternativa de shield:** se você já tem uma placa **PCA9685** (16 canais, I2C), ela também serve: liga em A4/A5 e tem borne V+ próprio. Aí o código troca `Servo.h` pela biblioteca `Adafruit_PWMServoDriver`. Este plano segue o Sensor Shield V5, que é mais simples para começar.

---

## 2. Lista de peças

| # | Item | Qtd | Estimativa (R$) |
|---|---|---|---|
| 1 | Arduino Uno R3 (ou compatível) + cabo USB | 1 | 50–130 |
| 2 | Sensor Shield V5.0 para Uno | 1 | 20–35 |
| 3 | Micro servo MG90S (metal) | 8 (+1 de reserva) | 18–30 cada |
| 4 | Bateria LiPo 2S 7,4 V 1000–2200 mAh (XT60) | 1 | 70–140 |
| 5 | UBEC / step-down 5–6 V, **5 A ou mais** | 1 | 20–45 |
| 6 | Chave liga/desliga (≥ 5 A) | 1 | 5–10 |
| 6b | Alarme de tensão para LiPo (bipador) | 1 | 8–15 |
| 6d | Plugue P4 macho 5,5 × 2,1 mm com rabicho (bateria → jack DC do Uno) | 1 | 5–10 |
| 6c | Fio 18 AWG vermelho/preto (~1 m) para a linha de força | 1 | 8–15 |
| 7 | Carregador balanceado para LiPo 2S | 1 | 60–150 |
| 8 | Chassi (placa 3 mm acrílico/MDF ou impressa) | 1 | 10–40 |
| 9 | Coxas e canelas (impressas ou cortadas) | 4 + 4 | filamento ~20 |
| 10 | Pés de borracha (ponteiras/pés de móvel) | 4 | 5–10 |
| 11 | Parafusos e porcas M2 e M3, espaçadores M3 | kit | 15–30 |
| 12 | Abraçadeiras, velcro, cantoneiras pequenas | — | 10–20 |
| 13 | Opcional: módulo Bluetooth HC-05/HC-06 | 1 | 25–40 |
| 14 | Opcional: sensor ultrassônico HC-SR04 | 1 | 8–15 |
| 15 | Opcional: IMU MPU-6050 (equilíbrio) | 1 | 15–25 |

**Total básico estimado:** R$ 450–850 (sem impressora 3D e sem os opcionais).

**Ferramentas:** chave Phillips pequena, alicate de corte, multímetro, ferro de solda (só para o conector XT60 e a chave), computador com Arduino IDE ou `arduino-cli`.

---

## 3. Diagramas

| Arquivo | O que mostra |
|---|---|
| `guia/assets/diagrama-pecas.svg` | Peças com medidas: parede lateral (2), base, coxa (4), canela (4), medidas do MG90S, peso alvo 350–450 g |
| `guia/assets/diagrama-fiacao.svg` | Bateria → chave → UBEC → borne dos servos; jack DC do Uno; jumper SEL; cabo do servo; regras de segurança |
| `guia/assets/diagrama-pinos.svg` | Vista de cima: número de cada perna no código, pinos D2–D9 e sinal de cada servo |
| `guia/assets/diagrama-perna.svg` | Geometria de uma perna em escala e as fórmulas da cinemática inversa |
| `guia/assets/diagrama-calibracao.svg` | Servos em 90°, como encaixar coxa e canela, ajuste fino pelo Monitor Serial |
| `guia/assets/diagrama-marcha.svg` | Quando cada pé sobe no trote e na marcha lenta |

Ilustrações (geradas por IA, servem de referência visual, não são fotos do protótipo): `robo4-montado.jpg`, `vista-explodida.jpg`, `perna-detalhe.jpg`, `fiacao-bancada.jpg`, `kit-pecas.jpg`.

---

## 4. Mapa de pinos

| Perna (código) | Quadril | Joelho | Sinal quadril / joelho |
|---|---|---|---|
| 0 · FE frente esquerda | D2 | D3 | +1 / −1 |
| 1 · FD frente direita | D4 | D5 | −1 / +1 |
| 2 · TE trás esquerda | D6 | D7 | +1 / −1 |
| 3 · TD trás direita | D8 | D9 | −1 / +1 |

Livres: **D0/D1** (USB e Bluetooth), **A4/A5** (I2C para IMU ou PCA9685), A0–A3 e D10–D13 para sensores.

O sinal vem do espelhamento do lado direito. Ele depende de como você parafusou o servo: se uma junta girar para o lado errado no teste, troque o sinal dela em `SENTIDO[][]`.

---

## 5. Passo a passo

### Fase 1 — Preparar (1 dia)
1. Compre as peças da lista. Carregue a LiPo no carregador balanceado.
2. Instale o `arduino-cli` (`mkdir -p ~/.local/bin && curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.local/bin sh`, com `~/.local/bin` no PATH, depois `arduino-cli core update-index`) e o compilador do Uno: `arduino-cli core install arduino:avr` e `arduino-cli lib install Servo` (ou use a Arduino IDE). No Linux: `sudo usermod -aG dialout $USER` e entre de novo na sessão. Clones com CH340 aparecem como `/dev/ttyUSB0`.
3. **Teste a energia sem nada ligado:** bateria → chave → UBEC. Meça a saída com o multímetro: tem que dar entre 5 e 6 V.

### Fase 2 — Eletrônica na bancada (1 dia)
4. Encaixe o Sensor Shield no Uno. **Retire o jumper SEL** (com ele, os servos puxariam energia do Uno). Clones mudam a posição do jumper: confira com o multímetro que o pino V do shield **não** está ligado ao 5 V do Uno.
5. Ligue a saída do UBEC no borne de servos do shield (V+ e GND). Ligue a LiPo também no jack DC do Uno com o plugue P4 (centro positivo; o shield cobre o pino VIN). Regule o alarme da LiPo para 3,5 V por célula: abaixo de ~7 V o regulador do Uno não segura e ele reinicia.
6. Plugue os 8 servos **ainda soltos** nas portas D2–D9 (marrom = G, vermelho = V, laranja = S).
7. Carregue o sketch de montagem: `arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno firmware/montagem` (compile antes com `arduino-cli compile --fqbn arduino:avr:uno firmware/montagem`). Os 8 servos vão para 90° e ficam parados.

### Fase 3 — Mecânica (1–2 dias)
8. Corte ou imprima as peças pelo `diagrama-pecas.svg`: 2 paredes laterais 140 × 35, base 140 × 80, 4 coxas (55 mm entre eixos) e 4 canelas (60 mm do eixo ao pé). Furos para o eixo do servo e um mancal (parafuso M3 com porca travante) do lado oposto ao eixo.
9. Com os servos parados em 90°, parafuse os servos do quadril no chassi e encaixe as coxas **na vertical**.
10. Encaixe os servos do joelho na ponta das coxas e as canelas formando **ângulo reto** com a coxa. O braço do servo tem dentes: escolha o dente mais próximo; o resto se acerta no ajuste fino.
11. Prenda Uno + shield em cima com espaçadores M3; bateria e UBEC embaixo com velcro. Organize os cabos com abraçadeiras ao longo das coxas, deixando folga no joelho.

### Fase 4 — Calibrar (1–2 horas)
12. Deixe o robô suspenso (pernas no ar). Sem calibração gravada, o firmware liga em modo montagem (tudo em 90°) e só fica em pé com `x`. Carregue o firmware: `arduino-cli compile --fqbn arduino:avr:uno firmware/robo4 && arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno firmware/robo4`.
13. Abra o Monitor Serial (9600 baud): `arduino-cli monitor -p /dev/ttyACM0 -c baudrate=9600`.
14. Digite `m` (tudo em 90°). Para cada junta torta, ajuste com `o <perna> <junta> <graus>` + Enter (ex.: `o 0 1 -6`; se piorar, use o sinal oposto). Entrada incompleta não muda nada. `p` mostra os ajustes, `g` grava na EEPROM.
15. Digite `x`: o robô fica em pé. Se uma perna dobrar para o lado errado, troque o sinal em `SENTIDO[][]`, recompile e repita.

### Fase 5 — Andar (1 dia de ajustes)
16. Com o robô suspenso (num pote), digite `c` (marcha lenta) e veja a ordem: TE → FE → TD → FD. O corpo balança para a frente e para trás (`BALANCO` = 12 mm): quando um pé da frente sobe, o corpo vai para trás, e vice-versa, para o peso ficar dentro do triângulo das três pernas no chão. Com pernas de 2 juntas o corpo não anda de lado, então esse balanço é o que segura o robô; se tombar, aumente `BALANCO`, baixe a altura (`-`) ou afaste as pernas.
17. No chão: `c` para andar devagar. Depois `w` (trote para frente), `s` (ré), `a`/`d` (girar), `x` (parar), `z` (deitar), `+`/`-` (altura).
18. Se escorregar, use pés de borracha mais macios. Se tropeçar, aumente `ELEVA`. Se cair para o lado no trote, diminua `PASSO` ou aumente `CICLO_TROTE`.

### Fase 6 — Extras (opcional)
19. **Bluetooth (HC-05)** no D0/D1: os mesmos comandos de letra pelo celular (app de terminal serial). Desconecte o módulo para carregar o código.
20. **HC-SR04** (D10/D11): parar ou desviar quando houver obstáculo.
21. **MPU-6050** (A4/A5): ler a inclinação e corrigir a altura de cada perna para não tombar.

---

## 6. Como o código funciona

- **Cinemática inversa** (`cinematica()`): dado onde o pé deve estar (x para frente, y para baixo, em mm), calcula o ângulo do quadril e do joelho pela lei dos cossenos. Em pé (x = 0, y = 85): quadril ≈ −44,7°, joelho ≈ 95,2°. Conferido pela cinemática direta (o pé volta ao ponto pedido).
- **Mapeamento para o servo** (`moverJunta()`): 90° no servo = coxa vertical e joelho a 90°. Ângulo enviado = `90 + sinal × ângulo + ajuste`, limitado a 10–170°.
- **Marcha** (`peNoCiclo()`): no apoio o pé vai de +passo/2 a −passo/2 no chão (empurra o corpo para frente); no balanço ele volta pelo ar num arco senoidal de altura `ELEVA`.
- **Partida suave**: liga um servo por vez (200 ms entre eles) e cada servo anda no máximo 5° por atualização, então ficar em pé leva ~0,3 s e não dá pico dos 8 juntos. Sem calibração gravada fica em modo montagem; com calibração, fica em pé.
- **Balanço na marcha lenta**: todos os pés deslocam `−BALANCO·cos(4π(t − 1/8))` em x, levando o corpo para longe do pé que está no ar.
- **Atualização** a 50 Hz (a cada 20 ms), sem `delay()`, então os comandos do Serial respondem na hora.
- **Calibração** salva na EEPROM (8 bytes + 1 de marca): sobrevive a desligar.

Compilado com `arduino-cli` 1.x para `arduino:avr:uno`: `firmware/robo4` usa 38% da memória de programa e 19% da RAM; `firmware/montagem` usa 9%.

**Ainda não testado em hardware.** Os sinais em `SENTIDO[][]`, as medidas L1/L2 e os tempos de marcha são pontos de partida; o ajuste final é feito no robô montado.

---

## 7. Problemas comuns

| Sintoma | Causa provável | Correção |
|---|---|---|
| Arduino reinicia quando os servos mexem | Servos tirando energia do Uno | Jumper SEL fora; servos só pelo UBEC; GND comum |
| Servo treme parado | Fonte fraca ou fio fino | UBEC ≥ 5 A, fio 18 AWG na linha de força, bateria carregada |
| Servo esquenta muito | Forçando contra o limite mecânico | Ajuste `o`; confira se a perna não bate no chassi |
| Perna dobra para o lado errado | Sinal invertido | Troque o sinal em `SENTIDO[][]` |
| Robô anda torto | Calibração diferente entre lados | Refaça `m` e iguale as pernas com `o` |
| Não carrega o código | Bluetooth em D0/D1 | Desconecte o módulo durante o upload |

---

## 8. Segurança da LiPo

- Carregue só com carregador balanceado e sem deixar sozinha.
- Não descarregue abaixo de 3,3 V por célula (6,6 V no total). Um alarme de LiPo (R$ 10) avisa.
- Bateria estufada = descarte em ponto de coleta. Nunca fure.
