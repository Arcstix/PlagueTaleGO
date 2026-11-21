// BoardPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BoardPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class ABoardNode;
class ABoardCameraRig;
struct FInputActionValue;
class ABoardPawn;

/**
 * Transforme les entrées du joueur en intentions pour UGameFlowSubsystem.
 * Ne contient AUCUNE logique de tour : il ne sait pas dans quel état est la partie.
 */
UCLASS()
class PLAGUETALEGO_API ABoardPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Rig de caméra du niveau (nullptr si la vue est un simple CameraActor)
	ABoardCameraRig* GetCameraRig() const { return CameraRig; }
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "MappingContext")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// Clic gauche / toucher : choisir une case
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SelectAction;

	// Molette : changer de point de vue
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CameraViewAction;

	// Tag de la caméra du plateau à placer dans le niveau
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FName BoardCameraTag = TEXT("BoardCamera");

private:
	// Clic gauche : case jouable → intention, sinon → glisser caméra
	void HandleSelectStarted();
	void HandleSelectCompleted();

	// Le pion joueur est créé par le GameMode après le BeginPlay du controller : on s'y abonne dès qu'il existe
	void TryBindPlayerPawn();

	UFUNCTION()   // obligatoire pour AddDynamic
	void HandlePlayerMoveFinished(ABoardPawn* MovedPawn);

	TWeakObjectPtr<ABoardPawn> BoundPlayerPawn;

	bool bCameraDragging = false;

	// Position du curseur à l'image précédente, pour calculer le mouvement du glisser
	FVector2D LastCursorPosition = FVector2D::ZeroVector;

	// Faux tant qu'aucune position de curseur n'a été lue depuis le début du glisser
	bool bHasLastCursorPosition = false;

	// Envoie au rig le mouvement du curseur depuis l'image précédente
	void UpdateCameraDrag();
	
	void SetupBoardCamera();

	// Vérifie la case sous le curseur et prévient le conducteur si elle a changé
	void UpdateHover();

	// Dernière case signalée (faible : le controller ne garde pas la case en vie)
	TWeakObjectPtr<ABoardNode> HoveredNode;

	ABoardNode* GetNodeUnderCursor() const;

	UPROPERTY()
	TObjectPtr<ABoardCameraRig> CameraRig;

	void HandleCameraView(const FInputActionValue& Value);
};
