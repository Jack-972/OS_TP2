#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "creme.h"
#include "gescom.h"

/* Globales pour la gestion des threads */
pthread_t tid_udp, tid_tcp;
char mon_pseudo[LPSEUDO + 1];
int beuip_running = 0;
char *reppub = "pub"; 

int main() {
    majComInt();
    char *ligne;
    char prompt[100];
    sprintf(prompt, "%s@polytech$ ", getenv("USER"));

    while ((ligne = readline(prompt)) != NULL) {
        if (strlen(ligne) > 0) {
            add_history(ligne);
            char *temp = strdup(ligne);
            
            if (analyseCom(temp) > 0) {
                if (strcmp(Mots[0], "beuip") == 0 && NMots >= 2) {
                    
                    /* beuip start user [cite: 32, 33, 93] */
                    if (strcmp(Mots[1], "start") == 0 && NMots == 3) {
                        if (!beuip_running) {
                            strncpy(mon_pseudo, Mots[2], LPSEUDO);
                            pthread_create(&tid_udp, NULL, serveur_udp, (void *)mon_pseudo);
                            pthread_create(&tid_tcp, NULL, serveur_tcp, (void *)reppub);
                            beuip_running = 1;
                            printf("Services BEUIP démarrés.\n");
                        }
                    } 
                    
                    /* beuip stop [cite: 48, 96] */
                    else if (strcmp(Mots[1], "stop") == 0) {
                        if (beuip_running) {
                            extern int serveur_actif;
                            extern int sockfd_udp;
                            serveur_actif = 0;
                            
                            /* Débloquer le serveur UDP pour le join */
                            shutdown(sockfd_udp, 2); 
                            
                            pthread_join(tid_udp, NULL);
                            pthread_join(tid_tcp, NULL);
                            
                            beuip_running = 0;
                            printf("Services BEUIP arrêtés.\n");
                        }
                    }

                    /* beuip list [cite: 80] */
                    else if (strcmp(Mots[1], "list") == 0) {
                        if (beuip_running) {
                            listeElts();
                        }
                    }

                    /* beuip message <user> <message> [cite: 41, 42] */
                    else if (strcmp(Mots[1], "message") == 0 && NMots >= 4) {
                        if (beuip_running) {
                            /* Mots[2] est le destinataire, Mots[3] est le début du message */
                            if (strcmp(Mots[2], "all") == 0) {
                                commande('5', Mots[3], NULL);
                            } else {
                                commande('4', Mots[3], Mots[2]);
                            }
                        }
                    }
                    
                    /* Bonus : beuip ls et get */
                    else if (strcmp(Mots[1], "ls") == 0 && NMots == 3) {
                        if (beuip_running) demandeListe(Mots[2]);
                    }
                    else if (strcmp(Mots[1], "get") == 0 && NMots == 4) {
                        if (beuip_running) demandeFichier(Mots[2], Mots[3]);
                    }
                }
                else if (!execComInt(NMots, Mots)) {
                    execComExt(Mots);
                }

                /* Nettoyage mémoire de gescom pour Valgrind */
                for (int i = 0; i < NMots; i++) free(Mots[i]);
            }
            free(temp);
        }
        free(ligne);
    }

    /* Libération finale avant CTRL+D */
    if (beuip_running) {
        serveur_actif = 0;
        shutdown(sockfd_udp, 2);
        pthread_join(tid_udp, NULL);
        pthread_join(tid_tcp, NULL);
    }
    viderListe(); // Nettoie la liste chaînée

    return 0;
}
