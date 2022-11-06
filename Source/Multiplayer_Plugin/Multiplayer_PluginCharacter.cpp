// Copyright Epic Games, Inc. All Rights Reserved.

#include "Multiplayer_PluginCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

//////////////////////////////////////////////////////////////////////////
// AMultiplayer_PluginCharacter

AMultiplayer_PluginCharacter::AMultiplayer_PluginCharacter() :
	//Member Intializer List

	//CreateSession Delegate
	//###ThisClass_is_typedef_for_current_class###
	CreateSessionCompleteDelegate
	(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)	//Member Intializer list to create delegate object and bind to function
	),
	//Find Session Delegate
	FindSessionsCompleteDelegate
	(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)
	)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

	if (OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage
			(	-1,
				20.0f,
				FColor::Green,
				FString::Printf(TEXT("OnlineSubsystem name is %s "), *OnlineSubsystem->GetSubsystemName().ToString())
			);
		}

	}


	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMultiplayer_PluginCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AMultiplayer_PluginCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AMultiplayer_PluginCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AMultiplayer_PluginCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AMultiplayer_PluginCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMultiplayer_PluginCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMultiplayer_PluginCharacter::TouchStopped);
}

void AMultiplayer_PluginCharacter::CreateGameSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	//Checking For Exisiting Ongoing Session to avoid creating multiple sessions
	auto ExisitingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (ExisitingSession != nullptr)
	{
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}

	//Delegate shoudn't add it self again and again
	if (bCanAddDelegate == true)
	{
		OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
		bCanAddDelegate = false;
	}
	//Adding Delegate to delegate list of Online session Interface

	//Creating Share Pointer of Session setting for Paramater in  Create session method
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	
	//Setting for Session
	SessionSettings->bIsLANMatch = false;
	SessionSettings->NumPublicConnections = 4;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->bUsesPresence = true;

	//Creating Pointer LocalPlayer to access Local Player id to pass as Paramater in  Create session method
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	
	//Creating Session
	OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
}


void AMultiplayer_PluginCharacter::JoinGameSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	//Delegate shoudn't add it self again and again
	if (bCanAddDelegate == true)
	{
		OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
		bCanAddDelegate = false;
	}
	//Adding Delegate to delegate list of Online session Interface

	TSharedPtr<FOnlineSessionSearch> SessionSearch = MakeShareable(new FOnlineSessionSearch());

}

void AMultiplayer_PluginCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful == true)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1,
				15.0f,
				FColor::Green,
				FString::Printf(TEXT("%s Session Was Created"), *SessionName.ToString())
			);
		}
	}

	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Session Creation Failed"))
			);
		}
	}

}

void AMultiplayer_PluginCharacter::OnFindSessionComplete(bool bWasSuccessful)
{
}

void AMultiplayer_PluginCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AMultiplayer_PluginCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AMultiplayer_PluginCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMultiplayer_PluginCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMultiplayer_PluginCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMultiplayer_PluginCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
