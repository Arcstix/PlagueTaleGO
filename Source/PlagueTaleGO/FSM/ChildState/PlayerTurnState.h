// PlayerTurnState.h
#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/FSM/StateBase.h"
#include "PlayerTurnState.generated.h"

UCLASS()
class PLAGUETALEGO_API UPlayerTurnState : public UStateBase
{
	GENERATED_BODY()

public:
	virtual bool AllowsPlayerInput() const override { return true; }
	virtual TSubclassOf<UStateBase> GetNextState() override;
};