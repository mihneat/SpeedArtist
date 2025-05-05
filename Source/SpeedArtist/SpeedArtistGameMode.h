// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpeedArtistGameMode.generated.h"

class APlayerCharacter;
class ASpeedArtistCharacter;

UCLASS(minimalapi)
class ASpeedArtistGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASpeedArtistGameMode();

	UFUNCTION(BlueprintCallable)
	void HandleOnConfirm(APlayerCharacter* Player);
};



