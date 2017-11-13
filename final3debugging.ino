byte first_full[8] = {
        0b11111,
        0b10000,
        0b10111,
        0b10111,
        0b10111,
        0b10111,
        0b10000,
        0b11111
};
byte Inside_full[8] = {
        0b11111,
        0b00000,
        0b11111,
        0b11111,
        0b11111,
        0b11111,
        0b00000,
        0b11111
};
byte last_full[8] = {
        0b11111,
        0b00001,
        0b11101,
        0b11101,
        0b11101,
        0b11101,
        0b00001,
        0b11111
};
byte last_empty[8] = {
        0b11111,
        0b00001,
        0b00001,
        0b00001,
        0b00001,
        0b00001,
        0b00001,
        0b11111
};
byte first_empty[8] = {
        0b11111,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b11111
};
byte inside_empty[8] = {
        0b11111,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b11111
};


#include <LiquidCrystal.h>
#include <Wire.h>
#include <max6675.h>
#include <PID_v1.h>

#define _version                "ENERGIEFLEX V1.1"
#define _year                   "2017"


#define RELAY1_ON               1
#define RELAY1_OFF              0
#define RELAY2_ON               1
#define RELAY2_OFF              0
#define Relay1                  8   // pin de electrovane ssr 1
#define Relay2                  9   // pin de electrovane ssr 2
#define SSR                     10  // pin de l'ssr de la resistance



#define encoder0PinA            3  // pin A de l'encodeur
#define encoder0PinB            2  // pin B de l'encodeur
#define encoder0Click           0  // click de l'encodeur

#define thermo_sck_pin          A0 // pin sck de la sonde
#define thermo_cs_pin           A1 // pin cs de la sonde
#define thermo_so_pin           A2 // pin so de la sonde
#define bip_pin                 A3
#define Pression_sensor         A4 // pin de pression_max
#define temp_sensor             A5 // pin de temp_max

//  parametres du pid
#define consKp                  20
#define consKi                  0.05
#define consKd                  0.1

int encoder_position    =       0;
int encoder0PinALast    =       LOW;
int MyClick             =       HIGH;

int temp_w              =       HIGH;
int Pression_w          =       HIGH;

int i                   =       0;
int f                   =       0;
int b                   =       0;

double temperature      =       0;
int temps_chauf_cart    =       0;
int temps_maintien_inj  =       0;


int flag                =       0;
int t_flag              =       0;
int p_flag              =       0;

double Setpoint, Input, Output;

int WindowSize          =       255;
int n                   =       LOW;
int cls                 =       250;
unsigned long windowStartTime;
int val;

#define matiralsize             4
char myChar             =       48;
char matiere [20]       =       " ";

String materialName[matiralsize] = {"Resine A", "Resine B vertex", "Resine C", "Tcs flex-semi,dur"};  // les noms de matieres
int materialSetting[matiralsize][3] = {{290, 15, 1}, {285, 15, 1}, {280, 15, 1}, {275, 15, 5} };       // {Temperature,Durée}// les parametres
int selectedMaterial    =       0;

// pins de lcd (RS,E,D4,D5,D6,D7)
LiquidCrystal lcd(12, 11, 4, 5, 6, 7);
MAX6675 sonde_typeK(thermo_sck_pin, thermo_cs_pin, thermo_so_pin); // de la sonde

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);

// setup the pinmode and lcd
void setup() {
        // set up the LCD's number of columns and rows:
        lcd.begin(20, 4);
        lcd.createChar(0, first_full);
        lcd.createChar(1, last_full);
        lcd.createChar(2,  Inside_full);
        lcd.createChar(3, first_empty);
        lcd.createChar(4, last_empty);
        lcd.createChar(5, inside_empty);

        pinMode(Relay1, OUTPUT);
        pinMode(Relay2, OUTPUT);
        pinMode (encoder0PinA, INPUT);
        pinMode (bip_pin, OUTPUT);
        pinMode (encoder0PinB, INPUT);
        pinMode (encoder0Click, INPUT);
        pinMode(SSR, OUTPUT);
        pinMode(temp_sensor, INPUT_PULLUP);
        pinMode(Pression_sensor, INPUT_PULLUP);

        digitalWrite (bip_pin, LOW );
        digitalWrite(encoder0PinB, HIGH);
        digitalWrite(encoder0PinA, HIGH);
        digitalWrite(encoder0Click, HIGH);
        digitalWrite (Relay2, RELAY2_OFF );
        digitalWrite (Relay1, RELAY1_OFF );

        Setpoint = materialSetting[selectedMaterial][0];
        myPID.SetOutputLimits(0, WindowSize);
        myPID.SetMode(AUTOMATIC);
        lcd.clear();
}

