# Source/PlagueTaleGO/BlueDust — Poussière bleue (traces du joueur)

> Lire d'abord `/CLAUDE.md` (racine) puis `/Source/PlagueTaleGO/CLAUDE.md` (normes C++).
> Ce fichier couvre les règles, l'architecture, l'API pour les futures features (Ver, Statue), le contrat du Niagara,
> les recettes et les décisions. Branche de développement : `feature/PlayerBlueDust`.
> Pas de guide tout public : seuls les devs touchent à cette feature (réglages dans des BP).

## 1. Règles de jeu (validées par les GD le 27/11/2026)

Le joueur laisse derrière lui une trace de poussière bleue sur ses dernières cases. Elle sert aux ennemis.

- À **chaque déplacement** du joueur, au **début** de l'animation :
  1. chaque trace perd 1 de durée de vie ; à 0 elle disparaît ;
  2. si le joueur arrive sur une case tracée, cette trace disparaît (il est toujours la tête de la trace) ;
  3. une trace de durée `TrailLength` est posée sur la case quittée.
- `TrailLength` = **2** par défaut (traces sur N-1 et N-2), **réglable par niveau**. Compté en déplacements.
- **Une seule trace par case.** **Aucune trace au début du niveau** (le spawn n'est pas un déplacement).
- **Statue** (pas encore codée) : quand elle détecte la trace ou le joueur et se met à le suivre, **toutes** les traces
  sont détruites d'un coup et le joueur **n'en laisse plus** tant qu'elle existe (elle bouge dans le même tour que
  lui : « elle mange la poussière » n'est que visuel). Statue détruite (par le Ver ou une tuile Artefact) : la trace
  reprend au **déplacement suivant**.
- **Ver** (pas encore codé) : s'il détecte une trace, il la suit jusqu'au joueur.
- **Tuile Artefact** : aucune interaction. **Pas de** type de case qui empêche la trace, **pas de** son, **pas de**
  déplacement forcé prévus (ne pas les ajouter sans demande des GD).

## 2. Fichiers

```
BlueDust/
  BlueDustTypes.h             FBlueDustTrace (Node, ExitNode, RemainingLifetime), EBlueDustRemovalReason
  BlueDustTrailComponent.h/.cpp  UBlueDustTrailComponent : SEULE source de vérité des traces, règles, API, délégués,
                              pilotage des visuels, dessin de debug
  BlueDustVisual.h/.cpp       ABlueDustVisual : visuel d'UNE trace = un segment Niagara animé (affichage seulement)
  BlueDustCommands.cpp        Commandes console BlueDust.* (hors Shipping)
Hors du dossier :
  Board/BoardSubsystem        délégué OnPawnMoved(Pawn, From, To), diffusé par TryMovePawn
  Entities/ABoardPawn.h       GetMoveDuration() (synchronise la queue de la trace avec le joueur)
  Entities/Player/APlayerPawn crée le composant BlueDustTrail (GetBlueDustTrail()), applique les réglages du niveau
  Entities/Player/PlayerPlacementSettings.h  UPlayerPlacementSettings : X par niveau (Board Editor)
Assets : Content/Blueprints/BlueDust/BP_BlueDustVisual, Content/FX/NS_BlueDust, Content/Materials/M_BlueDustParticle
Module : "Niagara" dans PlagueTaleGO.Build.cs (privé) ; plugin Niagara listé dans le .uproject
```

## 3. Architecture

```
Clic ─► TryMovePawn (occupation mise à jour, BeginMove) ─► OnPawnMoved(Joueur, From, To)
                                                              │
                         UBlueDustTrailComponent::HandlePawnMoved (filtre : seul son propriétaire)
                           AgeTraces → RemoveTraceInternal(To, OwnerReturned) → LeaveTrace(From, To) si non bloqué
                                                              │
                 Notify* (SEULS endroits qui diffusent) ─► délégués OnTraceAdded / OnTraceRemoved / OnTrailUpdated
                                                        └─► ABlueDustVisual : BeginSegment / Retract / FadeOut
```

- **Composant sur le joueur** (`CreateDefaultSubobject` dans `APlayerPawn`) : la trace « appartient » au joueur.
  Équivalent Godot : un nœud enfant `Trail` avec son script dans la scène du joueur.
- **Données ≠ visuel** : le composant tient les traces ; `ABlueDustVisual` ne fait qu'afficher et ne décide de rien
  (même principe que layout / `ABoardNode`).
- **Visuel = segments** : chaque trace dessine le segment `Node → ExitNode` (case par laquelle le joueur est reparti).
  Bout à bout, ils forment une traînée continue du plus ancien nœud tracé jusqu'au joueur.
  - construction : la fin du segment suit la position réelle du joueur pendant son déplacement ;
  - `Expired` : le début avance vers `ExitNode` en `MoveDuration` (la queue suit la tête) ;
  - `OwnerReturned` par le même lien (A → B → A) : la fin recule en suivant le joueur (segment « mangé » sous lui) ;
    sinon (boucle) : comme `Expired` ;
  - `Suppressed`, `Cleared`, `Removed` : fondu sur place.
  - Fin de chaque cas : fondu (`TrailAlpha` 1 → 0 en `FadeDuration`), puis l'acteur se détruit seul.
  - Un visuel ne sert qu'à un segment : retracer une case crée un nouveau visuel (l'ancien finit son fondu seul).

