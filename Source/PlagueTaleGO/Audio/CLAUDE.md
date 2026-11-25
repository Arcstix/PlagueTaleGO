# Source/PlagueTaleGO/Audio — Système audio

> Lire d'abord `/CLAUDE.md` (racine) puis `/Source/PlagueTaleGO/CLAUDE.md` (normes C++).
> Ce fichier couvre l'architecture, les invariants, les recettes et le **registre des événements sonores**.
> **Guide tout public (intégrer un son sans code)** : document partagé « Guide audio — Plague Tale GO »
> (https://claude.ai/artifact/KmwHnSM7EVKp6SkPsw19jZ). À tenir à jour quand un geste, un réglage ou un événement change.
> Branche de développement : `feature/AudioManager`.

## 1. Principe

Le code ne référence **jamais** un son : il annonce un **événement sonore** (Gameplay Tag `Audio.*`) à
`UBoardAudioSubsystem`, qui trouve le son à jouer :

```
Code (moment du jeu) ──► UBoardAudioSubsystem::Play*/StartLoop*(Tag, [Position], [Contexte])
                              │
                              ├─ 1. Contexte->SoundOverrides[Tag]   (type de case / pion / objet, si passé par le code)
                              ├─ 2. DA_AudioLibrary.Sounds[Tag]     (bibliothèque, Project Settings > Game > Board Audio)
                              └─ 3. sinon : warning [Audio] une fois par tag, silence
                              ▼
                     UGameplayStatics (PlaySound2D / PlaySoundAtLocation / SpawnSound*)
```

Le moteur fait le reste, réglé sur les assets : limite de sons simultanés (Sound Concurrency), catégories
de volume (Sound Class), atténuation et spatialisation (Sound Attenuation), recyclage des composants audio.
Équivalent Godot : l'autoload `AudioManager` + l'enum `SOUND_EFFECT_TYPE`, sans la gestion manuelle des players.

## 2. Fichiers

```
AudioTags.h/.cpp          Tags natifs Audio.* (un tag = un événement sonore)
BoardSoundEntry.h/.cpp    FBoardSoundEntry : sons (tirage au hasard), volume, hauteur ± aléatoire, atténuation, concurrency
BoardAudioLibrary.h/.cpp  UBoardAudioLibrary (Data Asset) : TMap tag → entrée, atténuation 3D par défaut
BoardAudioSettings.h/.cpp UBoardAudioSettings (UDeveloperSettings) : bibliothèque utilisée (DefaultGame.ini)
BoardAudioSubsystem.h/.cpp UBoardAudioSubsystem (UWorldSubsystem, Game/PIE) : seul lecteur de sons
BoardAudioCommands.cpp    Commandes console BoardAudio.* (hors Shipping)
```

Hors de ce dossier : `UBoardDefinition::SoundOverrides` (sons propres à un type), `ABoardPawn::MoveSoundEvent`
(son de déplacement réglé dans le BP), `ABoardPawn::EntityType` et `UBoardItem::ItemType` (contextes audio).

## 3. Invariants (ne pas casser)

1. **Un son de gameplay passe toujours par `UBoardAudioSubsystem`**, avec un tag `Audio.*`. Jamais de
   `UGameplayStatics::PlaySound*` ni de `USoundBase*` en dur dans une classe de gameplay.
2. **Son court → `PlaySound2D` / `PlaySoundAtLocation`. Boucle → `StartLoop*`**, garder le composant renvoyé dans
   un `UPROPERTY()` et l'arrêter avec `StopLoop`. `Play*` refuse un son en boucle (il ne pourrait jamais être arrêté).
3. **La Sound Class et le bouclage sont des propriétés de l'asset son**, pas du code ni de l'entrée.
   Sound Class par défaut : `SC_SFX` (Project Settings > Audio > Default Sound Class).
4. **Aucun son ne survit à un changement de niveau** (World Subsystem). Pas de musique continue entre niveaux.
5. **Le décor sonore d'un niveau** (ambiance, feu de cheminée) = acteur **Ambient Sound** posé dans le niveau,
   sans code ni tag. Il ne s'arrête pas à la victoire / défaite.
6. **Un son manquant n'est jamais une erreur** : warning `[Audio]` une seule fois par tag, le jeu continue.
   On peut donc brancher un événement avant que son son existe.
7. **Un événement qui varie selon le contenu reçoit son contexte du code** (dernier paramètre). Sans contexte,
   les `SoundOverrides` des Data Assets sont ignorés pour cet événement : le préciser dans le registre (§ 5).

## 4. Recettes

**Ajouter un événement sonore** (code)
1. Déclarer le tag dans `AudioTags.h` (`UE_DECLARE_GAMEPLAY_TAG_EXTERN`) et le définir dans `AudioTags.cpp`
   (`UE_DEFINE_GAMEPLAY_TAG_COMMENT`, nom `Audio.<Domaine>.<Nom>`).
2. Le déclencher là où l'événement a lieu :
   ```cpp
   #include "PlagueTaleGO/Audio/AudioTags.h"
   #include "PlagueTaleGO/Audio/BoardAudioSubsystem.h"

   if (UBoardAudioSubsystem* Audio = UBoardAudioSubsystem::Get(this))
   {
       Audio->PlaySoundAtLocation(AudioTags::Xxx, Node->GetActorLocation(), Item->GetItemType());  // contexte optionnel
   }
   ```
   Contexte : `Node->GetNodeType()`, `Pawn->GetEntityType()`, `Item->GetItemType()` (inclure le header du type :
   la conversion vers `UBoardDefinition*` exige la classe complète).
3. L'ajouter au registre (§ 5) et à la liste du guide partagé.

**Son de déplacement d'un pion** (aucun code) : BP du pion > Class Defaults > *Board › Audio* > `Move Sound Event`.
Joué au début de chaque déplacement (`ABoardPawn::BeginMove`), contexte = type de la case de destination.

**Boucle attachée à un pion** (présence d'un ennemi…)
```cpp
// .h : UPROPERTY() TObjectPtr<UAudioComponent> PresenceLoop;
PresenceLoop = Audio->StartLoopAttached(AudioTags::Xxx, GetRootComponent(), GetEntityType());  // BeginPlay
Audio->StopLoop(PresenceLoop, 0.5f);                                                         // quand elle doit cesser
```
La boucle s'arrête seule si le pion est détruit. `EntityType` est renseigné avant `BeginPlay`.

**Faire varier un son selon un type** (aucun code, si l'événement passe un contexte) : Data Asset du type >
*Audio* > `Sound Overrides`. Pas à pas dans le guide partagé.

**Futur menu de volume** : les Sound Classes existent (`SC_Master` > `SC_Music`, `SC_SFX`, `SC_Ambience`, `SC_UI`).
Piste : un Sound Mix + `UGameplayStatics::SetSoundMixClassOverride` / `PushSoundMixModifier` par classe.

**Tester** : commandes console (§ 6).

## 5. Registre des événements sonores

> ⚠️ **Agent : tenir ce tableau à jour dans le même changement que le code** (ajout, déplacement, suppression d'un
> événement), puis la liste du guide partagé. Les événements ne sont pas figés : indiquer le statut.

État au 25/11/2026 (survol et utilisation d'objet ajoutés le même jour) :

| Tag | Déclencheur | Nature | Contexte (varie selon) | Statut |
|---|---|---|---|---|
| `Audio.Player.Footstep` | `ABoardPawn::BeginMove` via `MoveSoundEvent` (réglé dans `BP_PlayerPawn`) | 3D, court | Type de la case de destination | Branché, prototype |
| `Audio.Game.Victory` | `UVictoryState::Enter` | 2D, court | — | Branché |
| `Audio.Game.Defeat` | `UDefeatState::Enter` | 2D, court | — | Branché, pas encore atteignable (`IsPlayerCaught()` = `false`) |
| `Audio.UI.HoverNode` | `UGameFlowSubsystem::PlayHoverSound` (survol d'une case **cliquable** dans l'état courant, sans objet) | 2D, court | Type de la case survolée | Branché |
| `Audio.UI.HoverItem` | `UGameFlowSubsystem::PlayHoverSound` (survol d'une case cliquable qui donne un objet ; remplace `HoverNode`) | 2D, court | Type de l'objet (`UBoardSubsystem::GetItemTypeAt`) | Branché |
| `Audio.Item.Use` | `UItemUseState::PlayUseSound` (début de `ApplyToNodes`, sur la case du joueur) | 3D, court | Type de l'objet | Branché |

Le survol ne joue rien au changement vers une case non cliquable ni quand le joueur ne peut pas agir (`UpdateHover` du
controller n'envoie que les changements de case, et seulement si `CanPlayerAct()`). Pendant le choix d'un objet, passer
d'une case jaune à une autre de la même direction rejoue le son.

**Propositions, non validées par les GD** (ne pas implémenter sans leur accord) : clic refusé (2D), ramassage d'un objet
(3D, type d'objet), déplacement du ver (3D, via `MoveSoundEvent` de `BP_WormPawn`), présence du ver (boucle 3D attachée, type de pion).

## 6. Commandes console

En PIE, console du jeu (touche **²** en AZERTY). Absentes du build Shipping.

| Commande | Effet |
|---|---|
| `BoardAudio.Play <Tag> [3D] [Loop]` | Joue un événement : 2D par défaut, `3D` sur le pion joueur, `Loop` avec `StartLoop*` |
| `BoardAudio.StopLoops` | Arrête toutes les boucles lancées par le subsystem (fondu 0,5 s) |

## 7. Configuration du projet (faite une fois)

| Où | Réglage |
|---|---|
| Project Settings > Game > Board Audio | `Library` = `DA_AudioLibrary` |
| Project Settings > Engine > Audio | `Default Sound Class` = `SC_SFX` |
| Project Settings > Packaging | `Additional Asset Directories to Cook` += `/Game/Audio` (§ 9) |
| `DA_AudioLibrary` | `Default Attenuation` = `ATT_Default` |
| `PlagueTaleGO.Build.cs` | module `DeveloperSettings` (privé) |

`Content/Audio/` : `DA_AudioLibrary`, `Classes/` (`SC_`), `Settings/` (`ATT_`, `SCon_`), `SFX/`, `Ambience/`, `Music/`,
`UI/` (`SW_` Sound Wave, `MSS_` MetaSound Source). Pas de Sound Cue.

## 8. Journal des décisions

| Date | Décision | Raison |
|---|---|---|
| 23/11/2026 | Pas de portage de l'AudioManager Godot : couche fine au-dessus du moteur | Concurrency, variation, bus et recyclage sont natifs dans UE (assets) |
| 23/11/2026 | Clés = Gameplay Tags natifs `Audio.*` (pas d'enum) | Hiérarchie, liste déroulante filtrée, même convention que `Board.Node.*` |
| 23/11/2026 | Une bibliothèque unique (`DA_AudioLibrary`), choisie dans les Project Settings (`UDeveloperSettings`) | Un seul endroit pour tous les sons, modifiable sans code |
| 23/11/2026 | Sons variables selon le contenu via `UBoardDefinition::SoundOverrides` | Suit « le visuel d'une case = son type » ; code mort accepté si inutilisé |
| 23/11/2026 | Sound Classes dès le départ | Seul réglage audio prévu pour le futur menu de volume ; coûteux à ajouter après coup |
| 23/11/2026 | `UWorldSubsystem` : pas de persistance entre niveaux | Choix de l'équipe : niveaux indépendants, pas de musique continue |
| 23/11/2026 | Pas de Sound Cue (MetaSounds si besoin) | `SC_` réservé aux Sound Classes ; MetaSounds remplacent les Cues dans UE5 |
| 23/11/2026 | Registre ici ; guide tout public = document partagé | Choix de l'équipe : tout le monde doit pouvoir intégrer un son |
| 24/11/2026 | Commentaires, logs et aide console en français | Norme du projet (`/Source/PlagueTaleGO/CLAUDE.md`) |
| 24/11/2026 | Son de déplacement générique : `ABoardPawn::MoveSoundEvent` réglé dans le BP | Le plus évolutif : un nouveau pion a son son sans code |
| 24/11/2026 | Ambiance de niveau = acteur Ambient Sound posé dans le niveau | Contrôlée directement par LD / GA ; ne s'arrête pas en fin de partie (à revoir si les GD le demandent) |
| 25/11/2026 | Son de déplacement joué au **début** du déplacement, contexte = case de destination | Demande de l'équipe (`ArrivalSoundEvent` renommé `MoveSoundEvent`) |
| 25/11/2026 | Survol : son seulement sur une case cliquable, 2D ; case avec objet = événement à part (`HoverItem`, contexte = type d'objet) ; joué dans `UGameFlowSubsystem::SubmitHover` (commun à tous les états) | Choix de l'équipe : le son dit « tu peux aller ici » ; le controller reste sans logique de tour |
| 25/11/2026 | Utilisation d'un objet : un seul événement `Item.Use` sur la case du joueur, selon le type d'objet. Le son de case superposé (`Item.UseOnNode`), ajouté le même jour, a été retiré | Choix de l'équipe : aucune situation où le son de case serait utile |

## 9. Pièges UE rencontrés

- **Live Coding et nouvelles `UPROPERTY` / classes** : fermer l'éditeur, compiler, rouvrir.
- **Renommer une `UPROPERTY` réglée dans un BP perd sa valeur** (ex. `ArrivalSoundEvent` → `MoveSoundEvent`) :
  la ressaisir, ou ajouter un `PropertyRedirect` dans `DefaultEngine.ini` (`[CoreRedirects]`).
- **`VisibleInstanceOnly` n'apparaît pas dans les Class Defaults d'un BP** : en PIE, Shift+F1, puis sélectionner
  l'instance dans l'Outliner (équivalent de l'arbre « Remote » de Godot).
- **Référence soft dans un `.ini`** : rien ne garantit que le packaging suive le chemin de la bibliothèque →
  `/Game/Audio` dans *Additional Asset Directories to Cook*, sinon le jeu packagé peut être muet.
- **Son 3D sans atténuation = entendu partout comme un son 2D** → `DefaultAttenuation` de la bibliothèque.
- **Le listener est la caméra**, loin du plateau : une atténuation trop courte rend les sons 3D presque inaudibles
  (point de départ : Inner Radius 3000, Falloff 3000).
- **`Play*` vs `Spawn*`** (`UGameplayStatics`) : seul `Spawn*` renvoie le composant audio, nécessaire pour arrêter une boucle.
- **Supprimer un tag natif encore utilisé dans un asset** (ex. `Audio.Item.UseOnNode` dans `DA_AudioLibrary`) : la clé devient
  invalide (« None ») et le log signale un tag inconnu. Retirer d'abord l'entrée de l'asset, ou la nettoyer après compilation.
- **Conversion vers `UBoardDefinition*`** (contexte) : un type seulement déclaré (`class UBoardNodeType;`) ne se convertit
  pas implicitement vers sa classe de base → inclure son header dans le `.cpp`.