//fonction du curseur

int getVal(int first, int last) {
      if (encoder_position >= last) {
      encoder_position = last;
                        }
      if (encoder_position <= first) {
       encoder_position = first;
                        }
                                          
  
        
        delay(10);
        int x = digitalRead(encoder0PinA);
        if ((encoder0PinALast == LOW) && (x == HIGH)) {
                if (digitalRead(encoder0PinB) == LOW) {
                        encoder_position--;
                        if (encoder_position <= first) {
                                encoder_position = first;
                        }
                        
                } else {
                        encoder_position++;
                        if (encoder_position >= last) {
                                encoder_position = last;
                        }
                        if (encoder_position <= first) {
                                encoder_position = first;
                        }
                        
                }

        }
        encoder0PinALast = x;
        return encoder_position;
}




int getPosition(int first, int last) {
        delay(10);
        int x = digitalRead(encoder0PinA);
        if ((encoder0PinALast == LOW) && (x == HIGH)) {
                if (digitalRead(encoder0PinB) == LOW) {
                        encoder_position--;
                        if (encoder_position <= first) {
                                encoder_position = first;
                        }
                        setCurs(encoder_position);
                } else {
                        encoder_position++;
                        if (encoder_position >= last) {
                                encoder_position = last;
                        }
                        setCurs(encoder_position);
                }

        }
        encoder0PinALast = x;
        return encoder_position;
}

int setCurs(int p) {
        lcd.setCursor(0, 0);
        lcd.print(" ");
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.setCursor(0, 2);
        lcd.print(" ");
        lcd.setCursor(0, 3);
        lcd.print(" ");
        if (p > 3)
        {
                p = p - 4;
        }
        lcd.setCursor(0, p);
        lcd.print(">");
        return 0;
}

// liste des matieres
int  material_list(int k) {

        if (k == 4) {
                f = 1;
        }
        if ((k == 2)) {
                b = 1;
        }


        if ( k <= 3) {

                if ((k == 3) && (f == 1)) {
                        lcd.clear();
                        //lcd.setCursor(0, 3);
                        //lcd.print(">");
                        f = 0;
                }

                for (int i = 0; i < 4; i++) {

                        lcd.setCursor(1, i);
                        lcd.print(materialName[i]);

                }

        }

        if ( k > 3) {

                if ((k == 4) && (b == 1)) {
                        lcd.clear();
                        //lcd.setCursor(0, 0);
                        //lcd.print(">");
                        b = 0;
                }

                for (int i = 0; i < 4; i++) {


                        lcd.setCursor(1, i);
                        lcd.print(materialName[i + 4]);

                }

        }


        return 0;
}

// fonction pid
void pid (){
        Setpoint = materialSetting[selectedMaterial][0];
        unsigned long now = millis();
        get_temp();
        Input = temperature; // temperature ;
        myPID.Compute();
        analogWrite(SSR, Output);

}

// fonction alarme
void warnings () {

        Pression_w = digitalRead(Pression_sensor);
        temp_w = digitalRead(temp_sensor);

        if (Pression_w == LOW || temp_w == HIGH   )
        {
                lcd.clear();
                delay(250);
                lcd.setCursor(3, 0);
                lcd.print ("   ERREUR !");

                while (Pression_w == LOW || temp_w == HIGH) {
                        temp_w = digitalRead(temp_sensor);
                        Pression_w = digitalRead(Pression_sensor);

                        if (Pression_w == LOW) {
                                digitalWrite (Relay1, RELAY1_OFF );
                                digitalWrite (Relay2, RELAY2_OFF );
                                lcd.setCursor(2, 2);
                                lcd.print ("PRESSION FAIBLE ");
                                digitalWrite (bip_pin, HIGH );
                        }

                        if (temp_w == HIGH) {
                                digitalWrite (SSR, LOW );

                                lcd.setCursor(1, 2);
                                lcd.print ("TEMPERATURE ELEVEE");
                                digitalWrite (bip_pin, HIGH );
                        }
                }

                lcd.clear();

                digitalWrite (Relay2, RELAY2_OFF );
                digitalWrite (Relay1, RELAY1_ON );
                digitalWrite (bip_pin, LOW );
                MenuPrincipal();
        }

}

