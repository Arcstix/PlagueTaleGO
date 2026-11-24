#pragma once

#include "NativeGameplayTags.h"

/**
 * Native Gameplay Tags for sound events: one tag = one game moment that plays a sound.
 *
 * Gameplay code never references a sound directly: it asks UBoardAudioSubsystem to "play Audio.Player.Footstep",
 * and the library (DA_AudioLibrary) decides which sound to play.
 * Godot equivalent: the SOUND_EFFECT_TYPE enum of the AudioManager, but hierarchical
 * (Audio.Player.*, Audio.Game.*) and offered as a dropdown in the editor.
 *
 * - Adding an event: UE_DECLARE here + UE_DEFINE in AudioTags.cpp, call the subsystem where the event happens,
 *   then add it to the sound event registry (Source/PlagueTaleGO/Audio/CLAUDE.md).
 * - A tag with no sound in the library breaks nothing: one [Audio] warning, then silence.
 */
namespace AudioTags
{
	// Player footstep when arriving on a cell (3D, one-shot; context: cell type)
	PLAGUETALEGO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Footstep);

	// Victory: the player reaches an exit (2D, one-shot)
	PLAGUETALEGO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Game_Victory);

	// Defeat: the player is caught (2D, one-shot)
	PLAGUETALEGO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Game_Defeat);
}
