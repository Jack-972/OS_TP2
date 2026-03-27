#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "gescom.h"
#include "creme.h"

// Stockage du PID du serveur pour pouvoir l'arrêter plus tard
pid_t pid_beuip = -1;

void handle_sigint(int sig) {
    printf("\nInterruption détectée ! Utilisez 'exit' pour quitter proprement.\n");
    rl_on_new_line();
    rl_redisplay();
}

int main() {
    // Gestion du CTRL+C pour l'interpréteur
    signal(SIGINT, handle_sigint);
    
    // Initialisation des commandes internes de Polytech 
    majComInt();

    char prompt[256];
    sprintf(prompt, "%s@polytech-sorbonne$ ", getenv("USER"));

    char *ligne;
    while ((ligne = readline(prompt)) != NULL) {
        if (strlen(ligne) > 0) {
            add_history(ligne);
            char *temp = strdup(ligne);
            
            if (analyseCom(temp) > 0) {
                // --- COMMANDE BEUIP START/STOP ---
                if (strcmp(Mots[0], "beuip") == 0 && NMots >= 2) {
                    if (strcmp(Mots[1], "start") == 0 && NMots == 3) {
                        pid_beuip = fork();
                        if (pid_beuip == 0) {
                            // Le fils exécute le serveur BEUIP 
                            lancer_serveur(Mots[2]);
                            exit(0);
                        }
                        printf("Protocole BEUIP démarré (PID fils: %d)\n", pid_beuip);
                    } 
                    else if (strcmp(Mots[1], "stop") == 0) {
                        if (pid_beuip > 0) {
                            stopper_serveur(pid_beuip); // Envoie le signal de fin
                            waitpid(pid_beuip, NULL, 0);
                            pid_beuip = -1;
                            printf("Protocole BEUIP arrêté.\n");
                        }
                    }
                }
                // --- COMMANDE MESS ---
                else if (strcmp(Mots[0], "mess") == 0 && NMots >= 2) {
                    if (strcmp(Mots[1], "list") == 0) {
                        envoyer_commande_locale('3', NULL, NULL);
                    } 
                    else if (NMots == 4) {
                        // Envoi à un pseudo spécifique
                        envoyer_commande_locale('4', Mots[2], Mots[3]);
                    }
                    else if (strcmp(Mots[1], "all") == 0 && NMots == 3) {
                        // Envoi à tout le monde
                        envoyer_commande_locale('5', Mots[2], NULL);
                    }
                }
                // --- COMMANDES STANDARDS ---
                else if (!execComInt(NMots, Mots)) {
                    execComExt(Mots);
                }

                // Libération de la mémoire de l'analyseur
                for (int i = 0; i < NMots; i++) free(Mots[i]);
            }
            free(temp);
        }
        free(ligne);
    }

    // Nettoyage avant de quitter
    if (pid_beuip > 0) stopper_serveur(pid_beuip);
    
    return 0;
}