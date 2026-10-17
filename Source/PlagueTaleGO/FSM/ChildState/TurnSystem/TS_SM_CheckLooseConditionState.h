
#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/FSM/StateBase.h"
#include "TS_SM_CheckLooseConditionState.generated.h"

/**
 * 
 */
UCLASS()
class PLAGUETALEGO_API UTS_SM_CheckLooseConditionState : public UStateBase
{
	GENERATED_BODY()
	
public:
	
	void PrimaryInput() override;
	
};
