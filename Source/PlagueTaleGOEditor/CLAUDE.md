# Level Editor — Guide pour les agents Claude et les devs

> Lire d'abord `/CLAUDE.md` (racine) puis `/Source/PlagueTaleGO/CLAUDE.md` (normes C++).
> Ce fichier documente **tout le Level Editor** : ses données (module runtime, dossier `Level/`)
> et son outil (module éditeur `PlagueTaleGOEditor`, ce dossier).
> Branche de développement : `feature/LevelEditor`.
> **Guide utilisateur (LD / GA, sans code)** : document partagé « Guide du Level Editor — Plague Tale GO »
> (https://claude.ai/artifact/QJjZaKR7saJu2GxhgbjC9h). À tenir à jour quand un geste ou un écran de l'outil change.

## 1. Objectif et périmètre

Le Level Editor est **l'unique moyen de configurer le plateau d'un niveau** : cases, types de cases,
liens, entités de départ, objets.

- **Dans le périmètre** : tout ce qui décrit le plateau de jeu.
- **Hors périmètre** : décor, lumières, set dressing, caméra… restent libres dans l'éditeur UE standard.
- **Contrainte n°1 : évolutivité.** Les mécaniques (types de cases, d'ennemis, d'objets) ne sont pas
  figées. Ajouter, modifier ou supprimer un élément doit se faire **sans toucher au code de l'outil**
  (voir § 6, Recettes). Ne jamais coder en dur un type précis dans l'outil.

Workflow cible :
```
Édition d'un BROUILLON (copie du layout) ──► aperçu dynamique dans le viewport
        │                                    (cases fantômes, liens, erreurs surlignées)
        ▼
Validation (règles enfichables) ──► Erreur = Submit refusé
        │                            (sauf option « Ignore Errors », décochée par défaut)
        ▼
SUBMIT ──► ABoard::Layout (source de vérité) ──► génération des acteurs ABoardNode
        (une seule transaction : un Ctrl+Z annule tout le Submit)
```

### Gestes de l'outil « Board » (spécification validée le 11/11/2026)

Un **seul outil**, piloté par la souris et des touches de modification. Survoler une case affiche les actions
possibles (aide contextuelle) ; la section « Aide » du panneau les liste toutes.

| Geste | Case vide | Case existante | Lien | Étape |
|---|---|---|---|---|
| Clic gauche | Poser une case du type du pinceau (elle est sélectionnée) | La sélectionner : section « Selected Cell » (type, entités, objets, réglages) | — | 2a / 3 |
| Clic gauche + glisser | Tracer un chemin : chaque case traversée devient une case reliée à la précédente | Pareil, en partant de cette case | — | 2b |
| Ctrl + clic gauche (ou glisser) | — | Appliquer le type du pinceau aux noeuds ciblés | — | 2a (clic) / 2b (glisser) |
| Shift + clic gauche (sans bouger) | — | Effacer la case et ses liens | Effacer le lien (prioritaire sur la case) | 2a / 2b |
| Shift + clic gauche + glisser | Effacer les cases survolées et leurs liens | idem | — | 2b |
| Clic droit | **Aucune action de l'outil** : caméra uniquement (navigation UE habituelle, clic droit maintenu + ZQSD) | idem | idem | — |
| Ctrl+Z / Ctrl+Y | Annuler / rétablir dans le brouillon | | | 2a |
| Échap | Annuler le tracé en cours, sinon désélectionner la case | | | 2b / 3 |

Règles du tracé (2b) : passer sur une case existante la relie au chemin et continue (fermer une boucle, relier deux
cases voisines) ; si la souris saute une case ou passe en diagonale, le chemin est complété en angle droit.
**Légende de l'aperçu** : cercle = état de la case (vert valide, orange avertissement, rouge erreur) ; point central =
couleur du type (`Editor Color`) ; flèche = entité (sa direction) ; losange = objet ; croix / trait gris = supprimé au
Submit ; carré fin à la couleur du type = case modifiée, pas encore soumise (nouvelle, type, entités, objets, réglages
ou liens : `UBoardLayoutDraft::GetModifiedCells`) ; carré cyan = sélection ; jaune = sous la souris ; pendant Ctrl + glisser,
les cases touchées sont encadrées de la couleur du type appliqué. Si tous les types gardent la couleur blanche par défaut,
le point central ne change pas au changement de type : donner une `Editor Color` distincte à chaque type.
Visée d'un lien : près du centre d'une case (30 % de `CellSize`) on vise la case ; vers le bord, entre deux cases
reliées, on vise le lien (surligné en jaune). Shift + clic sans bouger efface la cible visée ; Shift + glisser n'efface que des cases. Un tracé entier (appui → relâchement) = une seule étape Ctrl+Z.
Le pinceau présélectionne le type coché **« Is Default Type »** (`UBoardNodeType::bIsDefaultType`).
**Caméra** : à l'entrée dans le mode, le viewport perspective passe à la verticale du plateau, cadré sur toutes les
cases (+X / Nord en haut de l'écran) ; la vue d'avant est rétablie à la sortie. Bouton « Top View » pour recadrer.
Quitter le mode (ou ouvrir un autre niveau) avec un brouillon non soumis : question Submit (Yes) / Discard (No).

## 2. État d'avancement

| Étape | Contenu | Statut |
|---|---|---|
| 1 | Modèle de données (layout, définitions, réglages), liens explicites en jeu, validation enfichable, module éditeur, synchronisation scène, migration, commandes console | ✅ Terminé et testé (11/11/2026, commit 2245e2d) |
| 2a | Mode « Board Editor » : `UEdMode`, brouillon (Ctrl+Z), aperçu dans le viewport, survol + aide contextuelle, clic (poser / Ctrl appliquer le type / Shift effacer), panneau minimal (pinceau, Soumettre, Annuler, Ignorer les erreurs, problèmes, aide), question à la sortie | ✅ Terminé et testé (11/11/2026) |
| 2b | Gestes glissés (tracer un chemin relié, Shift + glisser pour effacer, Ctrl + glisser pour peindre le type), Shift + clic sur un lien pour l'effacer, Échap, vue de dessus automatique + bouton « Top View » | ✅ Terminé et testé (12/11/2026) |
| 3 | Sélection d'une case + section « Selected Cell » (type, réglages, entités avec orientation et réglages, objets, problèmes de la case) ; description et couleur du type du pinceau ; bouton « Next Error » (sélectionne et centre la caméra) ; aperçu des entités (flèches) et objets (losanges) | ✅ Terminé et testé (12/11/2026) |
| 4a | Verrouillage léger (`ABoardNode` en `NotPlaceable`, position verrouillée à la génération), suppression de l'édition manuelle temporaire du `BP_Board`, guide utilisateur (document partagé) | ✅ Écrit (12/11/2026), à compiler et tester |
| 4b | Suppression du code de migration (§ 8.9) | Plus tard : quand toutes les branches seront fusionnées dans `develop` et leurs niveaux migrés |

> Agent : mettre ce tableau à jour à chaque étape terminée.

## 3. Architecture

### Fichiers

```
Source/PlagueTaleGO/                       (runtime : lu par le jeu ET par l'éditeur)
  Board/BoardTypes.h                       FBoardLink (lien explicite), EBoardDirection (+ ToYaw, ToGridOffset)
  Board/BoardTags.h/.cpp                   Gameplay Tags natifs : Board.Node.Exit, Board.Node.Blocked
  Board/Board.h/.cpp                       ABoard : porte FBoardLayout Layout (lecture seule), SetLayout (éditeur)
  Board/BoardNode.h/.cpp                   ABoardNode : acteur GÉNÉRÉ, visuel + comportements via son type
  Level/BoardLayout.h/.cpp                 FBoardLayout, FBoardCell, FBoardEntityPlacement, FBoardItemPlacement
  Level/BoardPlacementSettings.h/.cpp      UBoardPlacementSettings : base des réglages par placement
  Level/Definitions/                       Data Assets : UBoardDefinition (base), UBoardNodeType,
                                           UBoardEntityType, UBoardItemType
  Level/Validation/                        FBoardValidator, UBoardValidationRule, types de résultat
  Level/Validation/Rules/                  Une règle par fichier

Source/PlagueTaleGOEditor/                 (éditeur uniquement : jamais dans le jeu)
  PlagueTaleGOEditor.h/.cpp                Module + catégorie de log LogBoardEditor, tag [BoardEditor]
  Definitions/BoardDefinitionRegistry      Trouve tous les Data Assets de définition (Asset Registry)
  Sync/BoardLayoutSync                     Submit, synchronisation layout → acteurs, migration
  Sync/BoardAutoSync                       Resynchronisation automatique après modification du NodeClass d'un type
  Commands/BoardEditorCommands.cpp         Commandes console BoardEditor.*
  Mode/BoardEditorMode                     UBoardEditorMode (UEdMode) : brouillon, Submit / Annuler, question à la sortie,
                                           caméra, relance de l'outil s'il s'arrête
  Mode/BoardEditorModeToolkit              Panneau du mode (FModeToolkit standard, sans Slate)
  Mode/BoardEditorModeCommands             Boutons de la barre d'outils du mode (TCommands)
  Mode/BoardEditorInputProcessor.h         Reçoit Échap avant les commandes de l'éditeur (préprocesseur Slate)
  Mode/BoardLayoutDraft                    UBoardLayoutDraft : copie du layout en cours d'édition + validation
  Tools/BoardEditTool                      UBoardEditTool : gestes, survol, sélection, aperçu ; ses 3 jeux de propriétés
                                           = le panneau (pinceau / brouillon / vue, case sélectionnée, aide)
  Tools/BoardDraftChange.h                 Étape d'historique Ctrl+Z du brouillon (FToolCommandChange)
```

Includes dans le module éditeur : même convention que le runtime, chemin depuis `Source/`
(`#include "PlagueTaleGO/Board/Board.h"`, `#include "PlagueTaleGOEditor/Sync/BoardLayoutSync.h"`).

### Flux des données

```
Data Assets (Content/Data/Board/…)          FBoardLayout (dans ABoard, sauvegardé avec le .umap)
  UBoardNodeType   ◄──── référencés par ────  Cells[] : Coord, NodeType, NodeSettings, Entities[], Items[]
  UBoardEntityType ◄────────────────────────  Links  : TSet<FBoardLink>
  UBoardItemType   ◄────────────────────────
                                                  │ Submit (FBoardLayoutSync)
                                                  ▼
                                    Acteurs ABoardNode (un par cellule, dossier Outliner Board/Nodes)
                                                  │ en jeu
                                                  ▼
UBoardSubsystem (graphe = acteurs + Layout.Links)    ABoardGameMode (pions = Layout.Cells[].Entities)
```

## 4. Invariants du Level Editor (ne pas casser)

1. **Le layout (`ABoard::Layout`) est la source de vérité du plateau.** Les acteurs `ABoardNode` en sont
   une représentation générée. On modifie le layout, puis on synchronise (`FBoardLayoutSync`) ; jamais l'inverse.
2. **Seul le module éditeur écrit le layout**, via `ABoard::SetLayout` (dans une transaction). Le jeu ne fait que le lire.
3. **Liens explicites** : deux cases sont reliées **uniquement** si `Layout.Links` contient leur lien.
   Un lien relie deux cases **existantes, distinctes, voisines orthogonales** (`FBoardLayout::CanLink`).
   Supprimer une case supprime ses liens (`RemoveCell`).
4. **Aucun type codé en dur dans l'outil.** La palette vient de `FBoardDefinitionRegistry`, les comportements
   de Gameplay Tags, les réglages de `SettingsClass`. Une règle de jeu se teste par tag (`HasBehavior`), jamais
   par nom d'asset.
5. **Les réglages instanciés appartiennent à leur propriétaire** : ceux du layout du Board ont le Board pour
   `Outer`. Après une copie de layout vers un nouveau propriétaire, appeler `DeepCopySettings(NouveauPropriétaire)`
   (déjà fait par `ABoard::SetLayout`). Sinon brouillon et Board partageraient les mêmes objets.
6. **Le layout et la validation sont des données pures** : ni acteur, ni subsystem, ni `GEditor`. Ils doivent
   rester utilisables en jeu comme dans l'éditeur.
7. **Le runtime ne dépend jamais du module éditeur.** Le code éditeur dans le runtime reste minimal et sous
   `#if WITH_EDITOR` / `#if WITH_EDITORONLY_DATA`.
8. **Toute modification de la scène par l'outil passe par une transaction** (`FScopedTransaction` + `Modify()`
   sur chaque objet modifié, y compris le `ULevel` quand on crée/détruit des acteurs).
9. **Tout texte affiché à l'utilisateur est en anglais** (`LOCTEXT`, `DisplayName`, `ToolTip`) : § 6 « Textes affichés ».
   Les logs, l'aide des commandes console et les commentaires restent en français.

## 5. Modèle de données

### Layout (`Level/BoardLayout.h`)

| Struct | Champs | Notes |
|---|---|---|
| `FBoardLayout` | `Cells`, `Links` | Requêtes : `FindCell`, `AddCell`, `RemoveCell`, `HasLink`, `CanLink`, `AddLink`, `RemoveLink`, `GetLinkedCoords`, `CountLinks`, `GetReachableCoords` (BFS). Recherches linéaires : volontaire (quelques dizaines de cases). |
| `FBoardCell` | `Coord`, `NodeType`, `NodeSettings`, `Entities[]`, `Items[]` | Listes : plusieurs entités / objets possibles par case. |
| `FBoardEntityPlacement` | `Type`, `Facing`, `Settings` | `Facing` (`EBoardDirection`) donne le Yaw du pion au spawn. |
| `FBoardItemPlacement` | `Type`, `Settings` | Le système d'objets n'est pas conçu : stocké mais ignoré en jeu. |

### Définitions (`Level/Definitions/`, Data Assets)

| Classe | Champs propres | Utilisé par |
|---|---|---|
| `UBoardDefinition` (base) | `DisplayName`, `Description` (aide), `EditorColor`, `Icon`, `SortOrder`, `SettingsClass` | Palette du Level Editor |
| `UBoardNodeType` | `Behaviors` (tags `Board.Node.*`), `NodeClass`, `Mesh`, `MaterialOverrides`, `MeshScale` | `ABoardNode` (visuel, `HasBehavior`) |
| `UBoardEntityType` | `PawnClass` ; `IsPlayer()` si dérivé de `APlayerPawn` | `ABoardGameMode` (spawn) |
| `UBoardItemType` | `ItemClass` | (futur système d'objets) |

Convention d'assets proposée : `Content/Data/Board/NodeTypes/DA_Node_<Nom>`, `.../Entities/DA_Entity_<Nom>`,
`.../Items/DA_Item_<Nom>`. Les assets de `Content/Sandbox/` sont **ignorés** par le Level Editor.

### Comportements de cases (Gameplay Tags testés par le code)

> Agent : tenir ce tableau à jour. Un tag absent de ce tableau n'a aucun effet en jeu.

| Tag | Testé par | Effet |
|---|---|---|
| `Board.Node.Exit` | `ABoardNode::IsExit` → `UGameFlowSubsystem` (victoire) | Atteindre la case fait gagner |
| `Board.Node.Blocked` | `ABoardNode::IsBlocked` → `UBoardSubsystem::TryMovePawn`, `FindPath` | Case infranchissable |

`ABoardNode::SetBlocked` pose un blocage **temporaire en jeu** (non sauvegardé), indépendant du tag.

### Règles de validation (`Level/Validation/Rules/`)

> Agent : tenir ce tableau à jour.

| Règle | Gravité | Vérifie |
|---|---|---|
| `UIntegrityRule` | Erreur | Coordonnées uniques ; liens entre cases voisines existantes |
| `UMissingDefinitionRule` | Erreur | Chaque case / entité / objet a un type complet (asset supprimé, `PawnClass` vide…) |
| `UIsolatedNodeRule` | Erreur | Chaque case a au moins un lien |
| `UPlayerStartRule` | Erreur | Exactement un départ joueur |
| `UExitRule` | Erreur | Au moins une case `Board.Node.Exit` |
| `UConnectedGraphRule` | Erreur | Un seul ensemble de cases reliées (pas d'îlot) |
| `UExitReachableRule` | Avertissement | Une sortie est atteignable depuis le départ (liens + cases bloquées, sans les ennemis) |

Les erreurs bloquent le Submit, sauf si l'option « Ignore Errors » (étape 3, décochée par défaut) est cochée :
elle sert au développement (niveau de test incomplet). La validation est aussi relancée au lancement de la
partie (`UBoardSubsystem::OnWorldBeginPlay`) et ses résultats vont dans le log.

## 6. Recettes (faire évoluer l'outil)

**Ajouter un type de case** (aucun code)
1. Content Browser > clic droit > Miscellaneous > Data Asset > `BoardNodeType`. Nommer `DA_Node_<Nom>`.
2. Renseigner `DisplayName`, `Description` (aide affichée dans l'outil), `EditorColor`, le visuel et les `Behaviors`.
3. Il apparaît dans la palette. Modifier son visuel met à jour immédiatement toutes les cases de ce type
   (`UBoardNodeType::PostEditChangeProperty`). Changer `Node Class` recrée les cases concernées (`FBoardAutoSync`).

**Distinguer visuellement une case** (ex. case de départ, aucun code)
Le visuel vient du type : créer un type dédié, sans comportement, et l'attribuer à la case.
1. Matériau : dans `Content/Materials/`, un Material `M_Node` avec un paramètre *Vector Parameter* `Color`
   branché sur *Base Color* ; puis clic droit > *Create Material Instance* > `MI_Node_Start`, régler `Color`.
2. Dupliquer `DA_Node_Floor` en `DA_Node_Start` : `Display Name`, `Material Overrides` [0] = `MI_Node_Start`, `Behaviors` vide.
3. Attribuer `DA_Node_Start` à la case : mode « Board Editor », sélectionner la case, « Cell Type » (ou Ctrl + clic).
Le joueur n'est pas lié à ce type : il apparaît là où l'entité joueur est placée. Si le départ doit
toujours être coloré automatiquement, en discuter avec les GD (ce serait une règle à coder).

**Ajouter un comportement de case** (code)
1. Déclarer le tag dans `Board/BoardTags.h` (`UE_DECLARE_GAMEPLAY_TAG_EXTERN`) et le définir dans `BoardTags.cpp`
   (`UE_DEFINE_GAMEPLAY_TAG_COMMENT`, nom sous `Board.Node.`).
2. Le tester là où la règle de jeu s'applique : `Node->HasBehavior(BoardTags::Node_Xxx)`. Jamais de test sur un nom d'asset.
3. L'ajouter dans les `Behaviors` des types de cases concernés, puis dans le tableau du § 5.
4. Si le comportement impose une contrainte au niveau, ajouter une règle de validation.

**Ajouter un type d'entité**
1. Créer le pion (recette « Ajouter un type de pion » de `/Source/PlagueTaleGO/CLAUDE.md`).
2. Créer un Data Asset `BoardEntityType` (`DA_Entity_<Nom>`), `PawnClass` = le BP du pion.

**Ajouter des réglages à un type** (ex. patrouille d'un ennemi)
1. Créer une sous-classe C++ de `UBoardPlacementSettings` (un fichier, `UCLASS()` suffit : les spécificateurs
   `EditInlineNew`/`DefaultToInstanced` sont hérités). Chaque réglage : `UPROPERTY(EditAnywhere)` avec un commentaire
   en français (pour les programmeurs) et `meta = (ToolTip = "...")` en anglais (affiché dans l'outil), `ClampMin` si numérique.
2. Dans le Data Asset du type, choisir cette classe dans `SettingsClass`.
3. Côté jeu : surcharger `ABoardPawn::ApplyPlacementSettings` et faire un `Cast` vers la classe de réglages.
   Les réglages arrivent **avant** `BeginPlay`.
4. Dans le mode « Board Editor », les réglages sont créés automatiquement quand on choisit le type d'une case, d'une
   entité ou d'un objet (section « Selected Cell »). Les placements déjà soumis les reçoivent à leur prochaine édition ;
   en attendant, gérer `Settings == nullptr` (valeurs par défaut).

**Ajouter ou modifier un geste de l'outil « Board »**
1. Mettre à jour d'abord le tableau des gestes (§ 1) et le valider avec l'équipe.
2. Dans `UBoardEditTool` : l'entrée passe par un comportement ITF (`USingleClickInputBehavior`, `UMouseHoverBehavior`…,
   créés dans `Setup`). Toute modification du brouillon passe par `ApplyDraftEdit` (historique Ctrl+Z automatique).
3. Mettre à jour l'aide, en anglais : `HelpText` (panneau) et `UpdateHoverMessage` (aide contextuelle).

**Textes affichés (anglais uniquement)**
L'interface de l'outil est en anglais, quelle que soit la langue de l'éditeur (la traduction française a été abandonnée).
- `.cpp` : `LOCTEXT("Cle", "English text")` entre `#define LOCTEXT_NAMESPACE "..."` et `#undef LOCTEXT_NAMESPACE`
  (fin du fichier). Variables : `FText::Format(LOCTEXT("Cle", "{0} cell(s)"), FText::AsNumber(N))`.
  `LOCTEXT` reste la convention UE pour tout `FText` affiché, même sans traduction (une traduction resterait possible plus tard).
- Propriétés et boutons des panneaux et des Data Assets : `meta = (DisplayName = "...", ToolTip = "...")` et `Category`
  en anglais. Le commentaire `//` au-dessus reste en français, pour les programmeurs.
- Logs (`UE_LOG`), aide des commandes console, commentaires : en français.

**Ajouter une règle de validation**
1. Créer `Level/Validation/Rules/XxxRule.h/.cpp`, classe `UXxxRule : public UBoardValidationRule` (`UCLASS()`).
2. Dans le constructeur : `Severity` (Erreur bloque le Submit, Avertissement informe) et `Description`.
3. Surcharger `Validate` : lire le layout, appeler `Report(OutMessages, Texte, Cases)` pour chaque problème.
   Message pour un LD : quoi, où (`CoordToString`), comment corriger. Passer les cases concernées pour qu'elles soient surlignées.
   Texte en anglais : `FText::Format(LOCTEXT("Xxx_Message", "... {0} ..."), ...)`, entre `#define LOCTEXT_NAMESPACE "BoardValidation"`
   et `#undef LOCTEXT_NAMESPACE` (modèle : `IsolatedNodeRule.cpp`).
4. Rien à enregistrer : `FBoardValidator` trouve toutes les sous-classes. Mettre à jour le tableau du § 5.
5. Une règle ne doit **pas** répéter le message d'une autre (ex. `UExitReachableRule` se tait s'il n'y a pas
   exactement un départ joueur : `UPlayerStartRule` le signale déjà).

**Changer la gravité d'une règle** : une ligne dans son constructeur (`Severity = …`). Prévenir les GD/LD.

**Supprimer un type** : supprimer le Data Asset (Content Browser > Delete, l'éditeur propose de remplacer les
références). Les niveaux qui l'utilisaient échouent à la validation (`UMissingDefinitionRule`) jusqu'à correction.

## 7. Commandes console

À taper dans le champ « Cmd » en bas de l'éditeur (Tab complète le nom). Niveau ouvert dans l'éditeur uniquement (pas en PIE).

| Commande | Effet |
|---|---|
| `BoardEditor.Validate` | Valide le layout du niveau ouvert, résultat dans l'Output Log |
| `BoardEditor.Resync` | Régénère les acteurs `ABoardNode` depuis le layout. Rarement utile : la scène se resynchronise seule (`FBoardAutoSync`) |
| `BoardEditor.ImportLegacy [force]` | Migration d'un niveau construit à la main vers le layout (`force` : écrase un layout existant). Guide : § 8 |

## 8. Guide : premiers Data Assets et migration d'un niveau (temporaire)

Les niveaux construits avant le Level Editor (cases placées à la main, `StartingEntity`, `bIsExit`, liens
bloqués) se relisent grâce aux propriétés `_DEPRECATED` de `ABoard` et `ABoardNode`. La migration a besoin
de Data Assets pour savoir en quoi convertir chaque case et chaque pion. Pas à pas :

### 8.0 Prérequis

- Le projet compile et l'éditeur a été **relancé** après la compilation. Live Coding ne sait pas ajouter de
  nouvelles classes ni un nouveau module : fermer l'éditeur, compiler depuis Visual Studio / Rider, rouvrir.
- Vérifier que le module éditeur est chargé : Window > Output Log, chercher `[BoardEditor] Module éditeur chargé.`

### 8.1 Choisir le mesh des cases

Le visuel d'une case vient de son type : il faut un Static Mesh **avec collision** (c'est lui que le clic du joueur touche).

- Le plus simple : réutiliser le mesh des anciennes cases. Ouvrir le BP de case utilisé dans le niveau
  (ex. `Sandbox/BP_NodeTemp`), sélectionner son composant mesh, noter le Static Mesh dans le panneau Details.
- Ou un mesh du moteur : `/Engine/BasicShapes/Cylinder` (cylindre de 100 cm de diamètre, avec collision).
  Pour le voir dans les sélecteurs : icône ⚙ du sélecteur > cocher *Show Engine Content*.
- Vérifier la collision : ouvrir le mesh, bouton *Collision* de la barre d'outils > cocher *Simple Collision*
  pour l'afficher. Rien de vert ? Menu *Collision* > *Add Box Simplified Collision*, puis sauvegarder.

### 8.2 Créer les dossiers

Dans le Content Browser : `Content/Data/Board/NodeTypes/` et `Content/Data/Board/Entities/`
(clic droit > *New Folder*). ⚠️ Jamais dans `Sandbox/` : le Level Editor ignore ce dossier.

### 8.3 Type de case « Sol » (obligatoire)

1. Dans `Data/Board/NodeTypes/` : clic droit > *Miscellaneous* > *Data Asset*.
2. Dans la fenêtre *Pick Class For Data Asset Instance*, chercher **Board Node Type** et valider.
   (UE affiche les noms de classes avec des espaces. `Board Definition` n'apparaît pas : classe abstraite.)
3. Nommer l'asset `DA_Node_Floor`, double-cliquer pour l'ouvrir.
4. Catégorie *Level Editor* : `Display Name` = « Sol », `Description` = « Case normale, traversable »,
   `Editor Color` au choix, `Sort Order` = 0. Laisser `Settings Class` vide.
   Cocher `Is Default Type` : c'est le type présélectionné dans le pinceau du mode « Board Editor ».
5. Catégorie *Node* : `Behaviors` **vide** (c'est ce qui en fait le type « sol » pour la migration),
   `Node Class` = `BoardNode` (valeur par défaut, ne pas toucher).
6. Catégorie *Node | Visuals* : `Mesh` = le mesh choisi en 8.1. `Mesh Scale` : pour le cylindre du moteur
   et une `CellSize` de 200, essayer (1.5, 1.5, 0.1) pour obtenir un disque plat. `Material Overrides` : optionnel.
7. Sauvegarder (Ctrl+S).

### 8.4 Type de case « Sortie » (obligatoire)

1. Dupliquer `DA_Node_Floor` (clic droit > *Duplicate*), nommer `DA_Node_Exit`.
2. `Display Name` = « Sortie », `Description` = « L'atteindre fait gagner », `Sort Order` = 10.
3. `Behaviors` : cliquer sur la liste déroulante, déplier **Board > Node** et cocher **Exit**.
   Le tag a été créé par le code (`BoardTags.cpp`) : il n'y a rien à ajouter dans les Project Settings.
4. Optionnel mais conseillé : un matériau différent dans `Material Overrides` (index 0) pour reconnaître la sortie.
5. Sauvegarder.

### 8.5 Type de case « Bloquée » (seulement si le niveau avait des cases `bIsBlocked`)

Même principe : `DA_Node_Blocked`, `Behaviors` = **Board > Node > Blocked**, `Sort Order` = 20.

### 8.6 Type d'entité « Joueur » (obligatoire)

1. Dans `Data/Board/Entities/` : clic droit > *Miscellaneous* > *Data Asset* > **Board Entity Type**.
2. Nommer `DA_Entity_Player`. `Display Name` = « Joueur », `Description` = « Point de départ du joueur (un seul par niveau) ».
3. Catégorie *Entity* : `Pawn Class` = **BP_PlayerPawn**.
4. Sauvegarder.

Faire de même pour tout autre pion utilisé en `StartingEntity` dans le niveau. Sinon la migration
l'ignore, et l'Output Log l'indique.

### 8.7 Lancer la migration

1. Ouvrir le niveau (ex. `Levels/NodeLevel`). Ne pas être en Play.
2. Ouvrir la console : touche **`** (accent grave, à gauche du 1 en QWERTY, souvent **²** en AZERTY), ou le
   champ *Cmd* en bas à gauche de l'éditeur.
3. Taper `BoardEditor.ImportLegacy` puis Entrée.
4. Window > Output Log, filtrer sur `BoardEditor` puis sur `Validation` : lire ce qui n'a pas pu être converti
   et le résultat de la validation.
5. Résultat attendu : les anciennes cases sont remplacées par des `Node_X_Y` (dossier Outliner `Board/Nodes`), avec
   le mesh de leur type. Les liens reproduisent l'ancien comportement : toutes les voisines sont reliées, sauf les liens bloqués.
6. Tester Ctrl+Z (tout l'import doit s'annuler d'un coup), puis Ctrl+Y.
7. Lancer en PIE : le jeu doit se comporter comme avant. Puis sauvegarder le niveau.

### 8.8 Problèmes fréquents

| Symptôme | Cause probable |
|---|---|
| `Board Node Type` absent de *Pick Class* | Éditeur pas relancé après compilation, ou compilation échouée |
| `Command not recognized: BoardEditor.ImportLegacy` | Module éditeur non chargé : régénérer les fichiers projet, recompiler, relancer |
| Log « Aucun type de case sans comportement » | `DA_Node_Floor` absent, dans `Sandbox/`, non sauvegardé, ou avec un `Behaviors` non vide |
| Log « aucun UBoardEntityType avec PawnClass = … » | Pas de Data Asset d'entité pour ce pion (ou `Pawn Class` différent) |
| « Ce Board a déjà un layout : import annulé » | Import déjà fait. Pour recommencer : `BoardEditor.ImportLegacy force` |
| Cases invisibles | `Mesh` vide sur le type de case. Si le `Mesh` est renseigné : `BoardEditor.Resync` (cases générées avant la correction de la mise à jour automatique) |
| Le clic du joueur ne fait rien en PIE | Le mesh n'a pas de collision (voir 8.1) |
| Pas de traits entre les cases | `LinkMeshes` sans mesh dans `BP_Board` (réglage de BP, inchangé) |

### 8.9 Après la migration de tous les niveaux (étape 4b)

Après l'import, compléter le niveau dans le mode « Board Editor » (départ joueur, sortie… via « Selected Cell »).

Quand toutes les branches sont fusionnées dans `develop` et tous les niveaux migrés : supprimer les propriétés
`_DEPRECATED`, leurs getters `GetLegacy*`/`IsLegacy*`, `FBoardLayoutSync::BuildLayoutFromLegacyScene` et la
commande `ImportLegacy`. D'ici là, ne pas les supprimer : un niveau d'une autre branche construit à l'ancienne
ne pourrait plus être converti.

## 8 bis. À améliorer (retours de l'équipe)

> Agent : ajouter ici chaque retour d'utilisation, puis cocher et dater quand il est traité. La liste évolue.

| Date | Retour | Statut |
|---|---|---|
| 12/11/2026 | Aperçu peu lisible (cercles blanc / vert / rouge sans légende) : un seul cercle d'état par case (rouge = erreur, orange = avertissement, vert = valide) + point central à la couleur du type | ✅ Testé (12/11/2026) |
| 12/11/2026 | Ctrl + glisser : aucun retour visuel | ✅ Testé (12/11/2026) : cases touchées encadrées pendant le geste |
| 12/11/2026 | Après le relâchement (avant Submit), rien n'indique les cases modifiées | ✅ Écrit (12/11/2026), à tester : carré fin à la couleur du type sur toute case modifiée (nouvelle, type, entités, objets, réglages, liens) |
| 12/11/2026 | Submit refusé : lister les erreurs (5 max) dans la fenêtre au lieu de renvoyer au panneau | ✅ Testé (12/11/2026) |
| 12/11/2026 | Échap ne désélectionne pas une case | ✅ Testé (12/11/2026) |
| 12/11/2026 | Traduction anglaise selon la langue de l'éditeur | ✅ Remplacé (12/11/2026) : interface **en anglais uniquement** (panneau, messages de validation, noms / tooltips / catégories des Data Assets du plateau). Traduction française abandonnée (procédure trop lourde pour l'équipe) |

## 9. Journal des décisions

| Date | Décision | Raison |
|---|---|---|
| 10/11/2026 | Mode éditeur `UEdMode` + Interactive Tools Framework (pas Scriptable Tools, pas d'Editor Utility Widget comme cœur) | Seule option avec une vraie interaction viewport sans Slate lourd ; un bug de compilation Shipping a été signalé avec le plugin Scriptable Tools en 5.7 |
| 10/11/2026 | Brouillon puis Submit | Validation avant écriture, un Submit = un Ctrl+Z |
| 10/11/2026 | Périmètre : plateau seulement | Le décor reste libre pour les GA/LD |
| 10/11/2026 | Liens explicites, voisines orthogonales seulement | Choix GD (à la Hitman GO). Change le modèle de jeu : auparavant, voisines reliées par défaut |
| 10/11/2026 | Pas de hauteur (plateau plat) | Confirmé : grille 2D `FIntPoint` suffisante |
| 10/11/2026 | Visuel d'une case = son type | Cohérence et mise à jour globale |
| 10/11/2026 | Listes d'entités / d'objets par case | Évolutif ; une règle pourra limiter à 1 si besoin |
| 10/11/2026 | Erreurs bloquantes : case isolée, 1 départ joueur, ≥ 1 sortie, graphe d'un seul tenant ; sortie atteignable = avertissement ; option « Ignore Errors » décochée par défaut | Choix de l'équipe |
| 10/11/2026 | Layout stocké dans `ABoard` (pas de Data Asset de niveau séparé) | Un seul fichier par niveau, pas de référence croisée à maintenir |
| 10/11/2026 | Comportements de cases par Gameplay Tags | Ajout de comportements sans nouveaux booléens ; les GD peuvent nommer des tags |
| 11/11/2026 | Un seul outil + gestes / raccourcis (tableau du § 1), avec aide contextuelle au survol | Choix de l'équipe : rapidité ; l'aide compense la mémorisation |
| 11/11/2026 | Effacer en glissant = Shift + clic gauche ; clic droit maintenu reste la caméra | Ne pas casser la navigation standard du viewport |
| 11/11/2026 | Nouvelles cases = type du pinceau, présélectionné par « Is Default Type » ; Ctrl + clic applique le type | Aucun type codé en dur, changement de type rapide |
| 11/11/2026 | Brouillon non soumis à la sortie du mode : question Soumettre / Abandonner | Ne jamais perdre ni écrire un brouillon sans le dire |
| 11/11/2026 | Étape 2 découpée en 2a (squelette testable) et 2b (gestes glissés) | Compiler et tester par petits morceaux |
| 11/11/2026 | Historique du brouillon par `FToolCommandChange` (copie avant / après du layout) | Mécanisme natif de l'ITF ; layouts petits |
| 12/11/2026 | Détails d'une case = section « Selected Cell » du panneau ; entités placées depuis ces détails ; palette = liste déroulante + description ; « Next Error » (sans Slate) | Choix de l'équipe : rapide à livrer, sans code Slate |
| 12/11/2026 | Échap : tracé en cours, puis sélection, puis arrêt (et relance) de l'outil | Comportement standard des outils UE (annulations imbriquées) |
| 12/11/2026 | Verrouillage léger : `NotPlaceable` + position verrouillée ; les cases restent sélectionnables (debug) | Choix de l'équipe : sûr sans gêner le débogage |
| 12/11/2026 | Code de migration conservé jusqu'à la fusion de toutes les branches ; édition manuelle du `BP_Board` supprimée | D'autres branches peuvent contenir des niveaux à l'ancien format |
| 12/11/2026 | Guide utilisateur = document partagé (Claude Docs), séparé de ce CLAUDE.md | Choix de l'équipe : lisible et commentable par les LD / GA sans le dépôt |
| 11/11/2026 | Vue de dessus en perspective à l'entrée du mode, vue d'avant rétablie à la sortie (proposition de l'équipe) | Vue d'ensemble du plateau sans perdre l'éclairage ni la navigation habituelle |
| 11/11/2026 | Aucun geste au clic droit (réservé à la caméra) ; effacer un lien = Shift + clic sans bouger | Décision d'équipe : éviter toute confusion. Remplace le « clic droit simple » écrit puis retiré le même jour |
| 12/11/2026 | Interface de l'outil en anglais uniquement (panneau, messages de validation, métadonnées des Data Assets du plateau) ; pas de traduction française | Choix de l'équipe : la localisation native UE (Localization Dashboard, `.po`) était trop lourde à maintenir. Les textes restent en `LOCTEXT` : une traduction reste possible plus tard sans toucher au code |
| 12/11/2026 | Logs, aide des commandes console et commentaires restent en français | Destinés aux programmeurs de l'équipe |

## 10. Pièges UE rencontrés

- **Data Asset et `EditDefaultsOnly`** : un Data Asset est une *instance*. Ses propriétés doivent être
  `EditAnywhere`, sinon elles ne sont pas modifiables dans l'asset.
- **Propriétés `_DEPRECATED`** : une propriété renommée `Xxx_DEPRECATED` relit la valeur sauvegardée sous le nom
  `Xxx` mais n'est jamais réécrite. C'est le moyen standard de migrer des données.
- **Sous-objets instanciés** (`Instanced`) : copier une struct copie les *pointeurs*, pas les objets → `DeepCopySettings`.
- **Transactions** : seuls les objets sur lesquels on a appelé `Modify()` pendant la transaction sont restaurés par
  Ctrl+Z. Créer/détruire un acteur : `Level->Modify()` avant. Les visuels recalculés (mesh d'une case, traits des
  liens) sont reconstruits dans `PostEditUndo`.
- **`TSubclassOf<T>` et déclaration anticipée** : déclarer un membre `TSubclassOf<T>` avec un simple `class T;` compile,
  mais le **tester ou le lire** (`if (!PawnClass)`, `->`, `.Get()`) appelle `T::StaticClass()` et exige la classe complète
  (erreur C2027 « type non défini »). Les headers des définitions incluent donc la classe complète qu'ils exposent.
  Piège aggravé par l'*adaptive unity build* : selon les fichiers regroupés, l'erreur peut apparaître ou disparaître.
- **Boutons sans Slate** : une `UFUNCTION(CallInEditor)` sur un `UInteractiveToolPropertySet` devient un bouton dans
  le panneau du mode. Quand le code modifie une propriété affichée, appeler `NotifyOfPropertyChangeByTool` pour rafraîchir.
- **Échap n'atteint pas l'outil** : dans le viewport, l'éditeur traite Échap avec ses propres commandes (« tout
  désélectionner »…) avant le mode. `IInteractiveToolNestedAcceptCancelAPI` ne suffit donc pas : un préprocesseur
  d'entrées Slate (`FBoardEditorInputProcessor`, enregistré dans `Enter`, retiré dans `Exit`) reçoit la touche en premier
  et ne la garde que si l'outil s'en sert, et seulement si le viewport a le focus.
- **`UEdMode` n'a pas d'`InputKey` en 5.8** (API des anciens modes) : les entrées passent par l'ITF. Échap pendant un
  tracé : l'outil implémente `IInteractiveToolNestedAcceptCancelAPI` (l'éditeur lui propose d'abord Échap). Hors tracé,
  Échap arrête l'outil : `UBoardEditorMode::OnToolEnded` + `ModeTick` le relancent, et `SaveProperties` /
  `RestoreProperties` conservent les réglages du panneau (type du pinceau…).
- **Le panneau du mode ne se reconstruit pas** : `FModeToolkit` ne liste les jeux de propriétés de l'outil qu'au démarrage
  de l'outil. `SetToolPropertySourceEnabled` ne fait donc ni apparaître ni disparaître une section. Pour une section
  conditionnelle : la garder toujours présente et cacher ses lignes avec `EditCondition` + `EditConditionHides`
  (cf. `UBoardCellDetailsProperties::bHasSelection`).
- **Le panneau édite des copies** : la section « Selected Cell » reçoit des copies (réglages dupliqués) de la
  case ; chaque modification est recopiée dans le brouillon par `ApplyDraftEdit` (Ctrl+Z). Ne jamais lui donner les
  objets du brouillon directement (modification hors historique). Pendant cette recopie (`bApplyingDetails`), on ne
  recharge pas le panneau depuis le brouillon, pour ne pas perturber l'édition en cours.
- **Ne pas capturer le clic droit avec l'ITF** : un comportement d'entrée sur le bouton droit bloquerait la rotation de caméra.
- **Dans le mode, clic gauche + glisser ne déplace plus la caméra** (il trace) : naviguer au clic droit maintenu + ZQSD,
  molette, ou clic du milieu.
- **Limite connue (2a)** : après un Submit, Ctrl+Z annule le Submit dans le niveau mais le brouillon ne suit pas.
  Cliquer « Discard Draft » pour repartir du niveau.
- **`LOCTEXT_NAMESPACE` : un `#undef` à la fin de chaque `.cpp`**. Avec l'*unity build*, plusieurs `.cpp` sont
  compilés ensemble : un `#define` oublié sans `#undef` donne une erreur de redéfinition (ou un mauvais espace de noms).
- **Les tooltips viennent des commentaires** quand `ToolTip` n'est pas renseigné : le commentaire français s'afficherait
  dans l'éditeur. Pour une propriété visible, toujours un `ToolTip` explicite en anglais.
- **Formes pleines dans l'aperçu** : `PDI->DrawPoint` dessine un carré, et le PDI ne remplit pas une forme sans matériau.
  Le disque central est fait de cercles concentriques épais (`DrawFilledDisc`).
- **`LogPlagueTaleGO` n'est pas exportée** : le module éditeur utilise sa propre catégorie `LogBoardEditor`.
- **Ajouter un module** (`.uproject` + `*.Target.cs` + `*.Build.cs`) : régénérer les fichiers projet
  (clic droit sur `PlagueTaleGO.uproject` > Generate Visual Studio project files).
