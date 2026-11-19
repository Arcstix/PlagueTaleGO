// BoardPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BoardPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class ABoardNode;

/**
 * Transforme les entrées du joueur en intentions pour UGameFlowSubsystem.
 * Ne contient AUCUNE logique de tour : il ne sait pas dans quel état est la partie.
 */
UCLASS()
class PLAGUETALEGO_API ABoardPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "MappingContext")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// Clic gauche / toucher : choisir une case
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SelectAction;

	// Tag de la caméra du plateau à placer dans le niveau
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FName BoardCameraTag = TEXT("BoardCamera");

private:
	void HandleSelect();
	void SetupBoardCamera();

	// Vérifie la case sous le curseur et prévient le conducteur si elle a changé
	void UpdateHover();

	// Dernière case signalée (faible : le controller ne garde pas la case en vie)
	TWeakObjectPtr<ABoardNode> HoveredNode;

	ABoardNode* GetNodeUnderCursor() const;
};