// beep init machine
void BEEP(int repeat) {
        for (int u = 0; u <= repeat; u++) {
                digitalWrite (bip_pin, LOW );
                analogWrite (bip_pin, 255 );
                delay(20);
                analogWrite (bip_pin, 150 );
                delay(200);
                analogWrite (bip_pin, 75 );
                delay(200);
                analogWrite (bip_pin, 50 );
                delay(400);
                digitalWrite (bip_pin, LOW );
        }
}

//fonction calcul temperature
void get_temp() {
        temperature = (sonde_typeK.readCelsius() / 10) + ((9 * temperature) / 10);
        delay(250);
}

// initialisation Machine LCD etc.
void initial_machine() {

        lcd.begin(20, 4);
        lcd.setCursor(2, 0);
        lcd.print("SOCIETE ENERGIE");
        lcd.setCursor(2, 1);
        lcd.print("MADE IN TUNISIA");
        lcd.setCursor(2, 2);
        lcd.print(_version);
        lcd.setCursor(8, 3);
        lcd.print(_year);
        delay(6000);
        lcd.clear();
        BEEP(2);
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("DEMARRAGE SYSTEME");
        lcd.setCursor(0, 2);
        lcd.write(byte(3));
        lcd.setCursor(19, 2);
        lcd.write(byte(4));
        

        for (i = 1; i <= 18; i++) {
                lcd.setCursor(i, 2);
                lcd.write(byte(5));
        }

        delay(100);
        lcd.setCursor(0, 2);
        lcd.write(byte(0));

        for (i = 1; i <= 18; i++) {
                lcd.setCursor(i, 2);
                lcd.write(byte(2));
                delay(20);
                get_temp();
        }

        lcd.setCursor(19, 3);
        lcd.write(byte(1));
        lcd.setCursor(19, 2);
        lcd.write(byte(1));
        digitalWrite (Relay2, RELAY2_OFF );
        digitalWrite (Relay1, RELAY1_ON );

}

// Menu principal de la machine
void MenuPrincipal() {
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print(_version);
        lcd.setCursor(1, 1);
        lcd.print("Mode Utilisateur"); // Menu 1
        lcd.setCursor(1, 2);
        lcd.print("Deplacement Piston");
        lcd.setCursor(0, 0);
        lcd.setCursor(1, 3);
        lcd.print("Info");
        lcd.setCursor(0, 1);
        lcd.print(">");
        encoder_position = 0;
        delay(cls);
        while (1) {
                digitalWrite (SSR, LOW );
                int n = getPosition(1, 3);
                warnings ();
                if ( digitalRead(encoder0Click) == LOW) {

                        switch (n) {
                        case 1:
                                Menu_Utilisateur();
                                break;
                        case 2:
                                Menu_Piston();
                                break;
                        case 3:
                                Menu_info();
                                break;
                        }
                        delay(cls);
                }

        }
}

void Menu_Utilisateur() {
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print(materialName[selectedMaterial]);
        lcd.setCursor(17, 0);
        lcd.print(materialSetting[selectedMaterial][0]);

        lcd.setCursor(1, 1);
        lcd.print("Choix Matriere"); //Menu 11
        lcd.setCursor(1, 2);
        lcd.print("Debut Cycle"); // Menu 12
        lcd.setCursor(1, 3);
        lcd.print("Retour");

        lcd.setCursor(0, 1);
        lcd.print(">");
        encoder_position = 1;
        delay(cls);

        while (1) {
                digitalWrite (SSR, LOW );
                int n = getPosition(0, 3);
                warnings ();

                if ( digitalRead(encoder0Click) == LOW) {

                        switch (n) {
                        case 0: Menu_Modif_Matiere();
                                 break;
                        case 1:
                                Menu_Choix_Matriere();
                                break;
                        case 2:
                                chauffage_machine();    // Debut du Cycle chauffage ---> Insertion cartouche ---> Injection
                                break;
                        case 3:
                                MenuPrincipal();
                                break;
                        }
                        delay(cls);
                }
        }
}


