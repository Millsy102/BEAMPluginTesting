#include "BeamEyeTrackerExampleCharacter.h"
#include "BeamEyeTrackerComponent.h"
#include "Engine/Canvas.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamDebugCVars.h"
#include "BeamDebugDraw.h"
#include "BeamLogging.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/App.h"

// Constructor

ABeamEyeTrackerExampleCharacter::ABeamEyeTrackerExampleCharacter()
{
	
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	BeamEyeTrackerComponent = CreateDefaultSubobject<UBeamEyeTrackerComponent>(TEXT("BeamEyeTrackerComponent"));

	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

// Input Setup

void ABeamEyeTrackerExampleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABeamEyeTrackerExampleCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABeamEyeTrackerExampleCharacter::Look);

		// Beam Eye Tracker controls
		EnhancedInputComponent->BindAction(StartEyeTrackingAction, ETriggerEvent::Triggered, this, &ABeamEyeTrackerExampleCharacter::StartEyeTracking);
		EnhancedInputComponent->BindAction(StopEyeTrackingAction, ETriggerEvent::Triggered, this, &ABeamEyeTrackerExampleCharacter::StopEyeTracking);
		EnhancedInputComponent->BindAction(ResetCalibrationAction, ETriggerEvent::Triggered, this, &ABeamEyeTrackerExampleCharacter::ResetCalibration);
		EnhancedInputComponent->BindAction(ToggleDebugHUDAction, ETriggerEvent::Triggered, this, &ABeamEyeTrackerExampleCharacter::ToggleDebugHUD);
		EnhancedInputComponent->BindAction(ToggleRecordingAction, ETriggerEvent::Triggered, this, &ABeamEyeTrackerExampleCharacter::ToggleRecording);
		EnhancedInputComponent->BindAction(TogglePlaybackAction, ETriggerEvent::Triggered, this, &ABeamEyeTrackerExampleCharacter::TogglePlayback);
	}
}

// Gameplay Functions

void ABeamEyeTrackerExampleCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABeamEyeTrackerExampleCharacter::Look(const FInputActionValue& Value)
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

// Beam Eye Tracker Integration

void ABeamEyeTrackerExampleCharacter::StartEyeTracking()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		if (Subsystem->StartBeamTracking())
		{
			UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Eye tracking started successfully"));
			
			// Start the update timer for HUD updates
			GetWorld()->GetTimerManager().SetTimer(HUDUpdateTimerHandle, this, &ABeamEyeTrackerExampleCharacter::UpdateHUD, 0.1f, true);
		}
		else
		{
			UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Failed to start eye tracking"));
		}
	}
}

void ABeamEyeTrackerExampleCharacter::StopEyeTracking()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		Subsystem->StopBeamTracking();
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Eye tracking stopped"));
		
		// Stop the HUD update timer
		GetWorld()->GetTimerManager().ClearTimer(HUDUpdateTimerHandle);
	}
}

void ABeamEyeTrackerExampleCharacter::ResetCalibration()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		Subsystem->ResetCalibration();
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Calibration reset"));
	}
}

void ABeamEyeTrackerExampleCharacter::ToggleDebugHUD()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		// Toggle debug HUD using console variable
		static bool bDebugHUDEnabled = true;
		bDebugHUDEnabled = !bDebugHUDEnabled;
		
		// This would ideally use the CVar system, but for now we'll use a local toggle
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Debug HUD %s"), bDebugHUDEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
	}
}

void ABeamEyeTrackerExampleCharacter::ToggleRecording()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		if (Subsystem->IsRecording())
		{
			Subsystem->StopRecording();
			UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Recording stopped"));
		}
		else
		{
			FString RecordingPath = FPaths::ProjectSavedDir() / TEXT("BeamRecordings") / FString::Printf(TEXT("recording_%s.csv"), *FDateTime::Now().ToString());
			if (Subsystem->StartRecording(RecordingPath))
			{
				UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Recording started to %s"), *RecordingPath);
			}
			else
			{
				UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Failed to start recording"));
			}
		}
	}
}

void ABeamEyeTrackerExampleCharacter::TogglePlayback()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		if (Subsystem->IsPlayingBack())
		{
			Subsystem->StopPlayback();
			UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Playback stopped"));
		}
		else
		{
			// For demo purposes, try to play back the last recording
			FString RecordingPath = FPaths::ProjectSavedDir() / TEXT("BeamRecordings") / TEXT("recording_latest.csv");
			if (Subsystem->StartPlayback(RecordingPath))
			{
				UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Playback started from %s"), *RecordingPath);
			}
			else
			{
				UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Failed to start playback"));
			}
		}
	}
}

// HUD Update and Drawing

