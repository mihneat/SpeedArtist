// Fill out your copyright notice in the Description page of Project Settings.


#include "CanvasManager.h"

#include "CanvasArea.h"
#include "Blueprint/UserWidget.h"
#include "Characters/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/MainCanvasWidget.h"

// Thanks to: https://forums.unrealengine.com/t/multithreading-and-performance-in-unreal/1216417
FRunOutputThroughModel::FRunOutputThroughModel(FString FullFilePath, UCanvasManager* CanvasManagerRef)
{
	Thread = FRunnableThread::Create(this, TEXT("Model runner"));
	FilePath = FullFilePath;
	CanvasManager = CanvasManagerRef;
}

bool FRunOutputThroughModel::Init()
{
	return true;
}

uint32 FRunOutputThroughModel::Run()
{
	UE_LOG(LogTemp, Display, TEXT("[FRunOutputThroughModel] Running the painting through the RNN model..."));
	
	FString Cmd = TEXT("C:\\Users\\mihne\\anaconda3\\envs\\SpeedArtist-PyTorch\\python.exe");
	FString Params = FString("PaintingRater.py \"").Append(FilePath).Append("\"");
	FString WorkingDir = TEXT("G:\\Python\\SpeedArtist-PyTorch");
	uint32 ProcId = 0;

	// void* ReadPipe = nullptr;
	// void* WritePipe = nullptr;
	// FWindowsPlatformProcess::CreatePipe(ReadPipe, WritePipe);
	// FProcHandle ProcHandle = FWindowsPlatformProcess::CreateProc(*Cmd, *Params, true, false, false, &ProcId,  0, *WorkingDir, nullptr);

	int32 ReturnCode = 0;
	FString ProcStdout;
	FString ProcStderr;
	bool ExecProcessResult = FWindowsPlatformProcess::ExecProcess(*Cmd, *Params, &ReturnCode, &ProcStdout, &ProcStderr, *WorkingDir);
	UE_LOG(LogTemp, Display, TEXT("[UCanvasManager] Exec process result: %d"), ExecProcessResult);
	UE_LOG(LogTemp, Display, TEXT("[UCanvasManager] Exec process stdout: %s"), *ProcStdout);
	UE_LOG(LogTemp, Display, TEXT("[UCanvasManager] Exec process stderr: %s"), *ProcStderr);
	UE_LOG(LogTemp, Display, TEXT("[UCanvasManager] Exec process return code: %d"), ReturnCode);

	// Setup variables for next tick
	CanvasManager->ReceivedNewEvaluationResult = true;
	CanvasManager->ResponseCode = ReturnCode;
	CanvasManager->StandardOutput = ProcStdout;
	CanvasManager->StandardError = ProcStderr;
	
	return 0;
}

void FRunOutputThroughModel::Stop()
{
	
}

// Sets default values for this component's properties
UCanvasManager::UCanvasManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCanvasManager::BeginPlay()
{
	Super::BeginPlay();

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!IsValid(PlayerCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("[UCanvasManager] Unable to find PlayerCharacter in the world"));
		return;
	}

	PlayerCharacter->OnConfirm.AddDynamic(this, &UCanvasManager::HandleOnConfirm);
	PlayerCharacter->OnReset.AddDynamic(this, &UCanvasManager::HandleOnReset);
	
	PlayerCharacter->OnStartDrawing.AddDynamic(this, &UCanvasManager::HandleOnStartDrawing);
	PlayerCharacter->OnDraw.AddDynamic(this, &UCanvasManager::HandleOnDraw);
	PlayerCharacter->OnStopDrawing.AddDynamic(this, &UCanvasManager::HandleOnStopDrawing);
}

// void UCanvasManager::StartGame()
// {
// 	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UCanvasManager::StartGameDelayed);
// }

void UCanvasManager::StartGame()
{
	// Instantiate the game widget
	const auto Widget = CreateWidget(UGameplayStatics::GetPlayerController(GetWorld(), 0), MainWidgetBP);
	if (!IsValid(Widget))
		return;
	
	MainCanvasWidget = Cast<UMainCanvasWidget>(Widget);
	MainCanvasWidget->AddToViewport();

	// CurrentDrawingState = Drawing;
	// BeginRound();
}

void UCanvasManager::BeginRound()
{
	// Choose a random class
	ChooseRandomClass();

	MainCanvasWidget->SetObjectToDraw(CurrentClass);
	
	CanvasArea->ClearCanvas();
	
	MainCanvasWidget->ResetPrediction();
}

void UCanvasManager::EndRound()
{
	CurrentDrawingState = RoundEnded;
}


