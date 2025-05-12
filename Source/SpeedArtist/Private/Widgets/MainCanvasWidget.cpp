// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MainCanvasWidget.h"

#include "Components/Throbber.h"

void UMainCanvasWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ObjectToDrawTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("ObjectToDrawText")));
	ObjectToDrawTextBlock->SetText(FText::FromString("-"));

	PredictedClassTextBlock = Cast<UTextBlock>(GetWidgetFromName(TEXT("PredictedClassText")));
	PredictedClassTextBlock->SetText(FText::FromString("-"));

	Throbber = Cast<UThrobber>(GetWidgetFromName(TEXT("Throbber")));
}

void UMainCanvasWidget::SetObjectToDraw(const FString& ObjectName)
{
	ObjectToDrawTextBlock->SetText(FText::FromString(ObjectName));
}

void UMainCanvasWidget::ResetObjectToDraw()
{
	ObjectToDrawTextBlock->SetText(FText::FromString("-"));
}

void UMainCanvasWidget::StartPrediction()
{
	// Start the throbber
	Throbber->SetRenderOpacity(1.0f);
	PredictedClassTextBlock->SetText(FText::FromString(""));
}

void UMainCanvasWidget::EndPrediction(const FString& PredictionName)
{
	// Stop the throbber
	Throbber->SetRenderOpacity(0.0f);
	PredictedClassTextBlock->SetText(FText::FromString(PredictionName));
}

void UMainCanvasWidget::ResetPrediction()
{
	if (!IsValid(PredictedClassTextBlock))
	{
		UE_LOG(LogTemp, Error, TEXT("[UMainCanvasWidget] Invalid predicted class text block"));
		return;
	}
	PredictedClassTextBlock->SetText(FText::FromString("-"));
}
