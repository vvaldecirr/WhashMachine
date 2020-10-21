/**
 * Autor: Valdecir (Black Chan) - 16/JUL/2020
 * Código Arduino para automação de máquina de lavar com 5 relés do módulo com 8
 * Testado na máquina Consul Maré 7,5Kg
 * vvaldecirr@hotmail.com
 */

// Declaração dos motores
const int escoamentoB       = 4;      // bomba de escoamento da água
const int amacianteB        = 3;      // bomba de enchimento pela via do amaciante
const int sabaoB            = 2;      // bomba de enchimento pela via do sabão
const int bateRoupasA       = 1;      // bater roupa no sentido anti-horário
const int bateRoupasH       = 0;      // bater roupa no sentido horário
const int pot               = A5;     // leitura do potenciômetro
const long tempoMolho       = 180000; // tempo de molho (3 min)
const long tempoCentrifuga  = 120000; // tempo de centrífuga (2 min)

// Declaração de auxiliares globais
long nivelBacia             = 0;      // memória do nível de água na bacia

/**
 * Função que enche a bacia por um determinado tempo para 4 níveis de roupa indicados na bacia 
 * e por determinadas vias como Bomba de Sabão ou Bomba de Amaciante
 */
void encheBacia(int nivel, boolean bs, boolean ba) {
  switch (nivel) {
    case 1:
      nivelBacia = 300000; // (5min e 0seg)
      break;
    case 2:
      nivelBacia = 425000; // (7min e 0seg)
      break;
    case 3:
      nivelBacia = 535000; // (9min e 0seg)
      break;
    case 4:
      nivelBacia = 665000; // (11min e 0seg)
      break;
  }

  // ligando bombas
  if (bs == true && ba == true) {
    digitalWrite(amacianteB, LOW);
    digitalWrite(sabaoB, LOW);
    delay(nivelBacia - 115000); // maior velocidade de enchimento
  } else if (bs == true) {
    digitalWrite(sabaoB, LOW);
    delay(nivelBacia);
  } else {
    digitalWrite(amacianteB, LOW);
    delay(nivelBacia);
  }

  digitalWrite(amacianteB, HIGH);
  digitalWrite(sabaoB, HIGH);
}

/**
 * Função que bate as roupas num determinado número de BATIDAS
 * na velocidade do potenciômetro, num determinado número de CICLOS
 * de um determinado MODO: (1=suave / 2=normal / 3=turbo)
 */
void baterRoupas(int batidas, int ciclos, int modo) {
  int pausa;
  
  for (int i = 0; i < ciclos; i++) {
    switch (modo) {
      case 1:
        pausa = 800;
        break;
      case 2:
        pausa = 400;
        break;
      case 3:
        pausa = 0;
        break;
    }
  
    // Bater roupas com ajuste fino do potenciômetro
    for (int j = 0; j < batidas; j++) {
      int tempoRele = map(analogRead(pot), 0, 1023, 400, 1500); // ajuste fino do tempo pelo potenciômetro

      digitalWrite(bateRoupasH, LOW);
      delay(tempoRele);
      digitalWrite(bateRoupasH, HIGH);
      delay(pausa);

      digitalWrite(bateRoupasA, LOW);
      delay(tempoRele);
      digitalWrite(bateRoupasA, HIGH);
      delay(pausa);
    }
  
    // retirando o tempo de molho após a última batida
    if (i < (ciclos - 1))
      delay(tempoMolho);      
  }
  
  digitalWrite(bateRoupasA, HIGH);
  digitalWrite(bateRoupasH, HIGH);
}

/**
 * Função que escoa a bacia:
 * 1 = sem centrífuga;
 * 2 = centrífuga sem pré-enxague;
 * 3 = centrífuga + pré-enxague;
 */
void escoarBacia(int modo) {
  int divisor = 4; // ajuste do tempo de escoamento para nível de água existente na bacia
  
  switch (modo) {
    case 1:
      digitalWrite(escoamentoB, LOW);
      delay(nivelBacia / divisor);
      break;
      
    case 2:
      digitalWrite(escoamentoB, LOW);
      delay(nivelBacia / divisor);
      
      // Centrifugar (sempre escoando)
      digitalWrite(bateRoupasH, LOW);
      delay(tempoCentrifuga);
      break;
      
    case 3:
      digitalWrite(escoamentoB, LOW);
      delay(nivelBacia / divisor);
      
      // Centrifugar (sempre escoando)
      digitalWrite(bateRoupasH, LOW);
      delay(tempoCentrifuga / 2);
      
      // Pré-enxague aprimorando a eliminação de sabão no escoamento
      digitalWrite(sabaoB, LOW);
      delay(tempoCentrifuga);
      break;
  }
  
  // Atualizando o nível de água na bacia
  nivelBacia = 0;

  digitalWrite(escoamentoB, HIGH);
  digitalWrite(bateRoupasH, HIGH);
  digitalWrite(sabaoB, HIGH);
}

/**
 * Função de aviso de fim de lavagem
 */
/*void fimHalt() {}*/

/**
 * FUNÇÃO PADRÃO DE CONFIGURAÇÃO DOS CANAIS DO ARDUINO
 */
void setup() {
  // put your setup code here, to run once:
  pinMode(escoamentoB, OUTPUT);
  pinMode(amacianteB, OUTPUT);
  pinMode(sabaoB, OUTPUT);
  pinMode(bateRoupasA, OUTPUT);
  pinMode(bateRoupasH, OUTPUT);
  pinMode(pot, INPUT);

  digitalWrite(escoamentoB, HIGH);
  digitalWrite(amacianteB, HIGH);
  digitalWrite(sabaoB, HIGH);
  digitalWrite(bateRoupasA, HIGH);
  digitalWrite(bateRoupasH, HIGH);
}

/**
 * FUNÇÃO PADRÃO DE CICLO DE EXECUÇÃO DO ARDUINO
 */
void loop() {
  // primeira lavagem
  encheBacia(4, true, false);
  baterRoupas(80, 5, 1);
  escoarBacia(3);

  // enxágue ou segunda lavagem
  encheBacia(4, true, false);
  baterRoupas(60, 1, 1);
  escoarBacia(2);

  // amaciante ou enxágue
  encheBacia(3, false, true);
  baterRoupas(60, 2, 1);
  escoarBacia(2);

  // FIM
  // fazer nada infinitamente
  while (1==1) {
    delay(10000);
  }
}
