# robo4 — peças para impressão 3D (STL)

Peças do quadrúpede robo4 (Arduino Uno + Sensor Shield V5 + 8 servos MG90S), feitas a partir das cotas do
[desenho de peças](https://inematds.github.io/robo4/guia/assets/diagrama-pecas.svg). Guia completo:
https://inematds.github.io/robo4/guia/

| Arquivo | Medida (mm) | Quantas imprimir |
|---|---|---|
| `robo4-base.stl` | 140 × 80 × 3 | 1 |
| `robo4-parede.stl` | 140 × 35 × 3 | 2 |
| `robo4-coxa.stl` | 20 × 82 × 3 | 4 |
| `robo4-canela.stl` | 14 × 74 × 3 | 4 |
| `robo4-cantoneira.stl` | 10 × 12 × 12 | 4 |
| `robo4-mesa-completa.stl` | 208 × 182 | as 15 peças de uma vez (mesa de 220 × 220) |

## Como imprimir

- PLA ou PETG, camada 0,2 mm, 4 perímetros (a placa de 3 mm sai praticamente maciça), sem suporte.
- Placas deitadas na mesa. A cantoneira vai com a perna maior deitada, como sai no arquivo.
- Tempo aproximado da mesa completa: 3 a 4 h.

## Folgas já incluídas

- Furos dos parafusos das abas do servo (M2): 2,3 mm. O parafuso que vem com o servo abre a rosca no plástico.
- Furos M3 (Uno, pé, cantoneira): 3,4 mm.
- Centro do horn: 3,2 mm, para passar a chave no parafuso do eixo.
- Janela do servo: 23,1 × 12,8 mm (corpo do MG90S 22,8 × 12,2 + folga).
- Rasgo de 1,8 mm abaixo do centro do horn: serve para qualquer horn de 1 ponta. Encoste o horn e parafuse
  os parafusinhos dentro do rasgo.

## Montagem

1. Servos do quadril nas janelas das paredes, eixo para fora, abas presas com os parafusos do servo.
2. Paredes em pé sobre as bordas compridas da base, presas com as 4 cantoneiras e parafusos M3 × 8 + porca.
3. Uno sobre espaçadores M3 de 25 mm nos 4 furos do meio da base.
4. Horn do quadril no topo da coxa; servo do joelho na janela da coxa (corpo subindo pela coxa, eixo embaixo).
5. Horn do joelho no topo da canela; parafuso M3 com ponteira de borracha no furo do pé.
6. Encaixe as pernas só depois do sketch `firmware/montagem` (servos em 90°), coxa vertical e canela em
   ângulo reto **apontando para a frente**.

Mudou alguma medida? Edite `cad/gerar_stl.py` e rode
`python cad/gerar_stl.py` (precisa de `pip install manifold3d trimesh numpy`). Se mudar o comprimento da
coxa (55 mm eixo a eixo) ou da canela (60 mm), troque `L1` e `L2` no `firmware/robo4/robo4.ino`.

Peças ainda não testadas impressas: confira a janela com o seu servo antes de imprimir as 15.
