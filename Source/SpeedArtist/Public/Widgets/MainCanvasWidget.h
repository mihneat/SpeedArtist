// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "MainCanvasWidget.generated.h"

class UThrobber;
/**
 * 
 */
UCLASS()
class SPEEDARTIST_API UMainCanvasWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

	UTextBlock* ObjectToDrawTextBlock;
	UTextBlock* PredictedClassTextBlock;
	UThrobber* Throbber;
	
public:
	void SetObjectToDraw(const FString& ObjectName);
	void ResetObjectToDraw();
	
	void StartPrediction();
	void EndPrediction(const FString& PredictionName);
	void ResetPrediction();
	
};
