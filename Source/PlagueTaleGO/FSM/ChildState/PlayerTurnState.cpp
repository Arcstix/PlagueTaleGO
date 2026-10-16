// PlayerTurnState.cpp
#include "PlayerTurnState.h"

TSubclassOf<UStateBase> UPlayerTurnState::GetNextState()
{
	// Proto : tant que l'état ennemi n'existe pas, le tour joueur se relance
	return UPlayerTurnState::StaticClass();
}