// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameStartWidget.generated.h"

class UInputAction;
/**
 * 
 */
UCLASS()
class SPEEDARTIST_API UGameStartWidget : public UUserWidget
{
	GENERATED_BODY()

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* ConfirmAction;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> CanvasManagerActorClass;

private:
	UFUNCTION(BlueprintCallable)
	void SubscribeToConfirm();
	void OnConfirm();

	bool HasConfirmed = false;
};
