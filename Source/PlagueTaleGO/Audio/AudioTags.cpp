#include "AudioTags.h"

namespace AudioTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Player_Footstep, "Audio.Player.Footstep", "Player footstep when arriving on a cell (3D). Varies with the cell type.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Game_Victory, "Audio.Game.Victory", "Victory: the player reaches an exit (2D).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Game_Defeat, "Audio.Game.Defeat", "Defeat: the player is caught (2D).");
}
