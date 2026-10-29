// robo4 — quadrúpede com joelhos (8 servos) em Arduino Uno + Sensor Shield V5
// Cada perna tem 2 juntas no mesmo plano: quadril (gira a coxa) e joelho (gira a canela).
// Controle pelo Monitor Serial (9600 baud, "Nenhum final de linha" ou "Nova linha"):
//   w frente   s ré   a gira esq.   d gira dir.   c marcha lenta (creep)
//   x parar (em pé)   z deitar   + / - altura   m modo montagem (todos a 90°)
//   o <perna> <junta> <graus>  ajusta a calibração. Ex.: "o 0 1 -6" (Enter no fim)
// Ao ligar sem calibração gravada, fica em modo montagem (90°); "x" põe em pé.
//   p mostra a calibração   g grava a calibração na EEPROM
// Pernas: 0=FE frente esquerda, 1=FD frente direita, 2=TE trás esquerda, 3=TD trás direita
// Juntas: 0=quadril, 1=joelho

#include <Servo.h>
#include <EEPROM.h>
#include <math.h>

// ---- Ligações (pinos do Sensor Shield) ----
// D0/D1 ficam livres para o USB/Bluetooth; A4/A5 livres para I2C (IMU, PCA9685).
const uint8_t PINO[4][2] = {{2, 3}, {4, 5}, {6, 7}, {8, 9}};

// Sentido de cada servo. O lado direito é espelhado. Se uma junta girar ao contrário
// no teste, troque o sinal dela aqui.
const int8_t SENTIDO[4][2] = {{1, -1}, {-1, 1}, {1, -1}, {-1, 1}};

// ---- Geometria (mm) — meça as suas peças e ajuste ----
const float L1 = 55.0;  // coxa: eixo do quadril até eixo do joelho
const float L2 = 60.0;  // canela: eixo do joelho até a ponta do pé
const bool JOELHO_PARA_TRAS = true;

float altura = 85.0;    // distância do quadril ao chão em pé
const float PASSO = 30.0;   // comprimento do passo
const float ELEVA = 20.0;   // quanto o pé sobe no ar
const unsigned long CICLO_TROTE = 600;   // ms por ciclo
const unsigned long CICLO_CREEP = 1600;
// Na marcha lenta o corpo vai para trás quando um pé da frente sobe e para a frente
// quando um pé de trás sobe: assim o peso fica dentro do triângulo das 3 pernas no chão.
const float BALANCO = 12.0;   // mm

// Fase de cada perna no ciclo (0..1)
const float FASE_TROTE[4] = {0.0, 0.5, 0.5, 0.0};     // pares diagonais juntos
// Uma perna no ar por vez, na ordem TE -> FE -> TD -> FD (a perna sobe quando fase >= duty)
const float FASE_CREEP[4] = {0.5, 0.0, 0.75, 0.25};

enum Modo { PARADO, TROTE, CREEP, DEITADO, MONTAGEM };
Modo modo = PARADO;
float frente = 0, giro = 0;

Servo servo[4][2];
int8_t ajuste[4][2];          // calibração em graus
const uint8_t EEPROM_MARCA = 0x4A;

// ---------- Cinemática inversa de uma perna de 2 juntas ----------
// x: para a frente a partir do quadril; y: para baixo. Devolve ângulos em graus:
// quadril = ângulo da coxa em relação à vertical (positivo = para a frente)
// joelho  = ângulo interno entre coxa e canela (180 = perna esticada)
void cinematica(float x, float y, float &quadril, float &joelho) {
  float d = sqrt(x * x + y * y);
  const float dmax = L1 + L2 - 2.0;
  const float dmin = fabs(L1 - L2) + 10.0;
  if (d > dmax) { x *= dmax / d; y *= dmax / d; d = dmax; }
  if (d < dmin) { x *= dmin / d; y *= dmin / d; d = dmin; }

  float a = atan2(x, y);
  float b = acos(constrain((L1 * L1 + d * d - L2 * L2) / (2 * L1 * d), -1.0, 1.0));
  float k = acos(constrain((L1 * L1 + L2 * L2 - d * d) / (2 * L1 * L2), -1.0, 1.0));
  quadril = degrees(JOELHO_PARA_TRAS ? a - b : a + b);
  joelho = degrees(k);
}

// Cada servo anda no máximo MAX_GRAUS por atualização (20 ms): ao ficar em pé ou trocar
// de modo, as pernas vão aos poucos e a fonte não leva o pico dos 8 servos de uma vez.
const float MAX_GRAUS = 5.0;   // o trote pede até ~4° por atualização no joelho
float atual[4][2];

void escreverServo(uint8_t p, uint8_t j, float alvo) {
  alvo = constrain(alvo, 10, 170);
  atual[p][j] += constrain(alvo - atual[p][j], -MAX_GRAUS, MAX_GRAUS);
  servo[p][j].write((int)(atual[p][j] + 0.5));
}

// Montagem: servo em 90° = coxa na vertical e joelho em ângulo reto.
void moverJunta(uint8_t p, uint8_t j, float graus) {
  float neutro = (j == 0) ? 0.0 : 90.0;
  escreverServo(p, j, 90.0 + SENTIDO[p][j] * (graus - neutro) + ajuste[p][j]);
}

void moverPe(uint8_t p, float x, float y) {
  float q, k;
  cinematica(x, y, q, k);
  moverJunta(p, 0, q);
  moverJunta(p, 1, k);
}

