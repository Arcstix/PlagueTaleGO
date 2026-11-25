#pragma once

#include "NativeGameplayTags.h"

/**
 * Gameplay Tags « natifs » des événements sonores : un tag = un moment du jeu qui joue un son.
 *
 * Le code ne connaît jamais un son directement : il demande « joue Audio.Player.Footstep »
 * à UBoardAudioSubsystem, et c'est la bibliothèque (DA_AudioLibrary) qui dit quel son jouer.
 * Équivalent Godot : l'enum SOUND_EFFECT_TYPE de l'AudioManager, mais hiérarchique
 * (Audio.Player.*, Audio.Game.*) et proposé en liste déroulante dans l'éditeur.
 *
 * - Ajouter un événement : UE_DECLARE ici + UE_DEFINE dans AudioTags.cpp, appeler le subsystem
 *   là où l'événement a lieu, puis l'ajouter au registre des événements (Source/PlagueTaleGO/Audio/CLAUDE.md).
 * - Un tag sans son dans la bibliothèque ne casse rien : warning [Audio] une fois, puis silence.
 */
namespace AudioTags
{
	// Pas du joueur au début de chaque déplacement (3D, son court ; contexte : type de la case de destination).
	// Joué via ABoardPawn::MoveSoundEvent, réglé dans BP_PlayerPawn
	PLAGUETALEGO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Footstep);

	// Victoire : le joueur atteint une sortie (2D, son court)
	PLAGUETALEGO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Game_Victory);

	// Défaite : le joueur est attrapé (2D, son court)
	PLAGUETALEGO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Game_Defeat);
}
