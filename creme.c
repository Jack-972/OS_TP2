#include "creme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Headers pour readline (Etape 3.4) */
#include <readline/readline.h>
#include <readline/history.h>

static int running = 1;

/* Table des couples (IP + pseudo) - Etape 2.2 */
User table_users[255];
int nb_users = 0;

/* Fonction pour ajouter un utilisateur sans doublon (Etape 2.2) */
void ajouter_utilisateur(struct in_addr ip, char *pseudo) {
    for (int i = 0; i < nb_users; i++) {
        if (table_users[i].ip.s_addr == ip.s_addr) return;
    }
    if (nb_users < 255) {
        table_users[nb_users].ip = ip;
        strncpy(table_users[nb_users].pseudo, pseudo, 49);
        nb_users++;
        #ifdef TRACE
        printf("[TRACE] Nouvel utilisateur identifié : %s (%s)\n", pseudo, inet_ntoa(ip));
        #endif
    }
}

void handle_stop(int sig) {
    (void)sig; 
    running = 0;
}

void lancer_serveur(char *pseudo) {
    int sockfd;
    struct sockaddr_in servaddr, bcaddr;
    int bc_enable = 1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) exit(1);

    /* Activation du mode Broadcast - Etape 2.2.2 */
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &bc_enable, sizeof(bc_enable));

    struct sigaction sa;
    sa.sa_handler = handle_stop;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT_BEUIP);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) exit(1);

    /* Envoi de l'annonce broadcast - Etape 2.2.2 */
    memset(&bcaddr, 0, sizeof(bcaddr));
    bcaddr.sin_family = AF_INET;
    bcaddr.sin_port = htons(PORT_BEUIP);
    bcaddr.sin_addr.s_addr = inet_addr(BC_ADDR);

    char msg_init[100];
    msg_init[0] = '1';
    sprintf(&msg_init[1], "BEUIP%s", pseudo);
    sendto(sockfd, msg_init, strlen(msg_init), 0, (struct sockaddr *)&bcaddr, sizeof(bcaddr));

    #ifdef TRACE
    printf("[TRACE] Serveur BEUIP démarré pour : %s\n", pseudo);
    #endif

    char buffer[2048];
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);

    while(running) {
        int n = recvfrom(sockfd, buffer, sizeof(buffer)-1, 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) continue;
        buffer[n] = '\0';

        char code = buffer[0];
        char *payload = &buffer[1];

        /* Gestion simplifiée des codes BEUIP (Etape 2.2 / 2.3) */
        if (n > 6 && strncmp(payload, "BEUIP", 5) == 0) {
            char *sender_pseudo = payload + 5;

            if (code == '1') { /* Identification */
                ajouter_utilisateur(cliaddr.sin_addr, sender_pseudo);
                char ack[100];
                ack[0] = '2';
                sprintf(&ack[1], "BEUIP%s", pseudo);
                sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr *)&cliaddr, len);
            } 
            else if (code == '2') { /* AR */
                ajouter_utilisateur(cliaddr.sin_addr, sender_pseudo);
            }
            else if (code == '9') { /* Message reçu */
                printf("\n[MSG de %s] %s\n", sender_pseudo, payload + 5);
                rl_on_new_line(); rl_redisplay(); 
            }
        }
        /* Gestion des commandes locales (Etape 2.3) */
        else if (strcmp(inet_ntoa(cliaddr.sin_addr), "127.0.0.1") == 0) {
            if (code == '3') { /* LISTE */
                printf("\n--- Liste des utilisateurs connectés ---\n");
                for(int i=0; i<nb_users; i++) {
                    printf("%s\t(%s)\n", table_users[i].pseudo, inet_ntoa(table_users[i].ip));
                }
                rl_on_new_line(); rl_redisplay();
            }
        }
    }

    /* Message '0' avant fermeture - Etape 2.3.4 */
    char msg_quit[100];
    msg_quit[0] = '0';
    sprintf(&msg_quit[1], "BEUIP%s", pseudo);
    sendto(sockfd, msg_quit, strlen(msg_quit), 0, (struct sockaddr *)&bcaddr, sizeof(bcaddr));

    close(sockfd);
    exit(0);
}

void stopper_serveur(pid_t pid_fils) {
    if (pid_fils > 0) kill(pid_fils, SIGUSR1);
}

void envoyer_commande_locale(char code, char *arg1, char *arg2) {
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[2048];
    int p_len = 0;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT_BEUIP);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    buffer[0] = code;
    if (code == '3') p_len = 1;
    else if (code == '4' && arg1 && arg2) {
        sprintf(&buffer[1], "%s", arg1);
        int offset = strlen(arg1) + 2;
        sprintf(&buffer[offset], "%s", arg2);
        p_len = offset + strlen(arg2);
    }
    else if (code == '5' && arg1) {
        sprintf(&buffer[1], "%s", arg1);
        p_len = strlen(arg1) + 1;
    }

    sendto(sockfd, buffer, p_len, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    close(sockfd);
}