#ifndef _BS_P4_VORGABE_H
#define _BS_P4_VORGABE_H

#include <stdint.h>
#include <stddef.h>

// Informationen zu einer Datei im Archiv
// Listenelement in einer fileInfoList
typedef struct fileInfo fileInfo;
struct fileInfo {
    char *path;      // Dateipfad
    uint64_t size;   // Dateigröße
    uint64_t offset; // Byte-Offset der Datei innerhalb des Archivs
    fileInfo *next;  // Zeiger auf das nächste Listenelement
};

// Liste für Elemente vom Typ fileInfo
typedef struct fileInfoList fileInfoList;
struct fileInfoList {
    fileInfo *head;  // Erstes Element
    fileInfo **tail; // Letztes ELement
};

// Initialisiert eine fileInfoList
// Muss vor der Benutzung der Liste einmal aufgerufen werden
void init(fileInfoList *list);

// Fügt ein Element an eine fileInfoList an
void enqueue(fileInfoList *list, fileInfo *item);

// Entfernt das erste Element aus der fileInfoList
fileInfo *dequeue(fileInfoList *list);

#endif