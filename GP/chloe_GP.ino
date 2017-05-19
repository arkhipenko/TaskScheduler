const int grid_size = 3;
const int nLed = grid_size * grid_size;
const int nPin = grid_size * 2;
const byte pin_m[nPin] = {4, 3, 2, 5, 6, 7}; //à redéfinir
boolean grid[grid_size][grid_size] = {
  0, 0, 0,
  0, 0, 0,
  0, 0, 0
};

int pointeur = 0;

void setup() {
  // Initialisation de la matrice
  for (int i = 0; i < grid_size; i++) {
    pinMode(pin_m[i], OUTPUT);
    digitalWrite(i, LOW);
    pinMode(pin_m[i + grid_size], OUTPUT);
    digitalWrite(i, HIGH);
  }

  // test matrice
  for (int i = 0; i < nLed; i++) {
    int pinIn = i / grid_size;
    int pinOut = grid_size + (i % grid_size);

    digitalWrite(pin_m[pinIn], HIGH);
    digitalWrite(pin_m[pinOut], LOW);
    delay(200);

    digitalWrite(pin_m[pinIn], LOW);
    digitalWrite(pin_m[pinOut], HIGH);
    delay(50);
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}

void display_grid() {
  for (int i = 0; i < nLed; i++) {

    int pinIn = i / grid_size;
    int gyOut = i % grid_size;
    int pinOut = grid_size + gyOut;
    boolean display_;
    if (i == pointeur) {
      display_ != grid[pinIn][gyOut];
    }
    else {
      display_ = grid[pinIn][gyOut];
    }

    if (display_) {
      digitalWrite(pin_m[pinIn], HIGH);
      digitalWrite(pin_m[pinOut], LOW);
      delay(2);
      digitalWrite(pin_m[pinIn], LOW);
      digitalWrite(pin_m[pinOut], HIGH);
    }
    else {}
  }
}


void commande(int _input) {

  switch (_input) {
    // nothing
    case 0: {
      }
      break;
    // UP
    case 1: {
        pointeur -= grid_size;
      }
      break;
    // DOWN
    case 2: {
        pointeur += grid_size;
      }
      break;
    // LEFT
    case 3: {
        pointeur --;
      }
      break;

    // RIGHT
    case 4: {
        pointeur ++;
      }
      break;

    // changer d'etat de la case actif
    case 5: {

      }
      break;
      
    default:
      break;
  }
}