// Called every frame
void UCanvasManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCanvasManager::HandleOnConfirm(APlayerCharacter* Player)
{
	if (CanvasArea == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UCanvasManager] Canvas Area not found"));
		return;
	}
	
	if (CurrentDrawingState == Evaluating)
		return;

	if (CurrentDrawingState == WaitingForStart || CurrentDrawingState == RoundEnded)
	{
		if (CurrentDrawingState == WaitingForStart)
			StartGame();
		
		BeginRound();

		CurrentDrawingState = Drawing;
		
		return;
	}

	if (CurrentDrawingState == Drawing)
	{
		// Get the FPainting from the CanvasArea
		FPainting& Painting = CanvasArea->GetCurrentPainting();
	
		// Simplify each stroke
		Painting.Simplify(2.0f);
	
		// Write the stroke data to a file, along with the class
		TArray<FString> StrokeData {
			// Add a test input
			// "{\"word\":\"axe\",\"drawing\":[[[0,43,62],[59,82,104]],[[2,25,118],[56,39,2]],[[118,118,131,142,73,53],[2,22,83,103,104,101]],[[120,122],[0,9]],[[122,124,153,204,255,252,242,221,207,190,178],[9,60,113,169,213,219,223,205,199,186,173]]]}"
		};

		if (Painting.Strokes.Num() > 0)
			StrokeData.Add(CreateModelInputJson(Painting, CurrentClass));

		// Get the path to the file
		FString FullContentPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectContentDir());
		FString RootPath = FullContentPath.Append("PaintingHistory/");
		FString FilePath = RootPath.Append("Painting_0.ndjson");
	
		FFileHelper::SaveStringArrayToFile(StrokeData, *FilePath);
	
		// Start a python process that reads the data, loads the model and predicts the class
		FRunOutputThroughModel* RunOutputThroughModel = new FRunOutputThroughModel(FilePath, this);
		RunOutputThroughModel->Init();

		CurrentDrawingState = Evaluating;
		MainCanvasWidget->StartPrediction();

		GetWorld()->GetTimerManager().SetTimer(CurrentTimerHandle, this, &UCanvasManager::CheckEvaluationDone, 0.5f, true);
	}
}

void UCanvasManager::CheckEvaluationDone()
{
	UE_LOG(LogTemp, Display, TEXT("[UCanvasManager] Evaluating.."));
	if (ReceivedNewEvaluationResult)
	{
		ReceivedNewEvaluationResult = false;

		GetWorld()->GetTimerManager().ClearTimer(CurrentTimerHandle);
		
		ProcessEvaluationResult();
	}
}

void UCanvasManager::HandleOnReset(APlayerCharacter* Player)
{
	if (CanvasArea == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UCanvasManager] Canvas Area not found"));
		return;
	}

	if (CurrentDrawingState != Drawing)
		return;

	CanvasArea->ClearCanvas();
}

void UCanvasManager::HandleOnStartDrawing(APlayerCharacter* Player)
{
	if (CanvasArea == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UCanvasManager] Canvas Area not found"));
		return;
	}

	if (CurrentDrawingState != Drawing)
		return;

	CanvasArea->StartDrawing();
}

void UCanvasManager::HandleOnDraw(APlayerCharacter* Player)
{
	if (CanvasArea == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UCanvasManager] Canvas Area not found"));
		return;
	}

	if (CurrentDrawingState != Drawing)
		return;

	CanvasArea->Draw();
}

void UCanvasManager::HandleOnStopDrawing(APlayerCharacter* Player)
{
	if (CanvasArea == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UCanvasManager] Canvas Area not found"));
		return;
	}

	if (CurrentDrawingState != Drawing)
		return;

	CanvasArea->StopDrawing();
}

FString UCanvasManager::CreateModelInputJson(FPainting Painting, FString ClassName)
{
	const FString ModelInputJson = FString::Format(TEXT("{\"word\":\"{0}\",\"drawing\":{1}}"), { *ClassName, *Painting.Serialize() });
	UE_LOG(LogTemp, Display, TEXT("[UCanvasManager] %s"), *ModelInputJson);
	
	return ModelInputJson;
}

void UCanvasManager::ChooseRandomClass()
{
	const int RandClassIndex = FMath::RandRange(0, Classes.Num() - 1);
	CurrentClass = Classes[RandClassIndex];

	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Emerald, FString::Format(TEXT("Object to draw: {0}"), { CurrentClass }));

	UE_LOG(LogTemp, Display, TEXT("[UCanvasManager] Object to draw: %s"), *CurrentClass);
}

void UCanvasManager::ProcessEvaluationResult()
{
	EndRound();
	
	if (ResponseCode != 0)
	{
		// Display an error
		UE_LOG(LogTemp, Display, TEXT("[UCanvasManager] AN ERROR OCCURED DURING EVALUATION"));
		
		return;
	}

	// Parse the standard output and extract the result
	TArray<FString> lines;
	StandardOutput.ParseIntoArrayLines(lines);

	FString LastOutputLine = *lines.rbegin();
	UE_LOG(LogTemp, Display, TEXT("[UCanvasManager] FINAL OUTPUT LINE: %s"), *LastOutputLine);

	MainCanvasWidget->EndPrediction(LastOutputLine);
}

