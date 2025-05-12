// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CanvasManager.generated.h"


class UCanvasManager;
class UMainCanvasWidget;
struct FPainting;
class ACanvasArea;
class APlayerCharacter;

enum EDrawingState
{
	WaitingForStart,
	Drawing,
	Evaluating,
	RoundEnded
};

class SPEEDARTIST_API FRunOutputThroughModel : public FRunnable
{
public:
	FRunOutputThroughModel(FString FullFilePath, UCanvasManager* CanvasManagerRef);
	
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

	FRunnableThread* Thread = nullptr;
	FString FilePath;
	UCanvasManager* CanvasManager;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPEEDARTIST_API UCanvasManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCanvasManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void BeginRound();
	void EndRound();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void StartGame();

	UPROPERTY(EditAnywhere, Category="Widget References")
	TSubclassOf<UUserWidget> MainWidgetBP;
	UMainCanvasWidget* MainCanvasWidget;

private:
	UFUNCTION(BlueprintCallable)
	void HandleOnConfirm(APlayerCharacter* Player);
	void CheckEvaluationDone();

	UFUNCTION(BlueprintCallable)
	void HandleOnReset(APlayerCharacter* Player);
	
	UFUNCTION(BlueprintCallable)
	void HandleOnStartDrawing(APlayerCharacter* Player);
	
	UFUNCTION(BlueprintCallable)
	void HandleOnDraw(APlayerCharacter* Player);
	
	UFUNCTION(BlueprintCallable)
	void HandleOnStopDrawing(APlayerCharacter* Player);

	FString CreateModelInputJson(FPainting Painting, FString ClassName);

	UPROPERTY(EditInstanceOnly)
	ACanvasArea* CanvasArea;

	void StartGameDelayed();
	void ChooseRandomClass();

	void ProcessEvaluationResult();

	TArray<FString> Classes{ "airplane", "ant", "axe", "bed" };
	FString CurrentClass;

	EDrawingState CurrentDrawingState = WaitingForStart;

	bool ReceivedNewEvaluationResult = false;
	int ResponseCode = 0;
	FString StandardOutput;
	FString StandardError;

	FTimerHandle CurrentTimerHandle;

	friend class FRunOutputThroughModel;
};