//Menu de Modif Matiere

void Menu_Modif_Matiere(){
        
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print(materialName[selectedMaterial]);
        lcd.setCursor(17, 0);
        lcd.print(materialSetting[selectedMaterial][0]);
        lcd.setCursor(0, 1);
        lcd.print(">");
        encoder_position = 1;
        delay(cls);
        digitalWrite (SSR, LOW );
        lcd.setCursor(1, 1);
        lcd.print("Modifier Temp"); //Menu 11
        lcd.setCursor(1, 2);
        lcd.print("Modifier Duree"); // Menu 12
        lcd.setCursor(1, 3);
        lcd.print("Retour");
        while (1) {
                digitalWrite (SSR, LOW );
                int n = getPosition(1, 3);
                warnings ();

                if ( digitalRead(encoder0Click) == LOW) {

                        switch (n) {
                        case 1:
                                Modification_Temp();
                                break;
                        case 2:
                                Modification_Duree();    // Debut du Cycle chauffage ---> Insertion cartouche ---> Injection
                                break;
                        case 3:
                                Menu_Utilisateur();
                                break;
                        }
                        delay(cls);
                }
        }
        

  
}

void Modification_Temp(){
        
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print(materialName[selectedMaterial]);
        lcd.setCursor(17, 0);
        lcd.print(materialSetting[selectedMaterial][0]);
        lcd.setCursor(0, 1);
        lcd.print(">");
        encoder_position = 1;
        delay(cls);
        digitalWrite (SSR, LOW );
        lcd.setCursor(1,1 );
        lcd.print("Choisir Temp"); //Menu 11
        lcd.setCursor(16,1 );
        lcd.print(materialSetting[selectedMaterial][0]); // Menu 12
        lcd.setCursor(1, 2);
        lcd.print("Retour");
        while (1) {
                digitalWrite (SSR, LOW );
                int n = getPosition(1, 2);
                warnings ();

                if ( digitalRead(encoder0Click) == LOW) {

                        switch (n) {
                        case 1:
                                modif_val_temp();
                                break;
                        case 2:
                                Menu_Modif_Matiere();    // Debut du Cycle chauffage ---> Insertion cartouche ---> Injection
                                break;
                        }
                        delay(cls);
                }
        }
        

  
}


void modif_val_temp(){
  lcd.setCursor(16,1 );
  lcd.print("   ");
  delay(cls);
  
  while (1){
        int x = getVal(200,450);
        warnings ();
        materialSetting[selectedMaterial][0] = x;
        lcd.setCursor(16,1 );   
        lcd.print(materialSetting[selectedMaterial][0]);
        if ( digitalRead(encoder0Click) == LOW) {
              Modification_Temp(); 
        }  


}
}

void modif_duree_chauffage(){
  lcd.setCursor(18,1 );
  lcd.print("  ");
  delay(cls);
  while (1){
        int x = getVal(0,50);
        warnings ();
        materialSetting[selectedMaterial][1] = x;
        lcd.setCursor(18,1 );
        lcd.print(materialSetting[selectedMaterial][1]);
        lcd.print(" ");
        if ( digitalRead(encoder0Click) == LOW) {
              Modification_Duree(); 
        }  


}
}


void modif_duree_maintien(){
  lcd.setCursor(18,2 );
  lcd.print("  ");
  
  delay(cls);
  while (1){
        int x = getVal(0,50);
        warnings ();
        materialSetting[selectedMaterial][2] = x;
        lcd.setCursor(18,2 );
        lcd.print(materialSetting[selectedMaterial][2]);
        lcd.print(" ");
        if ( digitalRead(encoder0Click) == LOW) {
              Modification_Duree(); 
        }  


}
}

