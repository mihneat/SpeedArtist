// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpeedArtistGameMode.h"
#include "Characters/PlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASpeedArtistGameMode::ASpeedArtistGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

void ASpeedArtistGameMode::HandleOnConfirm(APlayerCharacter* Player)
{
	// UE_LOG(LogTemp, Display, TEXT("[ASpeedArtistGameMode] Received player: %s"), *Player->GetName());
}
