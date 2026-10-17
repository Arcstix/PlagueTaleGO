
#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/FSM/StateBase.h"
#include "TS_SM_CheckVictoryCondition.generated.h"

/**
 * 
 */
UCLASS()
class PLAGUETALEGO_API UTS_SM_CheckVictoryCondition : public UStateBase
{
	GENERATED_BODY()
	
public:
	
	void PrimaryInput() override;
	
};