void Modification_Duree(){
        
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print(materialName[selectedMaterial]);
        lcd.setCursor(10, 0);
        lcd.print(materialSetting[selectedMaterial][1]);
        lcd.setCursor(17, 0);
        lcd.print(materialSetting[selectedMaterial][2]);
        lcd.setCursor(0, 1);
        lcd.print(">");
        encoder_position = 1;
        delay(cls);
        digitalWrite (SSR, LOW );
        lcd.setCursor(1,1 );
        lcd.print("Duree chauffage"); //Menu 11
        lcd.setCursor(18,1 );
        lcd.print(materialSetting[selectedMaterial][1]); // Menu 12
        lcd.setCursor(1, 2);
        lcd.print("Duree maintien"); //Menu 11
        lcd.setCursor(18,2 );
        lcd.print(materialSetting[selectedMaterial][2]); // Menu 12
        lcd.setCursor(1, 3);
        lcd.print("Retour"); //Menu 11
        
        while (1) {
                digitalWrite (SSR, LOW );
                int n = getPosition(1, 3);
                warnings ();

                if ( digitalRead(encoder0Click) == LOW) {

                        switch (n) {
                        case 1:
                              modif_duree_chauffage();
                                 
                                
                                break;
                        case 2:
                              modif_duree_maintien();
                        
                                break;
                         case 3:

                                Menu_Modif_Matiere(); 
                                break;}


      
                                
                        delay(cls);
                }
        }
        

  
}





// Menu de Choix de la Matriere
void Menu_Choix_Matriere() {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(">");
        encoder_position = 0;
        delay(cls);
        digitalWrite (SSR, LOW );
        // print all material on lcd
        for (int i = 0; i < 4; i++) {

                lcd.setCursor(1, i);
                lcd.print(materialName[i]);

        }

        while (1) {


                int n = getPosition(0, matiralsize-1);
                warnings ();
                //material_list(n);
                selectedMaterial = n;
                Setpoint = materialSetting[selectedMaterial][0];
                if ( digitalRead(encoder0Click) == LOW) {
                        if ( materialName[selectedMaterial] == "EMPLACEMENT VIDE") {
                                lcd.clear();
                                lcd.setCursor(3, 0);
                                lcd.print("AUCUNE MATIERE");
                                delay(1000);
                                Menu_Choix_Matriere();
                        }

                        Menu_Utilisateur();

                }
        }
}

// Menu de Choix de la Matriere
void Menu_Piston() {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(">");
        encoder_position = 1;
        delay(cls);
        digitalWrite (SSR, LOW );
        // print all material on lcd
        lcd.setCursor(1,1);
        lcd.print("Piston Haut");
        lcd.setCursor(1,2);
        lcd.print("Piston Bas");
        lcd.setCursor(1,3);
        lcd.print("Retour");

        while (1) {

                int n = getPosition(0,3);
                //warnings ();

                if ( digitalRead(encoder0Click) == LOW && n==1 ) {

                        digitalWrite (Relay1, RELAY1_OFF );
                        digitalWrite (Relay2, RELAY2_ON );

                }
                else if ( digitalRead(encoder0Click) == LOW && n==2 ) {
                        digitalWrite (Relay1, RELAY1_ON );
                        digitalWrite (Relay2, RELAY2_OFF );

                }
                else if ( digitalRead(encoder0Click) == LOW && n==3 ) {

                        delay(cls);
                        MenuPrincipal();

                }
        }
}

// Menu Info Machine
void Menu_info() {

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SOCIETE ENERGIE");
        lcd.setCursor(0, 1);
        lcd.print(_version);
        lcd.setCursor(0, 2);
        lcd.print("Email:info@cnc.tn");
        lcd.setCursor(0, 3);
        lcd.print("Tel:216/75 676 988");
        delay(cls);
        while (1) {
                digitalWrite (SSR, LOW );
                if (digitalRead(encoder0Click) == LOW) {
                        delay(cls);
                        MenuPrincipal();
                }

        }

}

// fonction chauffage
void chauffage_machine (){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(materialName[selectedMaterial]);
        lcd.setCursor(17, 0);
        lcd.print(materialSetting[selectedMaterial][0]);
        get_temp();
        while ( temperature < Setpoint )
        {
                warnings();
                pid();

                if ( digitalRead(encoder0Click) == LOW &&  digitalRead(encoder0PinA) == LOW) {

                        lcd.clear();
                        delay(250);
                        Menu_Utilisateur();
                }

                lcd.setCursor(0, 1);
                lcd.print("Temperature:");
                lcd.setCursor(15, 1);
                lcd.print(temperature);
                lcd.setCursor(19, 1);
                lcd.print("C");
                lcd.print("Min");
                lcd.setCursor(0, 3);
                lcd.print("Phase:");
                lcd.setCursor(8, 3);
                lcd.print("chauffage");
                pid();

        }
        inser_inj_cartouche();
}

