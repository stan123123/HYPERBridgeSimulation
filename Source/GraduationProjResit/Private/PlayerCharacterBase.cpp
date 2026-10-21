#include "PlayerCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

APlayerCharacterBase::APlayerCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("APlayerCharacterBase Spawned successfully"));

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("APlayerCharacterBase::BeginPlay - DefaultMappingContext is NULL!"));
			}
		}
	}
}

void APlayerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacterBase::Move);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("APlayerCharacterBase::SetupPlayerInputComponent - MoveAction is NULL!"));
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacterBase::Look);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("APlayerCharacterBase::SetupPlayerInputComponent - LookAction is NULL!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerCharacterBase::SetupPlayerInputComponent - Oh shit, EnhancedInputComponent is null!"));
	}
}

void APlayerCharacterBase::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void APlayerCharacterBase::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}