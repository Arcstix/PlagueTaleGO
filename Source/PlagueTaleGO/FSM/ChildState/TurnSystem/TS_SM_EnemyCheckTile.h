
#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/FSM/StateBase.h"
#include "TS_SM_EnemyCheckTile.generated.h"

/**
 * 
 */
UCLASS()
class PLAGUETALEGO_API UTS_SM_EnemyCheckTile : public UStateBase
{
	GENERATED_BODY()
	
public:
	
	void PrimaryInput() override;
	
};