## 4. Invariants (ne pas casser)

1. **`UBlueDustTrailComponent` est la seule source de vérité des traces.** Une autre feature lit ou modifie la trace
   par son API publique, jamais en touchant aux visuels ni en recréant sa propre liste.
2. **Toute diffusion passe par `NotifyTraceAdded` / `NotifyTraceRemoved` / `NotifyTrailUpdated`** : ce sont eux qui
   pilotent aussi les visuels. Ne jamais appeler `OnTrace*.Broadcast` directement.
3. **La trace réagit au mouvement, elle ne le contrôle pas** : aucune modification de l'occupation du plateau.
   Elle ne dépend que de `UBoardSubsystem::OnPawnMoved` (les téléportations ne laissent rien).
4. **Le visuel n'a aucune règle de jeu.** Un changement de règle se fait dans le composant ; le visuel ne fait
   qu'exécuter `BeginSegment` / `Retract` / `FadeOut`.
5. **Un bloqueur est un objet vivant** (pointeur faible) : un acteur détruit lève son blocage tout seul
   (`AActor::OnDestroyed`). Ne pas compter sur un appel explicite de `RemoveEmissionBlocker` à la mort d'une statue.

## 5. API pour les futures features

Accès : `GameFlow->GetPlayerPawn()->GetBlueDustTrail()` (`APlayerPawn`, include `BlueDust/BlueDustTrailComponent.h`).

| Besoin | API |
|---|---|
| Une case porte-t-elle une trace ? | `HasTraceAt(Node)`, `GetTraceLifetime(Node)` (0 = pas de trace) |
| Toutes les traces | `GetTraces()` (index 0 = N-1, la plus récente), `GetTraceCount()` |
| Détection dans une zone | `FindTracesIn(Nodes)` : traces sur ces cases, de la plus récente à la plus ancienne |
| Remonter la trace vers le joueur | `GetNextTowardOwner(Node)` = `ExitNode` : toujours une voisine reliée, sur le vrai chemin du joueur. La case renvoyée ne porte pas forcément de trace (case du joueur…) |
| Retirer une trace / tout | `RemoveTraceAt(Node)` (raison `Removed`), `ClearTrail()` (`Cleared`) |
| Statue | `SuppressTrail(Statue)` : tout retirer (`Suppressed`) + bloquer tant que la statue existe |
| Bloquer sans effacer | `AddEmissionBlocker(Source)` / `RemoveEmissionBlocker(Source)` / `IsEmissionBlocked()` (plusieurs bloqueurs cumulables) |
| Changer X en jeu | `SetTrailLength(N)` (raccourcit les traces existantes, jamais ne les rallonge ; 0 = tout retirer) |
| Réagir | `OnTraceAdded(Trace)`, `OnTraceRemoved(Trace, Reason)`, `OnTrailUpdated(Trail)`, `OnEmissionBlockedChanged(bBlocked)` |

## 6. Recettes

**Brancher la Statue** (quand elle sera codée)
1. Au moment où elle détecte la trace ou le joueur et commence à le suivre : `Trail->SuppressTrail(this)`.
2. Rien à faire à sa mort : sa destruction lève le blocage (la trace reprend au déplacement suivant du joueur).
3. Si elle peut cesser de suivre sans mourir (à confirmer par les GD) : `Trail->RemoveEmissionBlocker(this)`.
4. Effet visuel « elle mange la poussière » : dans `BP_BlueDustVisual`, événement `On Trace Hidden` avec
   `Reason == Suppressed`, ou côté statue.

**Brancher le Ver** (quand il sera codé)
1. Détection : `Trail->FindTracesIn(CasesDeDétection)`. Choisir laquelle suivre est une règle du Ver (la plus
   récente = `[0]`, demander aux GD).
