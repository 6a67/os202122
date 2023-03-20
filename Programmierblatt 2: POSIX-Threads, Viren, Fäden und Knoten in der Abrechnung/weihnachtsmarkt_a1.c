#include <stdio.h>


int konto = 3000;

// die Buchungen als Hilfsfunktion machen, weil der Prozess für alle Arbeitsgruppen gleich ist
void buchung(const char* name ,int delta) {
    int lkonto = konto;
    printf("%s: Kontostand vorher: %d\n", name, lkonto);
    lkonto += delta;
    printf("%s: Kontostand nachher: %d\n", name, lkonto);
    konto = lkonto;
}



void* verpflegung(void* arg) {
    for (int i = 0;i<10;i++) {
        buchung("Verpflegung", -400);
    }
    return NULL;
}

void* eisflaeche(void* arg) {
    for (int i = 0;i<10;i++) {
        buchung("Eisfläche", -400);
    }
    return NULL;
}


void* sonstiges(void* arg) {
    int* betrag = arg;
    for (int i = 0;i<10;i++) {
        buchung("Sonstiges", - *betrag);
    }
    return NULL;
}

void* finanzierung(void* arg) {
    for (int i = 0;i<5;i++) {
        buchung("Finanzierung", 2500);
    }
    return NULL;
}

int main(void) {
    verpflegung(NULL);
    eisflaeche(NULL);
    int betrag = 600;
    sonstiges(&betrag);
    finanzierung(NULL);
    printf("Endkontostand: %d\n", konto);
    
    return 0;
}
