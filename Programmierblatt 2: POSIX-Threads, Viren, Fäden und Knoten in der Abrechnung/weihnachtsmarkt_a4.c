#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


pthread_mutex_t konto_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t positiv = PTHREAD_COND_INITIALIZER; // diese contition wird signalisiert, wenn eine positive Transaktion gemacht wurde
int konto = 3000;

// die Buchungen als Hilfsfunktion machen, weil der Prozess für alle Arbeitsgruppen gleich ist
void buchung(const char* name ,int delta, int wait) {
    if (pthread_mutex_lock(&konto_lock)) {
        perror("konnte mutex nicht locken");
        exit(1);
    }
    if (delta < 0) { // abbuchung
        while (konto + delta < 0) { // auf höheren Kontostand warten
            printf("%s: warten auf höheren Kontostand\n", name);
            pthread_cond_wait(&positiv, &konto_lock);
        }
    }
    int lkonto = konto;
    printf("%s: Kontostand vorher: %d\n", name, lkonto);
    lkonto += delta;
    printf("%s: Kontostand nachher: %d\n", name, lkonto);
    konto = lkonto;
    if (delta > 0) {
        pthread_cond_broadcast(&positiv); // den anderen bescheid sagen, dass das konto aufgefüllt ist
    }
    if (pthread_mutex_unlock(&konto_lock)) {
        perror("konnte mutex nicht unlocken");
        exit(1);
    }
    sleep(wait);
}



void* verpflegung(void* arg) {
    for (int i = 0;i<10;i++) {
        buchung("Verpflegung", -400, 2);
    }
    return NULL;
}

void* eisflaeche(void* arg) {
    for (int i = 0;i<10;i++) {
        buchung("Eisfläche", -400, 3);
    }
    return NULL;
}


void* sonstiges(void* arg) {
    int* betrag = arg;
    for (int i = 0;i<10;i++) {
        buchung("Sonstiges", - *betrag, 3);
    }
    return NULL;
}

void* finanzierung(void* arg) {
    for (int i = 0;i<5;i++) {
        buchung("Finanzierung", 2500, 8);
    }
    return NULL;
}


int main(void) {
    
    int betrag = 600;
    
    pthread_t threads[4];
    if (pthread_create(&threads[0], NULL, verpflegung, NULL) != 0) {
        perror("konnte thread nicht erstellen");
        return 1;
    }
    if (pthread_create(&threads[1], NULL, eisflaeche, NULL) != 0) {
        perror("konnte thread nicht erstellen");
        return 1;
    }
    if (pthread_create(&threads[2], NULL, sonstiges, &betrag) != 0) {
        perror("konnte thread nicht erstellen");
        return 1;
    }
    if (pthread_create(&threads[3], NULL, finanzierung, NULL) != 0) {
        perror("konnte thread nicht erstellen");
        return 1;
    }
    
    for (int i = 0;i<4;i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("konnte nicht auf thread warten");
            return 1;
        }
    }
    
    if (pthread_mutex_destroy(&konto_lock) != 0) {
        perror("konnte mutex nicht zerstören");
        return 1;
    }
    
    if (pthread_cond_destroy(&positiv) != 0) {
        perror("konnte condition nicht zerstören");
        return 1;
    }
        
    
    printf("Endkontostand: %d\n", konto);
    
    return 0;
}




