# Source/PlagueTaleGO — Code C++

> Lire d'abord `/CLAUDE.md` (racine). Ce fichier couvre l'architecture et les normes du module `PlagueTaleGO`.

## Carte du module

```
PlagueTaleGO.h/.cpp     Module + catégorie de log LogPlagueTaleGO
PlagueTaleGO.Build.cs   Dépendances : Core, CoreUObject, Engine, InputCore, EnhancedInput, GameplayTags (+ UnrealEd en éditeur)
Board/                  Plateau : ABoard (grille, porte le layout, traits des liens), ABoardNode (case générée),
                        UBoardSubsystem (graphe, occupation, validation des déplacements, FindPath),
                        BoardTypes.h (FBoardLink, EBoardDirection), BoardTags.h (Gameplay Tags Board.Node.*)
Level/                  Données du Level Editor : FBoardLayout, définitions (Data Assets), réglages, validation.
                        Documenté dans /Source/PlagueTaleGOEditor/CLAUDE.md
Turns/                  Déroulé de la partie : UGameFlowSubsystem (chef d'orchestre), UGameFlowState (base),
                        GameFlowType.h (FPlayerIntent), States/ (un fichier par état)
Entities/               Pions : ABoardPawn (base abstraite), Player/APlayerPawn
Player/                 ABoardPlayerController (entrées souris → intentions). Ne pas confondre avec Entities/Player/
Core/                   ABoardGameMode (fait apparaître les pions, lance la partie)
Items/                  UBoardItem (base abstraite, vide pour l'instant)
```

Ajouter une dépendance de module : `PlagueTaleGO.Build.cs`, puis régénérer les fichiers projet.

## Architecture

### Démarrage d'une partie
1. `UBoardSubsystem::OnWorldBeginPlay` construit le graphe (avant les `BeginPlay` des acteurs) : cases `ABoardNode` + liens du layout. Il vérifie que la scène correspond au layout et relance la validation (résultats dans le log).
2. `ABoardGameMode::StartPlay` → `Super::StartPlay()` → `SpawnBoardEntities()` : pour chaque entité du layout, crée le pion (spawn différé), lui passe ses réglages (`ApplyPlacementSettings`), puis `SnapToNode` l'enregistre dans le Board.
3. `UGameFlowSubsystem::StartGame(PlayerPawn)` lance la machine à états.

### Un tour joueur
```
Clic → ABoardPlayerController (CanPlayerAct ?) → FPlayerIntent
     → UGameFlowSubsystem::SubmitPlayerIntent → état courant::HandleIntent
     → APlayerPawn::RequestMove → UBoardSubsystem::TryMovePawn (valide + met à jour l'occupation)
     → ABoardPawn::BeginMove (lerp) → OnMoveFinished → état::Finish()
     → UGameFlowSubsystem::DecideNextState
```

### State Machine (actuelle)
```
Start ──► ItemUse (si objet en attente) ──► PlayerMove
      └─► PlayerMove
PlayerMove ──► Victory    (si case de sortie)
           └─► EnemyTurn
EnemyTurn  ──► Defeat     (si joueur attrapé)
           └─► début du tour joueur (ItemUse ou PlayerMove)
Victory, Defeat : terminaux (DecideNextState renvoie nullptr)
```

## Invariants (ne pas casser)

