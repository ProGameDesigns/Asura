// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "AsuraPlayerController.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "AsuraCharacter.h"

#include "AsuraPlayerHUD.h"
#include "AsuraPlayerState.h"
#include "Characters/MarauderCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Menus/CharacterSelectionUI.h"

AAsuraPlayerController::AAsuraPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;

	

}

void AAsuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AsuraPlayerState = Cast<AAsuraPlayerState>(PlayerState);

	if (IsLocalPlayerController())
	{
		AsuraPlayerHUD = Cast<AAsuraPlayerHUD>(GetHUD());
		AsuraPlayerHUD->CreateGameWidgets();
		AsuraPlayerHUD->GetCharacterSelectionUIWidget()->ShowWidget();

	}

	//PlayerHUD->GetCharacterSelectionUIWidget()->AddToViewport(1);
	//PlayerHUD->GetCharacterSelectionUIWidget()->SetIsEnabled(true);
}

void AAsuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
	if (bMoveToMouseCursor)
	{
		MoveToMouseCursor();
	}



	//if (bCharacterSelected == true)
	//{
	//	bCharacterSelected = false;

	//	UGameplayStatics::OpenLevel(GetWorld(), FName("/Game/Levels/TopDownExampleMap"));
	//	AsuraPlayerHUD->GetCharacterSelectionUIWidget()->HideWidget();

	//	if (AsuraPlayerState->GetCurrentSelectedCharacterClass() == ECharacterClass::CC_Marauder)
	//	{
	//	

	//		FTransform DefaultMarauderSpawn = FTransform(FQuat(0, 0, 0, 0), FVector(-900.0025, -10.0, 263.55352));
	//		AMarauderCharacter* PlayerCurrentCharPawn = GetWorld()->SpawnActor<AMarauderCharacter>(MarauderClassTemplate,
	//			DefaultMarauderSpawn);

	//		//AActor* PlayerChar =
	//		//UGameplayStatics::BeginSpawningActorFromClass(GetWorld(), MarauderClassTemplate, DefaultMarauderSpawn, true);
	//		//UGameplayStatics::FinishSpawningActor(PlayerChar, DefaultMarauderSpawn);
	//		UnPossess();
	//		Possess(Cast<APawn>(PlayerCurrentCharPawn));

	//		bCharacterSelected = false;
	//		
	//	}
	//	else
	//	{
	//		FTransform DefaultTestCharSpawn = FTransform(FQuat(0, 0, 0, 0), FVector(-880.0025, -140.0, 263.55703));
	//		
	//		AAsuraCharacter* PlayerCurrentCharPawn = GetWorld()->SpawnActor<AAsuraCharacter>(TestClassTemplate,
	//			DefaultTestCharSpawn);
	//		//
	//		//AActor* PlayerChar =
	//		//UGameplayStatics::BeginSpawningActorFromClass(GetWorld(), TestClassTemplate, DefaultTestCharSpawn, true);
	//		//UGameplayStatics::FinishSpawningActor(PlayerChar, DefaultTestCharSpawn);
	//		
	//		
	//		UnPossess();
	//		Possess(Cast<APawn>(PlayerCurrentCharPawn));

	//		bCharacterSelected = false;

	//	}
	//}

}

void AAsuraPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &AAsuraPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &AAsuraPlayerController::OnSetDestinationReleased);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AAsuraPlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AAsuraPlayerController::MoveToTouchLocation);

	InputComponent->BindAction("ResetVR", IE_Pressed, this, &AAsuraPlayerController::OnResetVR);
}

void AAsuraPlayerController::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AAsuraPlayerController::MoveToMouseCursor()
{
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		if (AAsuraCharacter* MyPawn = Cast<AAsuraCharacter>(GetPawn()))
		{
			if (MyPawn->GetCursorToWorld())
			{
				UNavigationSystem::SimpleMoveToLocation(this, MyPawn->GetCursorToWorld()->GetComponentLocation());
			}
		}
	}
	else
	{
		// Trace to see what is under the mouse cursor
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		if (Hit.bBlockingHit)
		{
			// We hit something, move there
			SetNewMoveDestination(Hit.ImpactPoint);
		}
	}
}

void AAsuraPlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
	}
}

void AAsuraPlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
		float const Distance = FVector::Dist(DestLocation, MyPawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if (NavSys && (Distance > 120.0f))
		{
			NavSys->SimpleMoveToLocation(this, DestLocation);
		}
	}
}

void AAsuraPlayerController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void AAsuraPlayerController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}
