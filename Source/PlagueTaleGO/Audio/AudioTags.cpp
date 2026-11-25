#include "AudioTags.h"

namespace AudioTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Player_Footstep, "Audio.Player.Footstep", "Pas du joueur au début de chaque déplacement (3D). Varie selon le type de la case de destination.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Game_Victory, "Audio.Game.Victory", "Victoire : le joueur atteint une sortie (2D).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Game_Defeat, "Audio.Game.Defeat", "Défaite : le joueur est attrapé (2D).");
}
