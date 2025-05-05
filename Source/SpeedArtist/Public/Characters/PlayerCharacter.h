// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpeedArtist/SpeedArtistCharacter.h"
#include "PlayerCharacter.generated.h"

class UCanvasManager;
class UCanvasDrawer;
/**
 * 
 */
UCLASS()
class SPEEDARTIST_API APlayerCharacter : public ASpeedArtistCharacter
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
	virtual void Move(const FInputActionValue& Value) override;
	virtual void Look(const FInputActionValue& Value) override;
	virtual void StartDrawingInput(const FInputActionValue& Value) override;
	virtual void DrawInput(const FInputActionValue& Value) override;
	virtual void StopDrawingInput(const FInputActionValue& Value) override;
	virtual void ConfirmInput(const FInputActionValue& Value) override;
	virtual void ResetInput(const FInputActionValue& Value) override;

public:
	void ToggleDrawingMode(bool bEnterDrawingMode);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartDrawing, APlayerCharacter*, Player);
	UPROPERTY(BlueprintAssignable)
	FOnStartDrawing OnStartDrawing;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDraw, APlayerCharacter*, Player);
	UPROPERTY(BlueprintAssignable)
	FOnDraw OnDraw;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStopDrawing, APlayerCharacter*, Player);
	UPROPERTY(BlueprintAssignable)
	FOnStopDrawing OnStopDrawing;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConfirm, APlayerCharacter*, Player);
	UPROPERTY(BlueprintAssignable)
	FOnConfirm OnConfirm;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReset, APlayerCharacter*, Player);
	UPROPERTY(BlueprintAssignable)
	FOnReset OnReset;

private:
	APlayerController* PlayerController = nullptr;
	UCanvasDrawer* CanvasDrawerComp = nullptr;
	
	bool bIsLockedInDrawingMode = false;
	bool bIsDrawing = false;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> CanvasManagerActorClass;

	UCanvasManager* CanvasManager;
	
};