// fonction insersion et injection
void inser_inj_cartouche(){
        double temps_actuel = 0;
        temps_chauf_cart = (materialSetting[selectedMaterial][1]*10 ); // 1 *60 /500 = 120 (250ms pour le pid /get_temp) --->  500ms * 120 = 1Min
        temps_maintien_inj = (materialSetting[selectedMaterial][2]*10 ); //(250ms pour le pid /get_temp) --->  500ms * 120 = 1Min

        // Insertion Cartouche ==> attente jusqu'a apuis boutton Encodeur
        lcd.clear();
        lcd.setCursor(1, 3);
        lcd.print("Inserer Cartouche");
        BEEP(5);

        while (digitalRead(encoder0Click) == HIGH) {

                pid();
                warnings();
                lcd.setCursor(0, 1);
                lcd.print("Temperature:");
                lcd.setCursor(15, 1);
                lcd.print(temperature);
                lcd.setCursor(19, 1);
                lcd.print("C");
                lcd.setCursor(0, 2);
                lcd.print("         ");
                lcd.setCursor(17, 2);
                lcd.print("   ");
                lcd.setCursor(0, 3);
                lcd.print("       ");
                lcd.setCursor(1, 3);
                lcd.print("Inserer Cartouche");
                pid();


        }

        // voir si il y'un problem (Pression/Temperature)
        get_temp();         //250 ms
        warnings();

        // Chauffage de la cartouche ==>> attente jusqu' atendre le temps desirée
        temps_actuel = 0;
        lcd.clear();
        int temps_rest = 0;
        while (temps_actuel <= temps_chauf_cart) {

                pid();        //250 ms
                lcd.setCursor(0, 1);
                lcd.print("Temperature:");
                lcd.setCursor(15, 1);
                lcd.print(temperature);
                lcd.setCursor(19, 1);
                lcd.print("C");
                lcd.setCursor(0, 2);
                lcd.print("Temps:");
                lcd.setCursor(10, 2);
                temps_rest = (temps_actuel/temps_chauf_cart)*100;
                lcd.print(temps_rest);
                lcd.setCursor(15,2);
                lcd.print("%");
                lcd.setCursor(0, 3);
                lcd.print("Chauffage Cartouche");
                pid();        //250 ms
                temps_actuel += 1;

        }

        // voir si il y'un problem (Pression/Temperature)
        get_temp();         //250 ms
        warnings();

        // Injection de la matiere ==>> attente jusqu'a ateindre le temps desirée
        temps_actuel = 0;
        temps_rest = 0;
        lcd.clear();
        while (temps_actuel <= temps_maintien_inj) {

                get_temp();                // 250ms
                lcd.setCursor(0, 1);
                lcd.print("Temperature:");
                lcd.setCursor(15, 1);
                lcd.print(temperature);
                lcd.setCursor(18, 1);
                lcd.print("C ");
                lcd.setCursor(0, 2);
                lcd.print("temps:");
                lcd.setCursor(10, 2);
                temps_rest = (temps_actuel/temps_maintien_inj)*100;
                lcd.print(temps_rest);
                lcd.setCursor(15,2);
                lcd.print("%");
                lcd.setCursor(0, 3);
                lcd.print("Injection en cours");

                digitalWrite (Relay2, RELAY2_ON );
                digitalWrite (Relay1, RELAY1_OFF );
                digitalWrite (SSR, LOW );
                temps_actuel += 1;
                delay(250);                  // 250ms

        }
        digitalWrite (Relay2, RELAY2_OFF );
        digitalWrite (Relay1, RELAY1_ON);
        lcd.clear();
        lcd.setCursor(5,2);
        lcd.print("FIN Programme");
        BEEP(5);                           // Beep fin du cycle
        Menu_Utilisateur();
}


// Main function
void loop() {
        initial_machine();
        MenuPrincipal();

}

