/**
 * Autor: Valdecir (Black Chan) - 19/SET/2022
 * Código Arduino para automação de máquina de lavar com 5 relés do módulo com 8
 * Testado na máquina Consul Maré 7,5Kg
 * vvaldecirr@hotmail.com
 */

// Declaração das variáveis
const int escoamentoB        = 4;      // bomba de escoamento da água
const int amacianteB         = 3;      // bomba de enchimento pela via do amaciante
const int sabaoB             = 2;      // bomba de enchimento pela via do sabão
const int bateRoupasA        = 1;      // bater roupa no sentido anti-horário
const int bateRoupasH        = 0;      // bater roupa no sentido horário
const int pot                = A3;     // leitura do potenciômetro
const long tempoMolho        = 180000; // tempo de molho (3 min)
const long tempoCentrifuga   = 120000; // tempo de centrífuga (2 min)
long nivelBacia              = 0;      // memória do nível de água na bacia
const int confBtn            = 5;
const int selBtn             = 6;
unsigned long lastPiscaTime  = 0;
unsigned long piscaDelay     = 500;
int principal                = 0;
int subNivel                 = 0;
int nivelSel                 = 0;
int modoSel                  = 0;
int cicloSel                 = 0;
int batidasSel               = 0;
int readConf, readSel;
bool primeiroUso             = false;
bool inicio                  = false;
bool fim                     = false;
bool sel, conf, luz;

// Declaração de auxiliares globais

// Configurações do display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define endereco  0x27 // Endereços comuns: 0x27, 0x3F
#define colunas   16
#define linhas    2
LiquidCrystal_I2C lcd(endereco, colunas, linhas);
float percent = 100.0/12;

// Ícones especiais
byte bacia[8] = {B10001, B10001, B10001, B10001, B10001, B11001, B11101, B11111};
byte lavando[8] = { B11011, B10111, B00001, B10111, B11101, B10000, B11101, B11011};
byte centrifuga[8] = { B10001, B11111, B11011, B10001, B10101, B10001, B11011, B11111};

/**
 * Função que enche a bacia por um determinado tempo para 4 níveis de roupa indicados na bacia 
 * e por determinadas vias como Bomba de Sabão ou Bomba de Amaciante
 */
