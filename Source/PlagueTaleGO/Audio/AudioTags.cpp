#include "AudioTags.h"

namespace AudioTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Player_Footstep, "Audio.Player.Footstep", "Pas du joueur au début de chaque déplacement (3D). Varie selon le type de la case de destination.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Game_Victory, "Audio.Game.Victory", "Victoire : le joueur atteint une sortie (2D).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Game_Defeat, "Audio.Game.Defeat", "Défaite : le joueur est attrapé (2D).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_HoverNode, "Audio.UI.HoverNode", "Survol d'une case cliquable sans objet (2D). Varie selon le type de la case.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_HoverItem, "Audio.UI.HoverItem", "Survol d'une case cliquable qui donne un objet (2D). Varie selon le type de l'objet.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Use, "Audio.Item.Use", "Utilisation d'un objet, sur la case du joueur (3D). Varie selon le type de l'objet.");
}
