# PlagueTaleGO — Guide pour les agents Claude

> Document d'équipe (3 Game Programmers). **Source de vérité : ce dépôt.**
> Une copie miroir est tenue dans le Project claude.ai « Plague Tale GO » (voir § Maintenance).

## Le projet

- **GO-like** (inspiré de Hitman GO) dans l'univers **A Plague Tale**. Projet d'école M1 (1er trimestre), équipe GP / GA / GD / LD.
- **Deadline : janvier 2027.**
- **Unreal Engine 5.8** (5.8.3), C++, un seul module runtime : `PlagueTaleGO`.
- **Cible : Desktop uniquement**, contrôle à la souris.
- L'équipe découvre UE et le C++ cette année : Commenter tout mécanisme UE non évident.
- Les **mécaniques ne sont pas figées**. Ne pas inventer de règle de jeu : en cas de doute sur une mécanique, **demander plutôt que supposer**.

## Travailler sur ce dépôt depuis l'app Claude

L'équipe utilise l'app Claude, pas Claude Code : les CLAUDE.md ne sont **pas chargés automatiquement**.

1. En début de tâche, lire ce fichier.
2. Avant de lire ou modifier du code dans un dossier, lire le CLAUDE.md le plus proche (voir Index).
3. Le dépôt est sur le PC du dev (dossier lié à la session) : modifier les fichiers sur place, jamais en les réécrivant de mémoire.
4. Les fichiers source sont en **CRLF** : conserver les fins de ligne et l'encodage existants.
5. Claude ne peut ni compiler ni lancer l'éditeur : à la fin d'une modification C++, dire ce qu'il faut vérifier (compilation, test en PIE, réglage d'un BP).
6. Ne jamais committer ni pousser sans demande explicite.

## Index des CLAUDE.md

| Fichier | Contenu |
|---|---|
| `/CLAUDE.md` | Ce fichier : projet, dépôt, assets, niveaux, git, glossaire |
| `/Source/PlagueTaleGO/CLAUDE.md` | Architecture C++, invariants, normes de code, recettes, registre des entités |

## Plan du dépôt

```
Source/PlagueTaleGO/    Code C++ du jeu (voir son CLAUDE.md)
Content/                Assets .uasset / .umap (binaires, Git LFS)
Config/                 Default*.ini : GameMode par défaut, map de démarrage, inputs
PlagueTaleGO.uproject
```

- **Ne jamais lire, modifier ni committer** : `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `.vs/`, `.idea/`, `*.sln`, `*.slnx` (générés).
- Les `.uasset` / `.umap` sont binaires : Claude ne peut ni les lire ni les modifier. Décrire les étapes à faire dans l'éditeur.
- Config actuelle : map de démarrage `Levels/NodeLevel`, GameMode par défaut `BP_BoardGameMode`.

## Qui touche à quoi

- **Devs (GP)** : tout le C++ et tous les Blueprints.
- **LD / GA** : uniquement la **construction des niveaux** (placement d'acteurs, réglages dans le panneau Details). Aucun scripting Blueprint attendu de leur part.
- Les Blueprints sont des **sous-classes minces** des classes C++ (mesh, matériaux, valeurs de réglage). La logique de jeu reste en C++.
- Conséquence : toute donnée qu'un LD doit régler est un `UPROPERTY(EditAnywhere)` commenté et rangé dans une catégorie claire (ex. `Board|Level`).

### Construire un niveau (état actuel)

1. Placer **un seul** `BP_Board` : sa position est l'origine de la grille, `CellSize` la taille d'une case.
2. Placer des cases (`ABoardNode` ou BP enfant) : elles se calent automatiquement sur la grille.
3. Sur chaque case, si besoin : `StartingEntity` (pion créé au démarrage ; **un seul** départ joueur), `bIsExit`, `Item`, `bIsBlocked`.
4. Bloquer un passage : bouton « Toggle Link Between Selected Nodes » du Board, avec exactement 2 cases sélectionnées.
5. Placer une `CameraActor` avec le tag `BoardCamera`.

Cette section est faire évoluer avec le développement destiné sur la branche `feature/LevelEditor`.

## Assets (Content/)

État actuel des conventions :

| Préfixe | Type | Dossier |
|---|---|---|
| `BP_` | Blueprint | `Blueprints/` (sous-dossier par domaine, ex. `Blueprints/Board/`) |
| `IA_` / `IMC_` | Input Action / Input Mapping Context | `Input/` |
| `M_` / `MI_` | Material / Material Instance | `Materials/` |
| `L_` | Niveaux (pas de préfixe pour l'instant) | `Levels/` |
| `SM_` | Static Meshes | `StaticMesh/` |

- Pour un nouveau type d'asset, suivre la convention UE usuelle (`SM_` Static Mesh, `T_` Texture, `WBP_` Widget, `NS_` Niagara…).
- `Sandbox/` : tests et prototypes, ne jamais les référencer depuis un niveau ou une classe de production.
- `Developers/` : dossiers personnels.
- Les assets binaires ne se fusionnent pas : prévenir l'équipe avant de modifier un BP ou un niveau partagé.

## Git

- `main` = stable, `develop` = intégration. Les PR visent `develop`.
- Branches : `feature/<sujet>`, `fix/<sujet>`, `docs/<sujet>` (les anciennes `feature-xxx` restent telles quelles).
- Commits au format **Conventional Commits**, description en français :
  `type(scope): description`
  - types : `feat`, `fix`, `refactor`, `docs`, `chore`
  - scopes : `board`, `gameflow`, `entities`, `player`, `items`, `core`, `content`, `config`, `docs`, ou tout autre assocaitions en rapport avec la feature développée.
  - ex. : `feat(gameflow): ajout de l'état EnemyTurn`
- Git LFS est configuré pour les assets (`.gitattributes`).

## Glossaire

| Terme (FR) | Code | Sens |
|---|---|---|
| Plateau / grille | `ABoard`, `UBoardSubsystem` | La grille du niveau et l'autorité qui la gère |
| Case | `ABoardNode` | Une position de la grille, repérée par `GridCoord` (`FIntPoint`) |
| Lien | `FBoardLink` | Passage entre deux cases voisines (4 directions, pas de diagonale) |
| Lien bloqué | `ABoard::BlockedLinks` | Passage fermé entre deux cases voisines |
| Case de sortie | `ABoardNode::bIsExit` | L'atteindre fait gagner |
| Pion / entité | `ABoardPawn` | Tout acteur qui occupe une case (joueur, futurs ennemis) |
| Intention | `FPlayerIntent` | Ce que le joueur veut faire (clic sur une case…), interprétée par l'état courant |
| État | `UGameFlowState` | Une étape de la partie (tour joueur, tour ennemi, victoire…) |
| Objet | `UBoardItem` | Objet posé sur une case (système pas encore conçu) |

## Maintenance de ces fichiers

- Si un changement rend un CLAUDE.md faux (nouveau dossier, nouvelle règle, nouvelle entité), le mettre à jour **dans le même commit**.
- Quand un dossier grossit ou a ses propres règles, proposer un CLAUDE.md dédié et l'ajouter à l'Index. Pistes : `Turns/`, `Entities/`, `Board/`, `Items/`, un futur `UI/`.
- Garder ces fichiers **courts** : règles et pointeurs, pas de documentation exhaustive.
- Après modification d'un CLAUDE.md, mettre à jour sa copie dans le Project claude.ai (même chemin, ex. `PlagueTaleGO/CLAUDE.md`) si le Project est attaché à la session.
