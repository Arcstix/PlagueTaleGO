	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoardNode.generated.h"

class UStaticMeshComponent;
class ABoardConnection;

UCLASS()
class PLAGUETALEGO_API ABoardNode : public AActor
{
	GENERATED_BODY()

private:
	bool bIsOccupied = false;
	bool bIsBlocked = false;
	bool bIsSwipable = false;
	bool bIsClickable = false;
	
public:	
	// Sets default values for this actor's properties
	ABoardNode();
	// Ajoute la relation de voisinage (sans doublon, sans auto-référence)
	void AddAdjacent(ABoardNode* Other);
	
	UPROPERTY(EditAnywhere, Category= "0 - Parameters")
	TArray<ABoardNode*> AdjacentNodes;

	UPROPERTY(EditAnywhere, Category= "0 - Parameters")
	TArray<ABoardConnection*> Connections;

	bool IsOccupied() const;
	void SetOccupied(bool bValue);

	bool IsBlocked() const;
	void SetBlocked(bool bValue);

	bool IsSwipable() const;
	void SetSwipable(bool bValue);

	bool IsClickable() const;
	void SetClickable(bool bValue);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsConnectedTo(ABoardNode* node);

};
