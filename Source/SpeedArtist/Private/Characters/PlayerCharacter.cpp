// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"

#include "CanvasDrawer.h"
#include "CanvasManager.h"
#include "Kismet/GameplayStatics.h"

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	const AActor* CanvasManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), CanvasManagerActorClass);
	if (!IsValid(CanvasManagerActor))
		return;
		
	CanvasManager = CanvasManagerActor->GetComponentByClass<UCanvasManager>();
	if (!IsValid(CanvasManager))
		return;

	PlayerController = GetController<APlayerController>();
	CanvasDrawerComp = GetComponentByClass<UCanvasDrawer>();

	ToggleDrawingMode(true);
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (bIsLockedInDrawingMode)
		return;
	
	Super::Move(Value);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	if (bIsLockedInDrawingMode)
		return;
	
	Super::Look(Value);
}

void APlayerCharacter::StartDrawingInput(const FInputActionValue& Value)
{
	Super::StartDrawingInput(Value);

	if (!IsValid(CanvasDrawerComp))
		return;
	
	CanvasDrawerComp->StartDrawing();

	OnStartDrawing.Broadcast(this);
}

void APlayerCharacter::DrawInput(const FInputActionValue& Value)
{
	Super::DrawInput(Value);

	if (!IsValid(CanvasDrawerComp))
		return;
	
	CanvasDrawerComp->Draw();

	OnDraw.Broadcast(this);
}

void APlayerCharacter::StopDrawingInput(const FInputActionValue& Value)
{
	Super::StopDrawingInput(Value);

	if (!IsValid(CanvasDrawerComp))
		return;
	
	CanvasDrawerComp->StopDrawing();

	OnStopDrawing.Broadcast(this);
}

void APlayerCharacter::ConfirmInput(const FInputActionValue& Value)
{
	Super::ConfirmInput(Value);

	OnConfirm.Broadcast(this);
}

void APlayerCharacter::ResetInput(const FInputActionValue& Value)
{
	Super::ResetInput(Value);

	OnReset.Broadcast(this);
}

void APlayerCharacter::ToggleDrawingMode(const bool bEnterDrawingMode)
{	
	PlayerController->bShowMouseCursor = bEnterDrawingMode;
	bIsLockedInDrawingMode = bEnterDrawingMode;
	
	bIsDrawing = false;
}
