// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GameStartWidget.h"

#include "CanvasManager.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void UGameStartWidget::SubscribeToConfirm()
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwningPlayer()->GetCharacter()->InputComponent))
	{
		// Confirming
		EnhancedInputComponent->BindAction(ConfirmAction, ETriggerEvent::Triggered, this, &UGameStartWidget::OnConfirm);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("[UGameStartWidget] No input component found"));
	}
}

void UGameStartWidget::OnConfirm()
{
	if (HasConfirmed)
		return;
	
	// const AActor* CanvasManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), CanvasManagerActorClass);
	// if (!IsValid(CanvasManagerActor))
	// {
	// 	UE_LOG(LogTemp, Display, TEXT("[UGameStartWidget] No canvas manager actor found"));
	// 	return;
	// }
	// 	
	// UCanvasManager* CanvasManager = CanvasManagerActor->GetComponentByClass<UCanvasManager>();
	// if (CanvasManager == nullptr || !IsValid(CanvasManager))
	// {
	// 	UE_LOG(LogTemp, Display, TEXT("[UGameStartWidget] No canvas manager component found"));
	// 	return;
	// }
	//
	// CanvasManager->StartGame();

	HasConfirmed = true;
	RemoveFromParent();
}