// Posição do pé numa fase do ciclo. duty = fração do ciclo com o pé no chão.
void peNoCiclo(float fase, float duty, float passo, float &x, float &y) {
  if (fase < duty) {                       // apoio: o pé empurra o corpo
    float s = fase / duty;
    x = passo / 2 - passo * s;
    y = altura;
  } else {                                 // balanço: o pé sobe e volta à frente
    float s = (fase - duty) / (1.0 - duty);
    x = -passo / 2 + passo * s;
    y = altura - ELEVA * sin(PI * s);
  }
}

// ---------- Calibração ----------
bool carregarAjuste() {
  if (EEPROM.read(0) != EEPROM_MARCA) { memset(ajuste, 0, sizeof(ajuste)); return false; }
  for (uint8_t i = 0; i < 8; i++) ajuste[i / 2][i % 2] = (int8_t)EEPROM.read(1 + i);
  return true;
}

void gravarAjuste() {
  EEPROM.update(0, EEPROM_MARCA);
  for (uint8_t i = 0; i < 8; i++) EEPROM.update(1 + i, (uint8_t)ajuste[i / 2][i % 2]);
  Serial.println(F("Calibracao gravada."));
}

void mostrarAjuste() {
  const char *nome[4] = {"FE", "FD", "TE", "TD"};
  for (uint8_t p = 0; p < 4; p++) {
    Serial.print(nome[p]); Serial.print(F(" quadril=")); Serial.print(ajuste[p][0]);
    Serial.print(F(" joelho=")); Serial.println(ajuste[p][1]);
  }
}

// ---------- Comandos ----------
void comando(char c) {
  switch (c) {
    case 'w': modo = TROTE; frente = 1;  giro = 0;  break;
    case 's': modo = TROTE; frente = -1; giro = 0;  break;
    case 'a': modo = TROTE; frente = 0;  giro = -1; break;
    case 'd': modo = TROTE; frente = 0;  giro = 1;  break;
    case 'c': modo = CREEP; frente = 1;  giro = 0;  break;
    case 'x': modo = PARADO; break;
    case 'z': modo = DEITADO; break;
    case '+': altura = constrain(altura + 5, 50, 105); break;
    case '-': altura = constrain(altura - 5, 50, 105); break;
    case 'm': modo = MONTAGEM; break;
    case 'p': mostrarAjuste(); break;
    case 'g': gravarAjuste(); break;
    case 'o': {
      // Lê o resto da linha: "o <perna> <junta> <graus>". Entrada incompleta não muda nada.
      String linha = Serial.readStringUntil('\n');
      int p, j, g;
      if (sscanf(linha.c_str(), "%d %d %d", &p, &j, &g) == 3 && p >= 0 && p < 4 && j >= 0 && j < 2) {
        ajuste[p][j] = constrain(g, -30, 30);
        mostrarAjuste();
      } else {
        Serial.println(F("uso: o <perna 0-3> <junta 0-1> <graus -30..30>"));
        return;
      }
      break;
    }
    default: return;
  }
  Serial.print(F("ok ")); Serial.println(c);
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(3000);
  // Sem calibração gravada, liga em MONTAGEM (tudo em 90°) e só fica em pé com "x".
  // Com calibração gravada, liga direto em pé.
  bool calibrado = carregarAjuste();
  modo = calibrado ? PARADO : MONTAGEM;
  for (uint8_t p = 0; p < 4; p++)
    for (uint8_t j = 0; j < 2; j++) {
      atual[p][j] = 90 + ajuste[p][j];
      servo[p][j].write((int)atual[p][j]);
      servo[p][j].attach(PINO[p][j]);
      delay(200);             // um servo por vez: evita pico de corrente na partida
    }
  Serial.println(calibrado ? F("robo4 pronto (calibrado, em pe).") : F("robo4 pronto (sem calibracao: modo montagem, 90 graus)."));
  Serial.println(F("Comandos: w s a d c x z + - m p g o"));
}

void loop() {
  while (Serial.available()) comando(Serial.read());

  static unsigned long ultimo = 0;
  unsigned long agora = millis();
  if (agora - ultimo < 20) return;        // atualiza os servos a 50 Hz
  ultimo = agora;

  for (uint8_t p = 0; p < 4; p++) {
    float x = 0, y = altura;
    if (modo == MONTAGEM) {
      escreverServo(p, 0, 90 + ajuste[p][0]);
      escreverServo(p, 1, 90 + ajuste[p][1]);
      continue;
    }
    if (modo == DEITADO) y = 45;
    if (modo == TROTE || modo == CREEP) {
      bool esquerda = (p == 0 || p == 2);
      float passo = PASSO * constrain(esquerda ? frente + giro : frente - giro, -1, 1);
      unsigned long ciclo = (modo == TROTE) ? CICLO_TROTE : CICLO_CREEP;
      float duty = (modo == TROTE) ? 0.5 : 0.75;
      const float *fase0 = (modo == TROTE) ? FASE_TROTE : FASE_CREEP;
      float t = (float)(agora % ciclo) / ciclo;
      float fase = fmod(t + fase0[p], 1.0);
      peNoCiclo(fase, duty, passo, x, y);
      // TE sobe em t=0..¼, FE ¼..½, TD ½..¾, FD ¾..1. Pé x positivo = corpo para trás.
      if (modo == CREEP) x += -BALANCO * cos(4 * PI * (t - 0.125));
    }
    moverPe(p, x, y);
  }
}
