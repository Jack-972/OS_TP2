#ifndef CREME_H
#define CREME_H

#include <arpa/inet.h>

#define PORT_BEUIP 9998
#define BC_ADDR "192.168.88.255"

// Définition du type User pour la table des couples (IP + Pseudo)
typedef struct {
    struct in_addr ip;
    char pseudo[50];
} User;

// Prototypes des fonctions de la librairie creme
void lancer_serveur(char *pseudo);
void stopper_serveur(pid_t pid_fils);
void envoyer_commande_locale(char code, char *arg1, char *arg2);

#endif