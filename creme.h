#ifndef CREME_H
#define CREME_H

#include <arpa/inet.h>
#include <pthread.h>

#define PORT_BEUIP 9998
#define LPSEUDO 23
#define ADDR_BCAST "192.168.88.255"

/* Structure de la liste chaînée (Etape 2.2) */
struct elt {
    char nom[LPSEUDO + 1];
    char adip[16];
    struct elt *next;
};

/* Globales partagées */
extern int sockfd_udp;
extern int sockfd_tcp;
extern int serveur_actif;

/* Prototypes obligatoires */
void *serveur_udp(void *p);
void *serveur_tcp(void *p);
void ajouteElt(char *pseudo, char *adip);
void supprimeElt(char *adip);
void listeElts(void);
void diffuser_presence(int sock, char *pseudo);

/* Fonctions de requêtes (Etape 3) */
void demandeListe(char *pseudo);
void demandeFichier(char *pseudo, char *nomfic);
void envoiContenu(int fd, char *reppub);

void viderListe(void); // Pour Valgrind
void commande(char octet1, char *message, char *pseudo);
char *chercherIP(char *pseudo);

#endif
