# Agence de voyage de Cotonou — Gestion des vols et des réservations

Application console en **C++17 (programmation orientée objet)** permettant à une agence de voyage de Cotonou d'informatiser la gestion de ses passagers, de ses vols, de ses réservations de billets d'avion, des paiements et des annulations.

## Contenu du projet

| Fichier | Rôle |
|---|---|
| `main.cpp` | Code source complet de l'application |
| `diagramme.puml` | Diagramme de classes UML (PlantUML) |
| `scenario.txt` | Jeu de saisies utilisé pour la démonstration |
| `execution.pdf` | Résultat de l'exécution du scénario (captures) |
| `README.md` | Ce fichier |

## Fonctionnalités (menu principal)

1. **Enregistrer un passager** : nom, prénom, téléphone, e-mail (contrôlés)
2. **Ajouter un vol** : numéro, départ, destination, date, heure, capacité, prix
3. **Afficher les vols disponibles** : seuls les vols non complets sont listés
4. **Rechercher un vol** : par numéro, ville de départ, destination ou date
5. **Effectuer une réservation** : un passager + un vol
6. **Enregistrer un paiement** : paiement intégral du billet
7. **Annuler une réservation** : avec gestion du remboursement
8. **Consulter les réservations** : tableau récapitulatif
9. **Afficher les informations d'une réservation** : fiche détaillée
10. **Quitter**

## Règles de gestion appliquées

- Un passager doit être **enregistré avant** de pouvoir réserver.
- Un vol ne peut pas recevoir plus de réservations que sa **capacité**.
- Un passager ne peut pas réserver **deux fois le même vol** (tant que sa réservation n'est pas annulée).
- Une réservation est associée à **un seul passager et un seul vol**.
- Une réservation **annulée** ne peut plus être payée.
- Une place n'est **définitivement réservée qu'après validation** :
  - à la création, la réservation est `EN ATTENTE` et la place est retenue provisoirement ;
  - le paiement intégral la passe à `VALIDEE` (place définitive).
- Traitement du remboursement selon le statut :
  - annulation d'une réservation **non payée** → remboursement `Aucun` ;
  - annulation d'une réservation **payée** → `A rembourser`, puis `Rembourse` une fois le remboursement effectué (l'application le demande au moment de l'annulation).
- Toute annulation **libère la place** sur le vol.

## Principes de POO utilisés

- **Encapsulation** : tous les attributs sont `private`, accessibles via des accesseurs (`getXxx()`).
- **Constructeurs** : chaque classe possède un constructeur initialisant ses attributs (liste d'initialisation).
- **Associations** : `Reservation` référence un `Passager*` et un `Vol*` ; `Agence` possède les collections ; `Menu` utilise l'`Agence`.
- **Collections** : `std::vector<std::unique_ptr<...>>` pour les passagers, vols et réservations (les pointeurs restent valides quand le vecteur grandit).
- **Gestion des erreurs** : exceptions `ErreurMetier` pour les opérations interdites, validation de toutes les saisies (entiers, réels, dates `JJ/MM/AAAA`, heures `HH:MM`, téléphone, e-mail) avec redemande en boucle.

## Classes

`Passager`, `Vol`, `Reservation`, `Agence`, `Menu`, ainsi que les énumérations `StatutReservation` (`EN_ATTENTE`, `VALIDEE`, `ANNULEE`) et `StatutRemboursement` (`AUCUN`, `A_REMBOURSER`, `REMBOURSE`). Le détail est dans `diagramme.puml`.

## Compilation et exécution

Prérequis : un compilateur C++17 (g++ ≥ 7, clang, MinGW ou Visual Studio).

```bash
g++ -std=c++17 -Wall -Wextra -o agence main.cpp
./agence
```

Sous Windows (MinGW) :

```bat
g++ -std=c++17 -o agence.exe main.cpp
chcp 65001
agence.exe
```

(`chcp 65001` active l'UTF-8 dans la console pour afficher correctement les accents.)

Avec Visual Studio : créer un projet console vide, ajouter `main.cpp`, choisir le standard C++17, puis compiler.

### Rejouer la démonstration

```bash
./agence < scenario.txt
```

Le scénario couvre les 10 options du menu ainsi que les principaux cas d'erreur (saisies invalides, passager inconnu, vol complet, double réservation, paiement sur réservation annulée, etc.).

## Générer le diagramme UML

```bash
# avec PlantUML installé
plantuml diagramme.puml          # produit diagramme.png
```

Sans installation : coller le contenu de `diagramme.puml` sur <https://www.plantuml.com/plantuml> pour obtenir l'image.

## Limites et pistes d'amélioration

- Les données ne sont pas sauvegardées entre deux exécutions (une sauvegarde en fichier CSV pourrait être ajoutée).
- Le paiement est intégral (pas de paiement partiel).
- Une réservation annulée ne peut pas être réactivée : le passager doit refaire une réservation.
