// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ANode.generated.h"

class UStaticMeshComponent;
class AAConnection;

UCLASS()
class PLAGUETALEGO_API AANode : public AActor
{
	GENERATED_BODY()

private:
	bool bIsOccupied = false;
	bool bIsBlocked = false;
	bool bIsSwipable = false;
	bool bIsClickable = false;
	
public:	
	// Sets default values for this actor's properties
	AANode();
	
	UPROPERTY(VisibleAnywhere, Category = "0 - Parameters")
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(EditAnywhere, Category= "0 - Parameters")
	TArray<AANode*> m_adjacentNodes;

	UPROPERTY(EditAnywhere, Category= "0 - Parameters")
	TArray<AAConnection*> m_connections;

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
	virtual void OnConstruction(const FTransform& Transform);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsConnectedTo(AANode* node);

};