void encheBacia(int nivel, boolean bs, boolean ba) {
  switch (nivel) {
    case 1:
      nivelBacia = 460000; // (7min e 40seg) // 340000; // (5min e 40seg)
      break;
    case 2:
      nivelBacia = 555000; // (9min e 15seg) // 435000; // (7min e 15seg)
      break;
    case 3:
      nivelBacia = 655000; // (10min e 55seg) // 535000; // (8min e 55seg)
      break;
    case 4:
      nivelBacia = 785000; // (13min e 5seg) // 665000; // (11min e 5seg)
      break;
  }

  // ligando bombas
  if (bs == true && ba == true) {
    digitalWrite(amacianteB, LOW);
    digitalWrite(sabaoB, LOW);
    delay(nivelBacia - 80000); // maior velocidade de enchimento
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
      
      // Centrifugar com anti-desbalanço (sempre escoando)
      digitalWrite(bateRoupasH, LOW);
      delay(5000);
      digitalWrite(bateRoupasH, HIGH);
      delay(7000);
      digitalWrite(bateRoupasH, LOW);
      delay(5000);
      digitalWrite(bateRoupasH, HIGH);
      delay(7000);
      digitalWrite(bateRoupasH, LOW);
      delay(5000);
      digitalWrite(bateRoupasH, HIGH);
      delay(7000);
      digitalWrite(bateRoupasH, LOW);
      delay(tempoCentrifuga);
      break;
      
    case 3:
      digitalWrite(escoamentoB, LOW);
      delay(nivelBacia / divisor);
      
      // Centrifugar com anti-desbalanço (sempre escoando)
      digitalWrite(bateRoupasH, LOW);
      delay(5000);
      digitalWrite(bateRoupasH, HIGH);
      delay(7000);
      digitalWrite(bateRoupasH, LOW);
      delay(5000);
      digitalWrite(bateRoupasH, HIGH);
      delay(7000);
      digitalWrite(bateRoupasH, LOW);
      delay(5000);
      digitalWrite(bateRoupasH, HIGH);
      delay(7000);
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
  pinMode(confBtn, INPUT);
  pinMode(selBtn, INPUT);
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

  // Splash screen
  lcd.init();
  lcd.createChar(0, bacia);       // ícone de enchimento da bacia
  lcd.createChar(1, lavando);     // ícone de batida de roupas
  lcd.createChar(2, centrifuga);  // ícone da centrífuga nas roupas
  lcd.clear();
  lcd.backlight();
  delay(500);
  lcd.print(" LavaTRON  v1.1 ");
  delay(1000);
  lcd.setCursor(0, 1);
  for (int i=0; i<16; i++) {
    lcd.write(byte(255));
    delay(80);
  }
  delay(700);
  sel = true; // ir par a tela de seleção de nível de água automaticamente
  //lcd.noBacklight();
}

/**
 * FUNÇÃO PADRÃO DE CICLO DE EXECUÇÃO DO ARDUINO
 */
void loop() {
  
  // botão de confirmação
  readConf = digitalRead(confBtn);
  if (readConf == 1) {
    delay(10);
    readConf = digitalRead(confBtn);
    if (readConf == 0)
      conf = true;
  }
  
  // botão de seleção
  readSel = digitalRead(selBtn);
  if (readSel == 1) {
    delay(10);
    readSel = digitalRead(selBtn);
    if (readSel == 0)
      sel = true;
  }

  // System Menu
  if (conf || sel) {
    lcd.backlight();
    fim = false;

    if (principal == 0) // primeiro nível de menu
      principal = 1;

    switch (principal) {
      case 1: // Nível de água
        lcd.setCursor(0, 0);
        lcd.print(" NIVEL de agua "); lcd.write(byte(0));
        lcd.setCursor(0, 1);
        
          switch (subNivel) {
            case 0: // Seleção
              lcd.print("  selecione...  ");
              //lcd.setCursor(3, 1);
              if (sel) {
                subNivel = 1;
                sel = false;
              }
              break;
            case 1: // Nível 1 de água 
              lcd.print("  ");lcd.write(byte(126)); lcd.print("1"); lcd.write(byte(127)); lcd.print(" 2  3  4   ");
              lcd.setCursor(3, 1);
              if (sel) {
                subNivel = 2;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 2;
                conf = false;
                sel = true;
              }
              nivelSel = 4; // nivelBacia 1
              break;
            case 2: // Nível 2 de água 
              lcd.print("   1 ");lcd.write(byte(126)); lcd.print("2"); lcd.write(byte(127)); lcd.print(" 3  4   ");
              lcd.setCursor(6, 1);
              if (sel) {
                subNivel = 3;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 2;
                conf = false;
                sel = true;
              }
              nivelSel = 1; // nivelBacia 2
              break;
            case 3: // Nível 3 de água 
              lcd.print("   1  2 ");lcd.write(byte(126)); lcd.print("3"); lcd.write(byte(127)); lcd.print(" 4   ");
              lcd.setCursor(9, 1);
              if (sel) {
                subNivel = 4;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 2;
                conf = false;
                sel = true;
              }
              nivelSel = 2; // nivelBacia 3
              break;
            case 4: // Nível 4 de água 
              lcd.print("   1  2  3 ");lcd.write(byte(126)); lcd.print("4"); lcd.write(byte(127)); lcd.print("  ");
              lcd.setCursor(12, 1);
              if (sel) {
                subNivel = 1;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 2;
                conf = false;
                sel = true;
              }
              nivelSel = 3; // nivelBacia 4
              break;
          }
        lcd.blink();
      break;

      case 2: // Modo de lavagem
        lcd.noBlink(); // parar piscagem anterior do cursor
        lcd.setCursor(0, 0);
        lcd.print("MODO de lavagem:"); lcd.write(byte(0));
        lcd.setCursor(0, 1);
        if (subNivel == 0) // segundo nível de menu
          subNivel = 1;
          switch (subNivel) {
            case 1: // Nível 1 de água 
              lcd.print("    "); lcd.write(byte(126)); lcd.print("NORMAL"); lcd.write(byte(127)); lcd.print("  SU");
              if (sel) {
                subNivel = 2;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 3;
                conf = false;
                sel = true;
              }
              modoSel = 3; // modo 2 = Normal
              break;
            case 2: // Nível 2 de água 
              lcd.print("AL  "); lcd.write(byte(126)); lcd.print("SUAVE"); lcd.write(byte(127)); lcd.print("  TUR");
              if (sel) {
                subNivel = 3;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 3;
                conf = false;
                sel = true;
              }
              modoSel = 2; // modo 1 = Suave
              break;
            case 3: // Nível 3 de água 
              lcd.print("AVE  "); lcd.write(byte(126)); lcd.print("TURBO "); lcd.write(byte(127)); lcd.print("    ");
              if (sel) {
                subNivel = 1;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 3;
                conf = false;
                sel = true;
              }
              modoSel = 1; // modo 3 = Trubo
              break;
          }
        lcd.blink();
      break;

      case 3: // Modo de lavagem
        lcd.setCursor(0, 0);
        lcd.print("    DURACAO:   "); lcd.write(byte(3));
        lcd.setCursor(0, 1);
        if (subNivel == 0) // segundo nível de menu
          subNivel = 1;
          switch (subNivel) {
            case 1: // duração da lavagem
              lcd.print("    "); lcd.write(byte(126)); lcd.print("CURTO"); lcd.write(byte(127)); lcd.print("  NOR");
              if (sel) {
                subNivel = 2;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 4;
                conf = false;
                sel = true;
              }
              cicloSel = 4; // 4 ciclos de duração 
              break;
            case 2: // duração da lavagem
              lcd.print("TO  "); lcd.write(byte(126)); lcd.print("NORMAL"); lcd.write(byte(127)); lcd.print("  LO");
              if (sel) {
                subNivel = 3;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 4;
                conf = false;
                sel = true;
              }
              cicloSel = 2; // 2 ciclos de duração
              break;
            case 3: // duração da lavagem
              lcd.print("MAL  "); lcd.write(byte(126)); lcd.print("LONGO"); lcd.write(byte(127)); lcd.print("    ");
              if (sel) {
                subNivel = 1;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 4;
                conf = false;
                sel = true;
              }
              cicloSel = 3; // 3 ciclos de duração
              break;
          }
        lcd.blink();
      break;    

      case 4: // Estado de sujeira das roupas
        lcd.setCursor(0, 0);
        lcd.print("ESTADO sujeira: "); lcd.write(byte(3));
        lcd.setCursor(0, 1);
        if (subNivel == 0) // segundo nível de menu
          subNivel = 1;
          switch (subNivel) {
            case 1: // nível de sujeira
              lcd.print(" "); lcd.write(byte(126)); lcd.print("POUCO SUJA"); lcd.write(byte(127)); lcd.print(" SU");
              if (sel) {
                subNivel = 2;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 5;
                conf = false;
                sel = true;
              }
              batidasSel = 120; // 4 ciclos de duração 
              break;
            case 2: // nível de sujeira
              lcd.print("UJA  "); lcd.write(byte(126)); lcd.print("SUJA"); lcd.write(byte(127)); lcd.print("  MUI");
              if (sel) {
                subNivel = 3;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 5;
                conf = false;
                sel = true;
              }
              batidasSel = 60; // 2 ciclos de duração
              break;
            case 3: // nível de sujeira
              lcd.print("JA "); lcd.write(byte(126)); lcd.print("MUITO SUJA"); lcd.write(byte(127)); lcd.print(" ");
              if (sel) {
                subNivel = 1;
                sel = false;
              } else if (conf) {
                subNivel  = 0;
                principal = 5;
                conf = false;
                sel = true;
              }
              batidasSel = 90; // 3 ciclos de duração
              break;
          }
        lcd.blink();
      break;    
    default:
      inicio = true;
      sel = false; // bugfix de reinício da lavagem
      conf = false; // bugfix de reinício da lavagem
      principal = 0;
      subNivel = 0;
      break;
    }

  }

  // Se configuração estiver concluída INICIAR LAVAGEM
  if (inicio) {
    lcd.clear();
    lcd.backlight();
    // primeira lavagem
    lcd.setCursor(0, 0);
    lcd.print("L"); lcd.write(byte(127)); lcd.print("Enchendo 1/3 "); lcd.write(byte(0));
      lcd.setCursor(0, 1);
      lcd.print("  0%");
        //lcd.setCursor(4, 1);
        //lcd.write(byte(255));
    encheBacia(nivelSel, true, false); // [nível_de_água_1_ao_4, bomba_de_sabão, bomba_de_amaciante]
    lcd.setCursor(0, 0);
    lcd.print("L"); lcd.write(byte(127)); lcd.print("Batendo  1/3 "); lcd.write(byte(1));
      lcd.setCursor(1, 1);
      lcd.print((String)(int)ceil(percent*2));
        lcd.setCursor(4, 1);
        lcd.write(byte(255));
        lcd.write(byte(255));
    baterRoupas(batidasSel, cicloSel, modoSel); // [número_de_batidas, ciclos, modo_1_ao_3]
    lcd.setCursor(0, 0);
    lcd.print("L"); lcd.write(byte(127)); lcd.print("Escoando 1/3 "); lcd.write(byte(2));
      lcd.setCursor(1, 1);
      lcd.print((String)(int)ceil(percent*3));
        lcd.setCursor(6, 1);
        lcd.write(byte(255));
    escoarBacia(2); // [modo_1_ao_3]
  
    // enxágue ou segunda lavagem
    lcd.setCursor(0, 0);
    lcd.print("E"); lcd.write(byte(127)); lcd.print("Enchendo 2/3 "); lcd.write(byte(0));
      lcd.setCursor(1, 1);
      lcd.print((String)(int)ceil(percent*5));
        lcd.setCursor(7, 1);
        lcd.write(byte(255));
        lcd.write(byte(255));
    encheBacia(nivelSel, true, false); // [nível_de_água_1_ao_4, bomba_de_sabão, bomba_de_amaciante]
    lcd.setCursor(0, 0);
    lcd.print("E"); lcd.write(byte(127)); lcd.print("Batendo  2/3 "); lcd.write(byte(1));
      lcd.setCursor(1, 1);
      lcd.print((String)(int)ceil(percent*6));
        lcd.setCursor(9, 1);
        lcd.write(byte(255));
    baterRoupas((batidasSel - 20), 2, modoSel); // [número_de_batidas, ciclos, modo_1_ao_3]
    lcd.setCursor(0, 0);
    lcd.print("E"); lcd.write(byte(127)); lcd.print("Escoando 2/3 "); lcd.write(byte(2));
      lcd.setCursor(1, 1);
      lcd.print((String)(int)ceil(percent*7));
        lcd.setCursor(10, 1);
        lcd.write(byte(255));
    escoarBacia(3); // [modo_1_ao_3]
  
    // amaciante ou enxágue
    lcd.setCursor(0, 0);
    lcd.print("A"); lcd.write(byte(127)); lcd.print("Enchendo 3/3 "); lcd.write(byte(0));
      lcd.setCursor(1, 1);
      lcd.print((String)(int)ceil(percent*8));
        lcd.setCursor(11, 1);
        lcd.write(byte(255));
    encheBacia((nivelSel - 1), true, true); // [nível_de_água_1_ao_4, bomba_de_sabão, bomba_de_amaciante]
    lcd.setCursor(0, 0);
    lcd.print("A"); lcd.write(byte(127)); lcd.print("Batendo  3/3 "); lcd.write(byte(1));
      lcd.setCursor(1, 1);
      lcd.print((String)(int)ceil(percent*9));
        lcd.setCursor(12, 1);
        lcd.write(byte(255));
    baterRoupas((batidasSel - 40), 1, modoSel); // [número_de_batidas, ciclos, modo_1_ao_3]
    lcd.setCursor(0, 0);
    lcd.print("A"); lcd.write(byte(127)); lcd.print("Escoando 3/3 "); lcd.write(byte(2));
      lcd.setCursor(1, 1);
      lcd.print((String)(int)ceil(percent*11));
        lcd.setCursor(13, 1);
        lcd.write(byte(255));
        lcd.write(byte(255));
    escoarBacia(2); // [modo_1_ao_3]
  
    // FIM
    lcd.setCursor(0, 0);
    lcd.write(byte(126)); lcd.print(+"Roupas Limpas!"); lcd.write(byte(127));
      lcd.setCursor(0, 1);
      lcd.print((String)(int)ceil(percent*12));
        lcd.setCursor(14, 1);
        lcd.write(byte(255));
    inicio = false;
    fim = true;
    lcd.noBlink();
  }

  // pisca alerta
  if (fim) {
    if (millis() - lastPiscaTime > piscaDelay) {
      if (luz) {
        lastPiscaTime = millis();
        lcd.noBacklight();
        luz = false;
      } else {
        lastPiscaTime = millis();
        lcd.backlight();
        luz = true;
      }
    }
  }
}
