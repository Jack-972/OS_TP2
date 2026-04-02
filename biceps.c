#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "creme.h"
#include "gescom.h"

pthread_t tid_udp, tid_tcp; // Deux threads : UDP et TCP [cite: 94]
char mon_pseudo[LPSEUDO + 1];
int beuip_running = 0;
char *reppub = "pub"; // Dossier pour le partage de fichiers [cite: 87]

int main() {
    majComInt();
    char *ligne;
    char prompt[100];
    sprintf(prompt, "%s@bicepsV3$ ", getenv("USER"));

    while ((ligne = readline(prompt)) != NULL) {
        if (strlen(ligne) > 0) {
            add_history(ligne);
            char *temp = strdup(ligne);
            if (analyseCom(temp) > 0) {
                
                // --- COMMANDES BEUIP ---
                if (strcmp(Mots[0], "beuip") == 0) {
                    // START : lance les deux serveurs [cite: 33, 93]
                    if (strcmp(Mots[1], "start") == 0 && NMots == 3) {
                        if (!beuip_running) {
                            strncpy(mon_pseudo, Mots[2], LPSEUDO);
                            pthread_create(&tid_udp, NULL, serveur_udp, (void *)mon_pseudo);
                            pthread_create(&tid_tcp, NULL, serveur_tcp, (void *)reppub);
                            beuip_running = 1;
                            printf("Services BEUIP (UDP et TCP) démarrés.\n");
                        }
                    } 
                    // STOP : arrête les deux threads proprement [cite: 48, 96]
                    else if (strcmp(Mots[1], "stop") == 0) {
                        if (beuip_running) {
                            extern int serveur_actif;
                            serveur_actif = 0;
                            
                            // Débloquer les sockets pour terminer les threads
                            extern int sockfd_udp;
                            shutdown(sockfd_udp, 2); 
                            
                            pthread_join(tid_udp, NULL);
                            pthread_join(tid_tcp, NULL);
                            
                            beuip_running = 0;
                            printf("Services BEUIP arrêtés.\n");
                        }
                    }
                    // LS : demande la liste des fichiers d'un pseudo [cite: 97]
                    else if (strcmp(Mots[1], "ls") == 0 && NMots == 3) {
                        if (beuip_running) demandeListe(Mots[2]);
                    }
                    // GET : télécharge un fichier d'un pseudo [cite: 110]
                    else if (strcmp(Mots[1], "get") == 0 && NMots == 4) {
                        if (beuip_running) demandeFichier(Mots[2], Mots[3]);
                    }
                }
                // --- COMMANDES MESS ---
                else if (strcmp(Mots[0], "mess") == 0) {
                    if (strcmp(Mots[1], "list") == 0) {
                        listeElts(); // Accès direct à la liste partagée [cite: 80]
                    }
                }
                // --- AUTRES COMMANDES ---
                else if (!execComInt(NMots, Mots)) {
                    execComExt(Mots);
                }
            }
            free(temp);
        }
        free(ligne);
    }
    return 0;
}
