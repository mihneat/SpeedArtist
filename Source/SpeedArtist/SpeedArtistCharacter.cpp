// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpeedArtistCharacter.h"
#include "SpeedArtistProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASpeedArtistCharacter

ASpeedArtistCharacter::ASpeedArtistCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// // Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	// Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	// Mesh1P->SetOnlyOwnerSee(true);
	// Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	// Mesh1P->bCastDynamicShadow = false;
	// Mesh1P->CastShadow = false;
	// //Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	// Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

}

void ASpeedArtistCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////// Input

void ASpeedArtistCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpeedArtistCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpeedArtistCharacter::Look);

		// Confirming
		EnhancedInputComponent->BindAction(ConfirmAction, ETriggerEvent::Triggered, this, &ASpeedArtistCharacter::ConfirmInput);

		// Resetting
		EnhancedInputComponent->BindAction(ResetAction, ETriggerEvent::Triggered, this, &ASpeedArtistCharacter::ResetInput);
		
		// Drawing
		EnhancedInputComponent->BindAction(DrawAction, ETriggerEvent::Started, this, &ASpeedArtistCharacter::StartDrawingInput);
		EnhancedInputComponent->BindAction(DrawAction, ETriggerEvent::Triggered, this, &ASpeedArtistCharacter::DrawInput);
		EnhancedInputComponent->BindAction(DrawAction, ETriggerEvent::Completed, this, &ASpeedArtistCharacter::StopDrawingInput);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void ASpeedArtistCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ASpeedArtistCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASpeedArtistCharacter::StartDrawingInput(const FInputActionValue& Value)
{
	// UE_LOG(LogTemp, Display, TEXT("[ASpeedArtistCharacter] Started drawing!"));
	
	// TODO: Mark the beginning of a stroke
}

void ASpeedArtistCharacter::DrawInput(const FInputActionValue& Value)
{
	// UE_LOG(LogTemp, Display, TEXT("[ASpeedArtistCharacter] Drawing.."));
	
	// TODO: Paint the canvas and store the points
}

void ASpeedArtistCharacter::StopDrawingInput(const FInputActionValue& Value)
{
	// UE_LOG(LogTemp, Display, TEXT("[ASpeedArtistCharacter] Stopped drawing!!"));
	
	// TODO: End the current stroke
}

void ASpeedArtistCharacter::ConfirmInput(const FInputActionValue& Value)
{
	// UE_LOG(LogTemp, Display, TEXT("[ASpeedArtistCharacter] Confirmed!"));
}

void ASpeedArtistCharacter::ResetInput(const FInputActionValue& Value)
{
	// UE_LOG(LogTemp, Display, TEXT("[ASpeedArtistCharacter] Reset!"));
}
