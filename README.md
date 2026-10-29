# robo4 — robô quadrúpede com joelhos

[![robo4 — robô quadrúpede](guia/assets/banner.jpg)](https://inematds.github.io/robo4/guia/)

**🇧🇷 [Português](README.md) · 🇺🇸 [English](README.en.md) · 🇪🇸 [Español](README.es.md)**

## O que é

O robo4 é um plano completo para construir, em casa, um pequeno robô de quatro patas (uns 20 cm) que anda. Cada perna tem dois servos: um no quadril e outro no joelho, 8 no total. Quem controla tudo é um Arduino Uno com um Sensor Shield V5 encaixado por cima, onde os servos são plugados. O repositório traz a lista de peças, os diagramas de fiação, o passo a passo de montagem e calibração, e o código que já compila para o Uno.

## 📖 Guia de uso

Guia completo (landing + passo a passo): **https://inematds.github.io/robo4/guia/**

Plano detalhado (decisões, peças, pinos, problemas comuns): [PLANO.md](PLANO.md)

## O que tem aqui

| Caminho | Conteúdo |
|---|---|
| `PLANO.md` | Plano de construção completo |
| `firmware/montagem/` | Sketch que põe os 8 servos em 90° para encaixar as pernas |
| `firmware/robo4/` | Firmware: cinemática inversa, trote, marcha lenta, giro, calibração na EEPROM |
| `guia/assets/diagrama-*.svg` | Diagramas: fiação, pinos, geometria da perna, calibração, marcha (PT, `-en`, `-es`) |
| `guia/assets/*.jpg` | Ilustrações geradas por IA (kit, vista explodida, perna, fiação, robô montado) |
| `guia/` | Página do guia em PT, `guia/en/` e `guia/es/` |
| `pecas/stl/` | Peças para impressão 3D (base, paredes, coxas, canelas, cantoneiras) e a mesa completa; gerador em `cad/gerar_stl.py` |

## Hardware em uma linha

Arduino Uno + Sensor Shield V5 · 8× MG90S nos pinos D2–D9 · LiPo 2S → UBEC 6 V/5 A no borne dos servos (jumper SEL fora) · LiPo no jack DC do Uno (plugue P4) · GND comum.

## O processo, do início ao fim

Legenda: 🧠 você (com o Nei) · ⚙️ comando/script · 🤖 `claude -p`

| # | Passo | Comando | Quem |
|---|---|---|---|
| 1 | Comprar as peças da lista e carregar a LiPo | [PLANO.md › Lista de peças](PLANO.md#2-lista-de-peças) | 🧠 |
| 2 | Instalar o compilador do Uno | `arduino-cli core install arduino:avr && arduino-cli lib install Servo` | ⚙️ |
| 3 | Medir a saída do UBEC (5–6 V) antes de ligar servo | multímetro | 🧠 |
| 4 | Shield no Uno, jumper SEL fora, UBEC no borne, LiPo no jack DC | [diagrama de fiação](guia/assets/diagrama-fiacao.svg) | 🧠 |
| 5 | Pôr os 8 servos em 90° | `arduino-cli compile --fqbn arduino:avr:uno firmware/montagem && arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno firmware/montagem` | ⚙️ |
| 5b | Cortar ou imprimir as peças | [desenho com medidas](guia/assets/diagrama-pecas.svg) ou STL: `python cad/gerar_stl.py` → [`pecas/stl/`](pecas/stl/) | 🧠 |
| 6 | Montar coxas na vertical e canelas em 90° | [diagrama de calibração](guia/assets/diagrama-calibracao.svg) | 🧠 |
| 7 | Carregar o firmware | `arduino-cli compile --fqbn arduino:avr:uno firmware/robo4 && arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno firmware/robo4` | ⚙️ |
| 8 | Calibrar pelo Monitor Serial (`m`, `o <perna> <junta> <graus>`, `p`, `g`) | `arduino-cli monitor -p /dev/ttyACM0 -c baudrate=9600` | 🧠 |
| 9 | Ficar em pé (`x`) e corrigir `SENTIDO[][]` se alguma perna dobrar errado | editar `firmware/robo4/robo4.ino` e repetir o passo 7 | 🧠 |
| 10 | Marcha lenta (`c`), depois trote (`w s a d`) | Monitor Serial | 🧠 |
| 11 | Atualizar guia/diagramas e publicar | `git add -A && git commit && git push` (GitHub Pages via Actions) | 🧠 |

## Estado

O código compila para `arduino:avr:uno` (38% da memória de programa, 19% da RAM). **Ainda não foi testado num robô montado**: sinais dos servos, medidas L1/L2 e tempos de marcha são pontos de partida.

---

Feito por [INEMA.CLUB](https://inema.club).
