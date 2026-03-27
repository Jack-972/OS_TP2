#ifndef GESCOM_H
#define GESCOM_H

#define MAX_MOTS 64

/* Variables exportées pour le main */
extern char *Mots[MAX_MOTS];
extern int NMots;

/* Prototypes des fonctions de la bibliothèque */
char *copyString(char *s);
int analyseCom(char *b);
void ajouteCom(char *nom, int (*f)(int, char **));
void majComInt(void);
int execComInt(int N, char **P);
int execComExt(char **P);

/* Prototypes des commandes internes */
int Sortie(int N, char *P[]);
int ChangeDir(int N, char *P[]);
int PrintWorkDir(int N, char *P[]);
int Version(int N, char *P[]);

#endif