1. **`UBoardSubsystem` est la seule autorité sur le plateau** : graphe, occupation, validation des déplacements. L'occupation ne change **que** dans `SetPawnNode`. Pour déplacer un pion : `TryMovePawn` (ou `SnapToNode` au spawn), jamais un simple `SetActorLocation`.
2. **Une case ne connaît pas ses voisines** : deux cases sont reliées **uniquement** si un lien explicite existe dans `ABoard::Layout.Links` (tracé dans le Level Editor). Un lien relie deux voisines orthogonales, pas de diagonale.
3. **`UGameFlowSubsystem` est le seul à changer d'état.** Toutes les transitions sont dans `DecideNextState`, nulle part ailleurs. Un état ne crée jamais d'autre état : il fait son travail puis appelle `Finish()`.
4. **Un état s'abonne aux délégués dans `Enter()` et s'en désabonne dans `Exit()`.**
5. **Le controller ne contient aucune logique de tour** : il vérifie `CanPlayerAct()`, construit un `FPlayerIntent` et le soumet. C'est l'état courant qui interprète l'intention.
6. **Un pion demande, le Board valide** : `RequestMove` → `TryMovePawn`. `BeginMove` suppose la case déjà validée.
7. **Un seul `ABoard` et un seul départ joueur par niveau.** Le pion joueur est identifié par `Cast<APlayerPawn>`.
8. **Le code éditeur (`ABoard`, `ABoardNode`, `Level/`) ne dépend pas des subsystems** : `UGameFlowSubsystem` n'existe qu'en Game/PIE, et le graphe de `UBoardSubsystem` n'est construit qu'au lancement de la partie.
9. **Le layout du `ABoard` est la source de vérité du plateau** et seul le Level Editor l'écrit. Les `ABoardNode` sont générées ; une règle de jeu liée à une case se teste par comportement (`HasBehavior(BoardTags::…)`), jamais par nom d'asset ni par classe. Voir les invariants du Level Editor dans `/Source/PlagueTaleGOEditor/CLAUDE.md`.

## Normes C++

**Nommage** : standard UE (`A`/`U`/`F`/`E`/`I`/`T`, booléens en `b`, PascalCase). Paramètre entrant en conflit avec un membre : préfixe `In` (`InTargetNode`). Enums : `UENUM(BlueprintType) enum class EXxx : uint8`.

**Fichiers** : un fichier par classe, nommé **sans le préfixe** (`BoardNode.h` pour `ABoardNode`). Les types partagés d'un domaine vont dans `<Domaine>Types.h` (`BoardTypes.h`, `GameFlowType.h`).

**Includes** :
- même dossier : `#include "Board.h"`
- autre dossier : `#include "PlagueTaleGO/Board/BoardNode.h"` (chemin depuis le module, jamais de `../`)
- dans les `.h`, déclarations anticipées (forward declarations) dès que possible ; les includes vont dans le `.cpp`. Le `.generated.h` est toujours le dernier include.

**UPROPERTY** :
- tout membre pointant vers un UObject : `UPROPERTY()` + `TObjectPtr<>` (sinon le garbage collector peut le détruire).
- `EditAnywhere` : données réglées par le LD sur une instance. `EditDefaultsOnly` : configuration de classe dans le BP (assets, inputs). `VisibleAnywhere` : valeur calculée. `VisibleInstanceOnly` : état d'exécution utile au debug.
- `meta = (ClampMin = "...")` sur les valeurs numériques réglables.
- Getters : `UFUNCTION(BlueprintPure)` + `const`, définis directement dans le `.h`.

