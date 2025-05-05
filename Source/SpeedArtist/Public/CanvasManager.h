// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CanvasManager.generated.h"


struct FPainting;
class ACanvasArea;
class APlayerCharacter;

class SPEEDARTIST_API FRunOutputThroughModel : public FRunnable
{
public:
	FRunOutputThroughModel(FString FullFilePath);
	
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

	FRunnableThread* Thread = nullptr;
	FString FilePath;
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

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void NotifyCreated();

private:
	UFUNCTION(BlueprintCallable)
	void HandleOnConfirm(APlayerCharacter* Player);
	
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

	void ChooseRandomClass();

	TArray<FString> Classes{ "airplane", "ant", "axe", "bed" };
	FString CurrentClass;
};
