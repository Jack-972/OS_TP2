# TP2 OS - Projet BEUIP - Jacques ARNAULD & Adam BOUNOUR

NOM: ARNAULD
PRENOM: Jacques

NOM: BOUNOUR
PRENOM: Adam

## Présentation du Projet

Ce projet consiste en la création du protocole **BEUIP (BEUI over IP)**
et de la librairie **creme (Commandes Rapides pour l'Envoi de Messages
Evolués)**.\
Ces outils sont intégrés à l'interpréteur de commandes **biceps** pour
permettre une communication simplifiée entre étudiants sur un réseau
local.

Le fonctionnement s'inspire de **NetBEUI** : une annonce en broadcast
permet l'identification automatique des pairs, et une table de
correspondance *(IP/Pseudo)* permet l'envoi de messages ciblés.

------------------------------------------------------------------------

## Structure du Projet

-   **biceps.c** : Point d'entrée de l'application, gère les commandes
    utilisateur et le processus serveur.\
-   **creme.c / creme.h** : Coeur de la librairie réseau (gestion UDP,
    broadcast, table des utilisateurs).\
-   **gescom.c / gescom.h** : Gestionnaire des commandes internes de
    l'interpréteur.\
-   **Makefile** : Automatisation de la compilation et outils de
    vérification.

------------------------------------------------------------------------

## Installation et Compilation

Pour compiler le projet, utilisez la commande suivante dans votre
terminal :

``` bash
make
```

Pour activer les traces de débogage *(option -DTRACE)* :

``` bash
make CFLAGS="-Wall -Wextra -g -DTRACE"
```

------------------------------------------------------------------------

## Utilisation

### 1. Lancement de l'interpréteur

``` bash
./biceps
```

### 2. Commandes du protocole BEUIP

-   **beuip start \[pseudo\]** : Lance le serveur en tâche de fond.\
    Un message de broadcast est envoyé pour signaler votre présence aux
    autres.

-   **beuip stop** : Arrête proprement le serveur.\
    Un message de code `0` est envoyé en broadcast pour informer de
    votre déconnexion.

### 3. Commandes de messagerie (mess)

-   **mess list** : Affiche la liste des utilisateurs actuellement
    identifiés *(Pseudo et Adresse IP)*.\
-   **mess \[pseudo\] "\[message\]"** : Envoie un message privé à
    l'utilisateur spécifié.\
-   **mess all "\[message\]"** : Envoie le message à tous les
    utilisateurs enregistrés dans la table.

------------------------------------------------------------------------

## Détails Techniques

-   **Protocole** : UDP sur le port `9998`\
-   **Broadcast** : Utilisation de l'adresse `192.168.88.255` sur le
    réseau **TPOSUSER**\
-   **Sécurité** : Les commandes locales *(3, 4, 5)* sont filtrées pour
    n'accepter que les paquets provenant de `127.0.0.1`\
-   **Signaux** : Utilisation de **SIGUSR1** pour déclencher l'arrêt
    propre du processus fils

------------------------------------------------------------------------

## Vérification de la Librairie (Étape 3.1)

Conformément aux consignes, la table des symboles de **creme.o** peut
être vérifiée :

``` bash
nm creme.o
```

------------------------------------------------------------------------

## Auteurs

**Jacques ARNAULD & Adam BOUNOUR**\
**Date :** Mars 2026\
**Cours :** Programmation Système et Réseau
