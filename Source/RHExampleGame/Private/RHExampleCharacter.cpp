// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RHExampleCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

#include "Player/Controllers/RHPlayerController.h"
#include "GameFramework/RHPlayerInput.h"


//////////////////////////////////////////////////////////////////////////
// ARHExampleCharacter

ARHExampleCharacter::ARHExampleCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
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

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ARHExampleCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////
// Input


void ARHExampleCharacter::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	//Buttons
	static FName JumpAction(TEXT("Jump"));

	InInputComponent->BindAction(JumpAction, IE_Pressed, this, &ACharacter::Jump);
	InInputComponent->BindAction(JumpAction, IE_Released, this, &ACharacter::StopJumping);

	//Axis
	static FName MoveForwardAxis(TEXT("MoveForward"));
	static FName MoveRightAxis(TEXT("MoveRight"));

	static FName TurnAxis(TEXT("Turn"));
	static FName TurnRateAxis(TEXT("TurnRate"));

	static FName LookUpAxis(TEXT("LookUp"));
	static FName LookUpRateAxis(TEXT("LookUpRate"));


	InInputComponent->BindAxis(MoveForwardAxis, this, &ARHExampleCharacter::MoveForward);
	InInputComponent->BindAxis(MoveRightAxis, this, &ARHExampleCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
    // "turn" handles devices that provide an absolute delta, such as a mouse.
    // "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
    InInputComponent->BindAxis(TurnAxis, this, &ARHExampleCharacter::AddControllerYawInput);
    InInputComponent->BindAxis(TurnRateAxis, this, &ARHExampleCharacter::GamepadLookRight);
    InInputComponent->BindAxis(LookUpAxis, this, &ARHExampleCharacter::AddControllerPitchInput);
    InInputComponent->BindAxis(LookUpRateAxis, this, &ARHExampleCharacter::GamepadLookUp);

}

void ARHExampleCharacter::MoveRight(float Value)
{
	if (Controller != nullptr)
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

void ARHExampleCharacter::MoveForward(float Value)
{
	if (Controller != nullptr)
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void ARHExampleCharacter::GamepadLookUp(float Value)
{
    if (Value != 0.f && Controller && Controller->IsLocalPlayerController())
    {
		// Account invert for player
		APlayerController* PC = Cast<APlayerController>(Controller);
		if(PC != nullptr)
		{
			URHPlayerInput* PlayerInput = Cast<URHPlayerInput>(PC->PlayerInput);
			if (PlayerInput != nullptr && PlayerInput->GetInvertY())
			{
				Value = -(Value);
			}

			PC->AddPitchInput(Value);
		}        
    }
}

void ARHExampleCharacter::GamepadLookRight(float Value)
{
	if (Value != 0.f && Controller && Controller->IsLocalPlayerController())
	{
		// Account invert for player
		if (ARHPlayerController* const PC = Cast<ARHPlayerController>(Controller))
		{
			URHPlayerInput* PlayerInput = Cast<URHPlayerInput>(PC->PlayerInput);
			if (PlayerInput != nullptr && PlayerInput->GetInvertX())
			{
				Value = -(Value);
			}

			PC->AddYawInput(Value);
		}
	}
}

void ARHExampleCharacter::AddControllerPitchInput(float Value)
{
	if (Value != 0.f && Controller && Controller->IsLocalPlayerController())
	{
		Super::AddControllerPitchInput(Value);
	}
}

void ARHExampleCharacter::AddControllerYawInput(float Value)
{
	if (Value != 0.f && Controller && Controller->IsLocalPlayerController())
	{
		Super::AddControllerYawInput(Value);
	}
}