2. Poursuite : aller vers la trace détectée (`UBoardSubsystem::FindPath`), puis de case en case avec
   `GetNextTowardOwner` jusqu'au joueur. Si la trace a disparu entre-temps : `GetNextTowardOwner` renvoie nullptr,
   prévoir un repli (chemin direct vers le joueur ?) à valider avec les GD.
3. Le Ver détruit-il la trace en passant ? **Non décidé** : si oui, `Trail->RemoveTraceAt(Case)`.

**Changer X** : par défaut, `BP_PlayerPawn` > composant `BlueDustTrail` > `Trail Length`. Pour un niveau : Board
Editor > case du joueur > Selected Cell > entité joueur > « Blue Dust Trail Length » (case Override). Nécessite
`DA_Entity_Player` > `Settings Class` = `Player Placement Settings` (fait). Un placement soumis avant ce réglage
reçoit ses réglages à sa prochaine édition ; en attendant, la valeur du BP s'applique.

**Ajouter une règle** (ex. un type de case sans trace, si les GD le demandent) : dans le composant (`LeaveTrace` ou
`HandlePawnMoved`), testée par comportement (`Node->HasBehavior(BoardTags::…)`), puis mettre à jour § 1 et § 9.

**Ajouter une raison de disparition** : valeur dans `EBlueDustRemovalReason` (avec `ToolTip` anglais), puis choisir
son animation dans `NotifyTraceRemoved` (sinon : fondu sur place).

**Remplacer le Niagara par l'asset officiel** : respecter le contrat du § 7, puis le choisir dans
`BP_BlueDustVisual` > composant `Effect` > Niagara System Asset. Aucun code à changer ; un paramètre renommé se
règle dans les Class Defaults du BP (catégorie *Niagara Parameters*, None = non envoyé).

## 7. Contrat du Niagara (`NS_BlueDust`)

Le code envoie ces paramètres **User** à chaque frame d'animation (`SetVariable*` : nom sans « User. »). Un paramètre
absent ou **d'un autre type** est ignoré sans message (piège rencontré : vérifier les types).

