// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardItemPickup.h"
#include "Components/StaticMeshComponent.h"
#include "PlagueTaleGO/Level/Definitions/BoardItemType.h"


// Sets default values
ABoardItemPickup::ABoardItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	RootComponent = PickupMesh;

	// Ne doit pas intercepter le clic destiné à la case en dessous
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABoardItemPickup::Setup(UBoardItemType* InItemType)
{
	ItemType = InItemType;
	if (!ItemType || !PickupMesh)
	{
		return;
	}

	PickupMesh->SetStaticMesh(ItemType->PickupMesh);
	PickupMesh->SetRelativeScale3D(ItemType->PickupScale);

	// Surélève le visuel au-dessus de la case
	AddActorWorldOffset(FVector(0.f, 0.f, HeightOffset));
}
