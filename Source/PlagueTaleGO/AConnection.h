// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AConnection.generated.h"

class AANode;
class UStaticMeshComponent;

UCLASS()
class PLAGUETALEGO_API AAConnection : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAConnection();
	
	UPROPERTY(EditAnywhere, Category= "Parameters")
	AANode* node1;
	UPROPERTY(EditAnywhere, Category= "Parameters")
	AANode* node2;

	UPROPERTY(EditAnywhere, Category= "Parameters")
	float m_lineWidth = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Parameters")
	FColor m_color = FColor::Black;

	UPROPERTY(EditAnywhere, Category= "Parameters")
	float m_offsetFromNode = 0.2f;

private:
	TArray<FVector> m_Points;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	AANode* GetNode1() const;
	AANode* GetNode2() const;

	void ConnectNode();
	
};