| Paramètre | Type | Contenu | Usage dans le prototype |
|---|---|---|---|
| `SegmentStart` / `SegmentEnd` | **Position** | Extrémités actuelles du segment (monde, `Trail Height` comprise) | Émetteur ruban (Beam) optionnel |
| `SegmentBoxSize` | Vector | Boîte d'apparition alignée monde : longueur + `Box Width`, épaisseur `Box Thickness` | *Shape Location* (Box) > Box Size |
| `SegmentMaskSize` | Vector | Même empreinte, hauteur 2 × `Mask Height` | *Kill Particles in Volume* (Box, tue à l'extérieur) |
| `SegmentLength` | Float | Longueur actuelle (cm) | Optionnel : Spawn Rate proportionnel |
| `TrailAlpha` | Float | 1, puis 1 → 0 pendant le fondu final | *Scale Color* > Scale Alpha (tous les émetteurs) |
| `LifetimeRatio` | Float | Durée restante / `TrailLength` (1 = neuve) | Inutilisé (âge non distingué pour l'instant) |

Réglages obligatoires des émetteurs :
- **Local Space décoché** (espace monde) : le code déplace le composant `Effect` au centre du segment à chaque frame ;
  la boîte d'apparition et le masque, centrés sur le système, suivent ainsi le segment sans décalage à régler.
- *Loop Behavior* = Infinite ; jamais d'auto-activation (le code active / désactive).
- *Kill Particles in Volume* en tête de *Particle Update*, centre = position du système : c'est lui qui dévoile et
  retire les particules au rythme du segment.
- Opacité qui monte vite (≈ 10–15 % de la vie) : sinon les particules deviennent visibles après la fin du déplacement.

Prototype actuel : émetteurs `Ground` (polygones plats : sprites *Custom Facing Vector* (0, 0, 1), rotation
aléatoire) et `Motes` (particules qui montent), matériau `M_BlueDustParticle` (Translucent, Unlit, Particle Color
→ Emissive × `Intensity`, alpha → Opacity). Réglages du BP `BP_BlueDustVisual` : `Trail Height`, `Box Width`,
`Box Thickness`, `Mask Height`, `Fade Duration`. Événements BP : `On Trace Shown`, `On Trace Hidden(Reason)`.

## 8. Commandes console (PIE, touche ² ; absentes du Shipping)

| Commande | Effet |
|---|---|
| `BlueDust.Dump` | Log : traces (N-1, N-2…), durée restante, `ExitNode`, bloqueurs |
| `BlueDust.Debug [0\|1]` | Sphères + durée + segment de chaque trace (sans argument : bascule). Coupé par défaut |
| `BlueDust.Clear` | Retire toutes les traces |
| `BlueDust.SetLength <N>` | Change X en cours de partie |
| `BlueDust.Suppress` / `BlueDust.Release` | Simule une statue : bloqueur créé + `SuppressTrail` / bloqueur **détruit** (même chemin qu'une statue tuée) |

Logs `[BlueDust]` : événements de blocage en `Log`, chaque trace posée / retirée en `Verbose`
(`Log LogPlagueTaleGO Verbose`). État en PIE : Shift+F1 > joueur > composant `BlueDustTrail` (`Traces`, `EmissionBlockers`).

## 9. Journal des décisions

| Date | Décision | Raison |
|---|---|---|
| 27/11/2026 | Règles du § 1 (X = 2 en déplacements, par niveau ; statue : tout détruire + bloquer jusqu'à sa destruction ; pas de son, pas de case sans trace, pas de déplacement forcé) | Validées par les GD / l'équipe |
| 27/11/2026 | Une trace max par case ; la trace sous le joueur disparaît quand il y revient | Pas d'effet en double ni sous le joueur ; trace propre à suivre pour le Ver |
| 27/11/2026 | Composant sur le joueur (pas de subsystem) | La trace appartient au joueur ; une seule source de traces aujourd'hui |
| 27/11/2026 | Déclenchement par `UBoardSubsystem::OnPawnMoved` (pas par le GameFlow) | Le plateau est l'autorité du mouvement ; délégué réutilisable par d'autres features |
| 27/11/2026 | Durée de vie stockée dans les données du composant, pas dans l'acteur visuel | Le visuel ne porte aucune donnée de jeu |
| 27/11/2026 | Bloqueurs en pointeurs faibles + `OnDestroyed` | La reprise après la mort d'une statue ne dépend d'aucun appel explicite |
| 27/11/2026 | Réglage par niveau : `UPlayerPlacementSettings` avec case Override | Changer la valeur du BP met à jour les niveaux qui ne la remplacent pas |
| 27/11/2026 | Visuel : segments animés `Node → ExitNode` (et non un effet par case) | Retour de l'équipe : il faut une vraie traînée entre les nœuds, pas des « pierres » |
| 27/11/2026 | `GetNextTowardOwner` renvoie `ExitNode` (au lieu de la trace suivante) | Toujours une voisine reliée sur le vrai chemin du joueur |
| 27/11/2026 | Niagara créé à la main dans l'éditeur, code limité à un contrat de paramètres | L'asset officiel des GA le remplacera |
| 27/11/2026 | Le code déplace `Effect` au centre du segment ; masque `Kill Particles in Volume` pour dévoiler / retirer | Moins de liens à régler dans le Niagara ; dévoilement synchronisé avec le joueur |

## 10. Pièges UE rencontrés

- **Paramètre User d'un mauvais type = ignoré en silence** (`SegmentBoxOffset` créé en Position au lieu de Vector :
  particules centrées sur la case). Vérifier les types ; *Tools > Debug > Niagara Debugger* montre les valeurs en PIE.
- **Box Location en espace monde n'hérite pas de la rotation du système** : d'où une boîte alignée sur les axes
  monde, calculée par le code (les liens sont orthogonaux), et un acteur sans rotation.
- **Opacité qui monte lentement** : avec un déplacement de 0,4 s, les particules ne deviennent visibles qu'une fois le
  segment complet (impression d'apparition « d'un coup »).
- **Un `UFUNCTION` Blueprint ne renvoie pas de référence** : `GetTraces()` renvoie une copie.
- **`AddDynamic` dans un constructeur** vise l'objet par défaut de la classe : s'abonner dans `BeginPlay`.
- **Renommer une classe placée dans des BP** (`ABlueDustTrace` → `ABlueDustVisual`) : `ClassRedirects` dans
  `DefaultEngine.ini` (`[CoreRedirects]`), sinon les BP perdent leur parent.
- **Nouvelle `UPROPERTY` / classe ou `Build.cs` modifié** : fermer l'éditeur, (régénérer les fichiers projet), compiler.

## 11. Questions ouvertes (à poser aux GD avant de coder)

- Ver : quelle trace suivre s'il en détecte plusieurs ? Détruit-il la trace en passant ? Que faire si elle disparaît
  pendant la poursuite ?
- Statue : peut-elle cesser de suivre le joueur sans être détruite (le blocage doit-il alors être levé) ?
- Visuel : distinguer l'âge des traces (`LifetimeRatio` est déjà envoyé) ?