void ABeamEyeTrackerExampleCharacter::UpdateHUD()
{
	if (!FBeamDebugCVars::IsDebugHUDEnabled())
	{
		return;
	}

	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		
		bIsTracking = Subsystem->IsBeamTracking();
		CurrentHealth = Subsystem->GetBeamHealth();
		CurrentFPS = Subsystem->GetTrackingFPS();
		CurrentBufferUtilization = Subsystem->GetBufferUtilization();
		CurrentGazePoint = Subsystem->CurrentGaze();
		CurrentHeadPose = Subsystem->HeadPosition();
		bIsRecording = Subsystem->IsRecording();
		bIsPlayingBack = Subsystem->IsPlayingBack();

		// Apply head tracking if we have valid data
		if (bIsTracking && CurrentHeadPose.Confidence > 0.1f)
		{
			ApplyHeadTracking(CurrentHeadPose);
		}
	}
}

void ABeamEyeTrackerExampleCharacter::ApplyHeadTracking(const FHeadPose& HeadPose)
{
	if (!FollowCamera || !CameraBoom)
	{
		return;
	}

	// Apply head rotation to camera boom
	FRotator CurrentRotation = CameraBoom->GetRelativeRotation();
	FRotator NewRotation = CurrentRotation;
	
	// Apply head yaw (left/right)
	NewRotation.Yaw += HeadPose.Rotation.Yaw * HeadTrackingSensitivity;
	
	// Apply head pitch (up/down) with limits
	float NewPitch = CurrentRotation.Pitch + HeadPose.Rotation.Pitch * HeadTrackingSensitivity;
	NewPitch = FMath::Clamp(NewPitch, -80.0f, 80.0f);
	NewRotation.Pitch = NewPitch;
	
	// Apply head roll (tilt) with limits
	float NewRoll = CurrentRotation.Roll + HeadPose.Rotation.Roll * HeadTrackingSensitivity;
	NewRoll = FMath::Clamp(NewRoll, -45.0f, 45.0f);
	NewRotation.Roll = NewRoll;
	
	// Smoothly interpolate to new rotation
	FRotator InterpolatedRotation = FMath::RInterpTo(CurrentRotation, NewRotation, GetWorld()->GetDeltaSeconds(), HeadTrackingSmoothing);
	CameraBoom->SetRelativeRotation(InterpolatedRotation);
}

// Debug Drawing

void ABeamEyeTrackerExampleCharacter::DrawDebugHUD(UCanvas* Canvas)
{
	if (!Canvas || !FBeamDebugCVars::IsDebugHUDEnabled())
	{
		return;
	}

	// Draw tracking status
	FString StatusText = FString::Printf(TEXT("Beam Eye Tracking: %s"), bIsTracking ? TEXT("Active") : TEXT("Inactive"));
	Canvas->DrawText(DebugFont, StatusText, 10.0f, 10.0f, 1.2f, 1.2f);
	
	// Draw gaze point if valid
	if (CurrentGazePoint.bValid)
	{
		FString GazeText = FString::Printf(TEXT("Gaze: (%.1f, %.1f) Conf: %.2f"), 
			CurrentGazePoint.Screen01.X, CurrentGazePoint.Screen01.Y, CurrentGazePoint.Confidence);
		Canvas->DrawText(DebugFont, GazeText, 10.0f, 40.0f, 1.0f, 1.0f);
	}
	else
	{
		Canvas->DrawText(DebugFont, TEXT("Gaze: Invalid"), 10.0f, 40.0f, 1.0f, 1.0f);
	}
	
	// Draw head pose if valid
	if (CurrentHeadPose.Confidence > 0.0)
	{
		FString HeadText = FString::Printf(TEXT("Head: P:%.1f° Y:%.1f° R:%.1f° Conf: %.2f"), 
			CurrentHeadPose.Rotation.Pitch, CurrentHeadPose.Rotation.Yaw, CurrentHeadPose.Rotation.Roll, CurrentHeadPose.Confidence);
		Canvas->DrawText(DebugFont, HeadText, 10.0f, 70.0f, 1.0f, 1.0f);
	}
	else
	{
		Canvas->DrawText(DebugFont, TEXT("Head: Invalid"), 10.0f, 70.0f, 1.0f, 1.0f);
	}
	
	// Draw tracking quality based on confidence values
	float GazeQuality = CurrentGazePoint.bValid ? CurrentGazePoint.Confidence : 0.0f;
	float HeadQuality = CurrentHeadPose.Confidence;
	float OverallQuality = (GazeQuality + HeadQuality) * 0.5f;
	FString QualityText = FString::Printf(TEXT("Quality: %.1f%%"), OverallQuality * 100.0f);
	Canvas->DrawText(DebugFont, QualityText, 10.0f, 100.0f, 1.0f, 1.0f);
}

FBeamFrame ABeamEyeTrackerExampleCharacter::GetCurrentFrame() const
{
	FBeamFrame Frame;
	Frame.Gaze = CurrentGazePoint;
	Frame.Head = CurrentHeadPose;
	return Frame;
}

// Lifecycle

void ABeamEyeTrackerExampleCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	StartEyeTracking();
}

void ABeamEyeTrackerExampleCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Stop eye tracking
	StopEyeTracking();
	
	Super::EndPlay(EndPlayReason);
}

void ABeamEyeTrackerExampleCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsTracking)
	{
		UpdateHUD();
	}
}


