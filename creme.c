#define _GNU_SOURCE
#include "creme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <readline/readline.h>

/* Variables globales partagées */
struct elt *ma_liste = NULL;
pthread_mutex_t mutex_liste = PTHREAD_MUTEX_INITIALIZER;
int serveur_actif = 0;
int sockfd_udp; // Définition globale pour biceps.c

/* --- DÉTECTION RÉSEAU (Etape 2.1) --- */
void diffuser_presence(int sock, char *pseudo) {
    struct ifaddrs *ifaddr, *ifa;
    char bcast[NI_MAXHOST];
    char msg[100];

    msg[0] = '1'; 
    sprintf(&msg[1], "BEUIP%s", pseudo);

    if (getifaddrs(&ifaddr) == -1) return;

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET) continue;
        
        if (getnameinfo(ifa->ifa_broadaddr, sizeof(struct sockaddr_in),
                        bcast, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0) {
            if (strcmp(bcast, "127.0.0.1") != 0) {
                struct sockaddr_in dest;
                memset(&dest, 0, sizeof(dest));
                dest.sin_family = AF_INET;
                dest.sin_port = htons(PORT_BEUIP);
                dest.sin_addr.s_addr = inet_addr(bcast);
                sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&dest, sizeof(dest));
            }
        }
    }
    freeifaddrs(ifaddr);
}

/* --- GESTION DE LA LISTE (Etape 2.2) --- */
void ajouteElt(char *pseudo, char *adip) {
    pthread_mutex_lock(&mutex_liste);
    struct elt **curr = &ma_liste;
    while (*curr && strcmp((*curr)->nom, pseudo) < 0) curr = &((*curr)->next);
    if (*curr && strcmp((*curr)->nom, pseudo) == 0) {
        pthread_mutex_unlock(&mutex_liste);
        return;
    }
    struct elt *nouveau = malloc(sizeof(struct elt));
    strncpy(nouveau->nom, pseudo, LPSEUDO);
    strncpy(nouveau->adip, adip, 15);
    nouveau->next = *curr;
    *curr = nouveau;
    pthread_mutex_unlock(&mutex_liste);
}

void supprimeElt(char *adip) {
    pthread_mutex_lock(&mutex_liste);
    struct elt **curr = &ma_liste;
    while (*curr) {
        if (strcmp((*curr)->adip, adip) == 0) {
            struct elt *tmp = *curr;
            *curr = (*curr)->next;
            free(tmp);
            break;
        }
        curr = &((*curr)->next);
    }
    pthread_mutex_unlock(&mutex_liste);
}

void listeElts(void) {
    pthread_mutex_lock(&mutex_liste);
    struct elt *curr = ma_liste;
    printf("\n--- Annuaire BEUIP ---\n");
    if (!curr) printf("(Aucun utilisateur connecté)\n");
    while (curr) {
        printf("%-15s [%s]\n", curr->nom, curr->adip);
        curr = curr->next;
    }
    pthread_mutex_unlock(&mutex_liste);
}

/* --- SERVEUR TCP (Etape 3.1) --- */
void *serveur_tcp(void *p) {
    char *reppub = (char *)p;
    int sock, nsock;
    struct sockaddr_in sin;
    
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT_BEUIP);
    sin.sin_addr.s_addr = INADDR_ANY;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) pthread_exit(NULL);
    listen(sock, 5);

    while (serveur_actif) {
        nsock = accept(sock, NULL, NULL);
        if (nsock > 0) {
            envoiContenu(nsock, reppub);
            close(nsock);
        }
    }
    close(sock);
    return NULL;
}

void envoiContenu(int fd, char *reppub) {
    char type;
    if (read(fd, &type, 1) <= 0) return;

    if (type == 'L') {
        if (fork() == 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            execlp("ls", "ls", "-l", reppub, NULL);
            exit(1);
        }
        wait(NULL);
    } 
    else if (type == 'F') {
        char nomfic[100];
        int i = 0;
        while (read(fd, &nomfic[i], 1) > 0 && nomfic[i] != '\n') i++;
        nomfic[i] = '\0';

        if (fork() == 0) {
            char chemin[256];
            snprintf(chemin, sizeof(chemin), "%s/%s", reppub, nomfic);
            dup2(fd, STDOUT_FILENO);
            execlp("cat", "cat", chemin, NULL);
            exit(1);
        }
        wait(NULL);
    }
}