**Catégories** (sans accents) : `Board`, `Board|Level` (données du layout), `Board|Grid`, `Board|Grid Preview`, `Board|Visuals`, `Board|Edition` (boutons `CallInEditor`), `GameFlow`, `Input`, `Camera`. Data Assets du Level Editor : `Level Editor` (présentation dans l'outil), `Node`, `Node|Visuals`, `Entity`, `Item`. Validation : `Validation`.

**Data Assets** : leurs propriétés sont `EditAnywhere` (un Data Asset est une instance : `EditDefaultsOnly` les rendrait non modifiables).

**Style** :
- tabulations, accolades sur leur propre ligne, accolades aussi pour les `if` d'une ligne.
- retours anticipés (early return) et vérification des pointeurs (`if (!X) { return; }`), y compris `GetWorld()`.
- Tick : `bCanEverTick = false` par défaut. Si un acteur en a besoin, `bStartWithTickEnabled = false` et activation seulement quand c'est nécessaire (cf. `ABoardPawn`).
- délégués : `DECLARE_DYNAMIC_MULTICAST_DELEGATE_*` nommés `FOn<Événement>`, exposés en `BlueprintAssignable`. Les fonctions qui y répondent sont des `UFUNCTION()` nommées `Handle<Événement>`.
- code éditeur dans `#if WITH_EDITOR` ; messages de debug à l'écran dans `#if !UE_BUILD_SHIPPING`.
- nouveau fichier : commencer directement par `#pragma once` / les includes (pas de texte « Fill out your copyright notice »).

**Logs** : toujours `UE_LOG(LogPlagueTaleGO, <Niveau>, TEXT("[Tag] message en français"))`.
- tags existants : `[Board]`, `[GameFlow]`, `[GameMode]`, `[Controller]`, `[PlayerMove]`, `[Validation]`.
- module éditeur : `UE_LOG(LogBoardEditor, …, TEXT("[BoardEditor] …"))` (`LogPlagueTaleGO` n'est pas exportée).
- `Error` : niveau mal construit ou situation impossible. `Warning` : anormal mais récupérable. `Log` : événement important (changement d'état, spawn). `Verbose` : détail de debug.

**Commentaires** : en français. Ils expliquent le *pourquoi* et les mécanismes UE non évidents. Chaque classe a un commentaire `/** */` qui décrit son rôle et ce dont elle est responsable. Dans les `.h`, les membres sont regroupés par sections `// --- Section ---`.

## Recettes

**Ajouter un état de partie**
1. Créer `Turns/States/XxxState.h/.cpp`, une classe `UXxxState : public UGameFlowState`.
2. Surcharger `Enter` / `Exit` / `HandleIntent` / `AllowsPlayerInput` selon le besoin. Appeler `Finish()` quand l'état a terminé (possible dès `Enter()`, l'enchaînement est géré).
3. Ajouter la transition dans `UGameFlowSubsystem::DecideNextState`, et nulle part ailleurs.
4. Mettre à jour le schéma de la machine à états et le registre ci-dessous.

**Ajouter un type de pion**
1. Créer une sous-classe de `ABoardPawn` dans `Entities/<Famille>/`.
2. Créer un BP enfant dans `Content/Blueprints/` (mesh, réglages).
3. Créer un Data Asset `BoardEntityType` (`DA_Entity_<Nom>`, `PawnClass` = le BP) : le pion devient plaçable dans le Level Editor. Pas de code de spawn spécifique dans le GameMode, sauf besoin réel.
4. Réglages propres à chaque placement (orientation déjà gérée) : recette « Ajouter des réglages à un type » de `/Source/PlagueTaleGOEditor/CLAUDE.md`.
5. Mettre à jour le registre ci-dessous.

**Ajouter un type de case, un comportement de case, une règle de validation** : voir `/Source/PlagueTaleGOEditor/CLAUDE.md`, § Recettes.

**Ajouter un objet** : le système d'objets n'est pas encore conçu. En discuter avec l'équipe avant d'implémenter.

## Registre des entités, objets et états

> ⚠️ **Agent : quand une entité, un objet ou un état est ajouté, modifié ou supprimé pendant la session, mettre ce registre à jour dans le même changement.** Les mécaniques ne sont pas figées : indiquer le statut (prototype, validé par les GD…).

État au 10/11/2026 :

**Pions**
| Classe | Fichier | BP | Statut |
|---|---|---|---|
| `ABoardPawn` | `Entities/ABoardPawn.h` | — | Base abstraite : occupation, déplacement en lerp (`MoveDuration`), `OnMoveFinished`, `ApplyPlacementSettings` (vide) |
| `APlayerPawn` | `Entities/Player/APlayerPawn.h` | `BP_PlayerPawn` | Vide : sert à identifier le joueur |

Aucun ennemi en C++. Des assets de prototype (`Sandbox/BP_WormTest`, `MI_Worm`, `M_SpawnWormNode`) existent sans classe C++ correspondante sur `develop`.

**Objets** : `UBoardItem` est une base abstraite vide. Aucun objet concret. Le layout peut contenir des objets (`FBoardCell::Items`, type `UBoardItemType`), mais rien ne les utilise en jeu : ramassage non implémenté, `PendingItem` jamais renseigné.

**Data Assets de définition** (types de cases / d'entités / d'objets) : aucun encore créé. À créer dans `Content/Data/Board/` (voir `/Source/PlagueTaleGOEditor/CLAUDE.md`).

**États**
| État | Statut |
|---|---|
| `UStartState` | Finit immédiatement (plus tard : intro, caméra) |
| `UPlayerMoveState` | Implémenté : attend un clic sur une case voisine, puis la fin du déplacement |
| `UItemUseState` | Stub : finit immédiatement |
| `UEnemyTurnState` | Stub : finit immédiatement (`IsPlayerCaught()` renvoie toujours `false`) |
| `UVictoryState` / `UDefeatState` | Terminaux : log + message à l'écran |
