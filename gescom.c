#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "gescom.h"

#define NBMAXC 10

/* Variables globales accessibles via le header */
char *Mots[MAX_MOTS];
int NMots;

/* Variables privées à la bibliothèque */
typedef struct {
    char *nom;
    int (*fonction)(int, char **);
} CommandeInterne;

static CommandeInterne TabComInt[NBMAXC];
static int NbComInt = 0;

char *copyString(char *s) {
    if (s == NULL) return NULL;
    char *copy = malloc(strlen(s) + 1);
    if (copy) strcpy(copy, s);
    return copy;
}

int analyseCom(char *b) {
    char *token;
    NMots = 0;
    while ((token = strsep(&b, " \t\n")) != NULL && NMots < MAX_MOTS - 1) {
        if (*token != '\0') {
            Mots[NMots] = copyString(token);
            NMots++;
        }
    }
    Mots[NMots] = NULL;
    return NMots;
}

void ajouteCom(char *nom, int (*f)(int, char **)) {
    if (NbComInt < NBMAXC) {
        TabComInt[NbComInt].nom = nom;
        TabComInt[NbComInt].fonction = f;
        NbComInt++;
    }
}

int execComInt(int N, char **P) {
    for (int i = 0; i < NbComInt; i++) {
        if (strcmp(P[0], TabComInt[i].nom) == 0) {
            return TabComInt[i].fonction(N, P);
        }
    }
    return 0;
}

int execComExt(char **P) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(P[0], P);
        perror("biceps");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    }
    return 0;
}

/* Implémentation des commandes internes */
int Sortie(int N, char *P[]) { 
    (void)N; (void)P; // Signale explicitement que les variables sont inutilisées
    exit(0); 
}

int ChangeDir(int N, char *P[]) {
    if (N > 1) chdir(P[1]);
    else (void)P; // Optionnel ici car P[1] est utilisé, mais par sécurité
    return 1;
}

int PrintWorkDir(int N, char *P[]) {
    (void)N; (void)P; // Correction ici pour supprimer l'erreur
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) printf("%s\n", cwd);
    return 1;
}

int Version(int N, char *P[]) {
    (void)N; (void)P; // Correction ici pour supprimer l'erreur
    printf("biceps version 1.0\n");
    return 1;
}

void majComInt(void) {
    ajouteCom("exit", Sortie);
    ajouteCom("cd", ChangeDir);
    ajouteCom("pwd", PrintWorkDir);
    ajouteCom("vers", Version);
}
