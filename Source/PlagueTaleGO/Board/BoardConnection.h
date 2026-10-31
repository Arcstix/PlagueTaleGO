// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoardConnection.generated.h"

class ABoardNode;
class UStaticMeshComponent;

UCLASS()
class PLAGUETALEGO_API ABoardConnection : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoardConnection();
	
	UPROPERTY(EditAnywhere, Category= "0 - Parameters")
	TObjectPtr<ABoardNode> NodeA;
	UPROPERTY(EditAnywhere, Category= "0 - Parameters")
	TObjectPtr<ABoardNode> NodeB;

	UPROPERTY(EditAnywhere, Category= "0 - Parameters")
	float ConnectionWidth = 0.2f;

	UPROPERTY(EditAnywhere, Category = "0 - Parameters")
	float ZOffset = -1;

	UPROPERTY(EditAnywhere, Category= "0 - Parameters")
	float OffsetFromNode = 0.2f;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	ABoardNode* GetNode1() const;
	ABoardNode* GetNode2() const;

	void ConnectNode();
};
