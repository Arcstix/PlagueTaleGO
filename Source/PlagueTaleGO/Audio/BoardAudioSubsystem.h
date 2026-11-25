#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "BoardAudioSubsystem.generated.h"

class UAudioComponent;
class UBoardAudioLibrary;
class UBoardDefinition;
class USceneComponent;
class USoundAttenuation;
class USoundBase;
struct FBoardSoundEntry;

/**
 * Le lecteur de sons du jeu : SEUL point d'entrée du code pour jouer un son de gameplay.
 * Équivalent Godot : l'autoload AudioManager. Un World Subsystem est créé et détruit avec le niveau :
 * aucun son ne survit à un changement de niveau (choix de l'équipe : pas de musique continue).
 *
 * Responsabilités :
 * - trouver le son d'un événement (tag Audio.*) : d'abord dans les SoundOverrides du contexte
 *   (type de case / de pion / d'objet), sinon dans la bibliothèque (UBoardAudioLibrary) ;
 * - le jouer en 2D ou en 3D, en son court ou en boucle ;
 * - garder la trace des boucles pour pouvoir toutes les arrêter (victoire, défaite).
 *
 * Ce qu'il ne fait PAS, parce que le moteur le fait déjà (réglé sur les assets) :
 * limite de sons simultanés (Sound Concurrency), catégories de volume (Sound Class),
 * recyclage des composants audio, atténuation et spatialisation (Sound Attenuation).
 *
 * Usage depuis le C++ :
 *   if (UBoardAudioSubsystem* Audio = UBoardAudioSubsystem::Get(this))
 *   {
 *       Audio->PlaySoundAtLocation(AudioTags::Player_Footstep, GetActorLocation(), Node->GetNodeType());
 *   }
 */
UCLASS()
class PLAGUETALEGO_API UBoardAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Raccourci : le subsystem du monde de cet objet (nullptr dans l'éditeur hors PIE)
	static UBoardAudioSubsystem* Get(const UObject* WorldContextObject);

	// Créé seulement en jeu et en PIE (pas dans l'éditeur ni les aperçus)
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Sons courts (lancés puis oubliés : le moteur libère tout seul le son à la fin) ---

	// Son non spatialisé (interface, victoire…) : même volume quelle que soit la caméra
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySound2D(FGameplayTag Event, const UBoardDefinition* Context = nullptr);

	// Son spatialisé à une position du monde (pas, impact…)
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySoundAtLocation(FGameplayTag Event, FVector Location, const UBoardDefinition* Context = nullptr);

	// --- Boucles (renvoient le composant audio : le garder dans un UPROPERTY pour l'arrêter) ---

	// Boucle non spatialisée (ambiance, musique)
	UFUNCTION(BlueprintCallable, Category = "Audio")
	UAudioComponent* StartLoop2D(FGameplayTag Event, const UBoardDefinition* Context = nullptr);

	// Boucle spatialisée immobile. Pour un son de décor, préférer un acteur « Ambient Sound » posé dans le niveau.
	UFUNCTION(BlueprintCallable, Category = "Audio")
	UAudioComponent* StartLoopAtLocation(FGameplayTag Event, FVector Location, const UBoardDefinition* Context = nullptr);

	// Boucle spatialisée qui suit un composant (pion…) et s'arrête toute seule s'il est détruit
	UFUNCTION(BlueprintCallable, Category = "Audio")
	UAudioComponent* StartLoopAttached(FGameplayTag Event, USceneComponent* AttachTo, const UBoardDefinition* Context = nullptr);

	// Arrête une boucle, avec un fondu (0 = coupure nette). Sans effet si Loop est nul ou déjà arrêtée.
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopLoop(UAudioComponent* Loop, float FadeOutDuration = 0.5f);

	// Arrête toutes les boucles lancées par ce subsystem (victoire, défaite…)
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopAllLoops(float FadeOutDuration = 0.5f);

private:
	// Entrée à jouer : SoundOverrides du contexte, sinon bibliothèque. nullptr (+ warning une fois par tag) si aucune.
	const FBoardSoundEntry* FindEntry(FGameplayTag Event, const UBoardDefinition* Context);

	// Tire le son à jouer et vérifie la règle boucle / son court. nullptr si on ne doit rien jouer.
	USoundBase* PickSound(const FBoardSoundEntry& Entry, FGameplayTag Event, bool bExpectLoop);

	// Atténuation d'un son 3D : celle de l'entrée, sinon celle de l'asset (nullptr = le moteur la lit), sinon celle par défaut
	USoundAttenuation* ResolveAttenuation(const FBoardSoundEntry& Entry, const USoundBase* Sound) const;

	// Mémorise une boucle (pour StopAllLoops) et la renvoie
	UAudioComponent* RegisterLoop(UAudioComponent* Loop);

	// Warning affiché une seule fois par clé (évite d'inonder le log à chaque pas)
	void WarnOnce(const FString& Key, const FString& Message);

	// --- Données ---

	// Bibliothèque chargée depuis les Project Settings (UBoardAudioSettings)
	UPROPERTY()
	TObjectPtr<UBoardAudioLibrary> Library;

	// Boucles en cours. Faibles (weak) : un composant audio se détruit seul quand sa boucle est arrêtée.
	TArray<TWeakObjectPtr<UAudioComponent>> ActiveLoops;

	// Clés des warnings déjà affichés
	TSet<FString> ReportedWarnings;
};