/* --- CLIENT TCP (Etape 3.2 & 3.3) --- */
char *chercherIP(char *pseudo) {
    pthread_mutex_lock(&mutex_liste);
    struct elt *curr = ma_liste;
    while (curr) {
        if (strcmp(curr->nom, pseudo) == 0) {
            char *res = strdup(curr->adip);
            pthread_mutex_unlock(&mutex_liste);
            return res;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&mutex_liste);
    return NULL;
}

void demandeListe(char *pseudo) {
    char *ip = chercherIP(pseudo);
    if (!ip) { printf("Pseudo inconnu.\n"); return; }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT_BEUIP);
    sin.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) == 0) {
        write(sock, "L", 1);
        char buf[1024];
        int n;
        while ((n = read(sock, buf, sizeof(buf))) > 0) write(STDOUT_FILENO, buf, n);
    }
    close(sock); free(ip);
}

void demandeFichier(char *pseudo, char *nomfic) {
    char *ip = chercherIP(pseudo);
    if (!ip) {
        fprintf(stderr, "Erreur : pseudo %s inconnu\n", pseudo);
        return;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT_BEUIP);
    sin.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("Erreur connexion TCP");
        free(ip);
        return;
    }

    dprintf(sock, "F%s\n", nomfic);

    char chemin[256];
    snprintf(chemin, sizeof(chemin), "pub/%s", nomfic);

    int fd = open(chemin, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Erreur création fichier local");
        close(sock); free(ip);
        return;
    }

    char buf[1024];
    int n, total_recu = 0;
    while ((n = read(sock, buf, sizeof(buf))) > 0) {
        write(fd, buf, n);
        total_recu += n;
    }

    if (total_recu > 0) printf("Fichier %s reçu (%d octets)\n", nomfic, total_recu);
    else unlink(chemin);

    close(fd); close(sock); free(ip);
}

/* --- SERVEUR UDP (Etape 1 & 2) --- */
void *serveur_udp(void *p) {
    char *mon_pseudo = (char *)p;
    struct sockaddr_in sin;
    
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT_BEUIP);
    sin.sin_addr.s_addr = INADDR_ANY;

    sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
    int bc = 1;
    setsockopt(sockfd_udp, SOL_SOCKET, SO_BROADCAST, &bc, sizeof(bc));
    bind(sockfd_udp, (struct sockaddr *)&sin, sizeof(sin));

    serveur_actif = 1;
    diffuser_presence(sockfd_udp, mon_pseudo);

    char buf[2048];
    while (serveur_actif) {
        struct sockaddr_in csin;
        socklen_t clen = sizeof(csin);
        int n = recvfrom(sockfd_udp, buf, sizeof(buf)-1, 0, (struct sockaddr *)&csin, &clen);
        if (n <= 0) continue;
        buf[n] = '\0';
        
        char code = buf[0];
        if (n > 6 && strncmp(&buf[1], "BEUIP", 5) == 0) {
            char *p_dist = &buf[6];
            char *ip_dist = inet_ntoa(csin.sin_addr);

            if (code == '1') {
                ajouteElt(p_dist, ip_dist);
                char ack[100]; ack[0] = '2';
                sprintf(&ack[1], "BEUIP%s", mon_pseudo);
                sendto(sockfd_udp, ack, strlen(ack), 0, (struct sockaddr *)&csin, clen);
            } 
            else if (code == '2') ajouteElt(p_dist, ip_dist);
            else if (code == '0') supprimeElt(ip_dist);
            else if (code == '9') {
                printf("\n[MSG de %s] %s\n", p_dist, &buf[11]);
                rl_on_new_line(); rl_redisplay();
            }
        }
    }
    close(sockfd_udp);
    return NULL;
}
