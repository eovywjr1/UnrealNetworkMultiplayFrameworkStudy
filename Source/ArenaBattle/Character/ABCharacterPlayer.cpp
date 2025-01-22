// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ABCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ABCharacterControlData.h"
#include "ABCharacterMovementComponent.h"
#include "ArenaBattle.h"
#include "EngineUtils.h"
#include "UI/ABHUDWidget.h"
#include "CharacterStat/ABCharacterStatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Interface/ABGameInterface.h"
#include "Net/UnrealNetwork.h"
#include "Physics/ABCollision.h"

AABCharacterPlayer::AABCharacterPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UABCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Input
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionJumpRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Jump.IA_Jump'"));
	if (nullptr != InputActionJumpRef.Object)
	{
		JumpAction = InputActionJumpRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputChangeActionControlRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ChangeControl.IA_ChangeControl'"));
	if (nullptr != InputChangeActionControlRef.Object)
	{
		ChangeControlAction = InputChangeActionControlRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ShoulderMove.IA_ShoulderMove'"));
	if (nullptr != InputActionShoulderMoveRef.Object)
	{
		ShoulderMoveAction = InputActionShoulderMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderLookRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ShoulderLook.IA_ShoulderLook'"));
	if (nullptr != InputActionShoulderLookRef.Object)
	{
		ShoulderLookAction = InputActionShoulderLookRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionQuaterMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_QuaterMove.IA_QuaterMove'"));
	if (nullptr != InputActionQuaterMoveRef.Object)
	{
		QuaterMoveAction = InputActionQuaterMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionAttackRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Attack.IA_Attack'"));
	if (nullptr != InputActionAttackRef.Object)
	{
		AttackAction = InputActionAttackRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> TeleportActionAttackRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Teleport.IA_Teleport'"));
	if (TeleportActionAttackRef.Object)
	{
		TeleportAction = TeleportActionAttackRef.Object;
	}

	CurrentCharacterControlType = ECharacterControlType::Quater;
}

void AABCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		EnableInput(PlayerController);
	}

	SetCharacterControl(CurrentCharacterControlType);
}

void AABCharacterPlayer::SetDead()
{
	Super::SetDead();
	
	GetWorldTimerManager().SetTimer(DeadTimerHandle, this, &AABCharacterPlayer::ResetPlayer, 5.0f, false);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		DisableInput(PlayerController);
	}
}

void AABCharacterPlayer::PossessedBy(AController* NewController)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::PossessedBy(NewController);

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::PostNetInit()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s %s"), TEXT("Begin"), *GetName());

	Super::PostNetInit();

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInputComponent->BindAction(ChangeControlAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ChangeCharacterControl);
	EnhancedInputComponent->BindAction(ShoulderMoveAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ShoulderMove);
	EnhancedInputComponent->BindAction(ShoulderLookAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ShoulderLook);
	EnhancedInputComponent->BindAction(QuaterMoveAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::QuaterMove);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::Attack);
	EnhancedInputComponent->BindAction(TeleportAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::Teleport);
}

void AABCharacterPlayer::ChangeCharacterControl()
{
	if (CurrentCharacterControlType == ECharacterControlType::Quater)
	{
		SetCharacterControl(ECharacterControlType::Shoulder);
	}
	else if (CurrentCharacterControlType == ECharacterControlType::Shoulder)
	{
		SetCharacterControl(ECharacterControlType::Quater);
	}
}

void AABCharacterPlayer::SetCharacterControl(ECharacterControlType NewCharacterControlType)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	UABCharacterControlData* NewCharacterControl = CharacterControlManager[NewCharacterControlType];
	check(NewCharacterControl);

	SetCharacterControlData(NewCharacterControl);

	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		UInputMappingContext* NewMappingContext = NewCharacterControl->InputMappingContext;
		if (NewMappingContext)
		{
			Subsystem->AddMappingContext(NewMappingContext, 0);
		}
	}

	CurrentCharacterControlType = NewCharacterControlType;
}

void AABCharacterPlayer::SetCharacterControlData(const UABCharacterControlData* CharacterControlData)
{
	Super::SetCharacterControlData(CharacterControlData);

	CameraBoom->TargetArmLength = CharacterControlData->TargetArmLength;
	CameraBoom->SetRelativeRotation(CharacterControlData->RelativeRotation);
	CameraBoom->bUsePawnControlRotation = CharacterControlData->bUsePawnControlRotation;
	CameraBoom->bInheritPitch = CharacterControlData->bInheritPitch;
	CameraBoom->bInheritYaw = CharacterControlData->bInheritYaw;
	CameraBoom->bInheritRoll = CharacterControlData->bInheritRoll;
	CameraBoom->bDoCollisionTest = CharacterControlData->bDoCollisionTest;
}

void AABCharacterPlayer::ShoulderMove(const FInputActionValue& Value)
{
	if (!bCanAttack)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void AABCharacterPlayer::ShoulderLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AABCharacterPlayer::QuaterMove(const FInputActionValue& Value)
{
	if (!bCanAttack)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	float InputSizeSquared = MovementVector.SquaredLength();
	float MovementVectorSize = 1.0f;
	float MovementVectorSizeSquared = MovementVector.SquaredLength();
	if (MovementVectorSizeSquared > 1.0f)
	{
		MovementVector.Normalize();
		MovementVectorSizeSquared = 1.0f;
	}
	else
	{
		MovementVectorSize = FMath::Sqrt(MovementVectorSizeSquared);
	}

	FVector MoveDirection = FVector(MovementVector.X, MovementVector.Y, 0.0f);
	GetController()->SetControlRotation(FRotationMatrix::MakeFromX(MoveDirection).Rotator());
	AddMovementInput(MoveDirection, MovementVectorSize);
}

void AABCharacterPlayer::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AABCharacterPlayer::Attack()
{
	//ProcessComboCommand();

	if (bCanAttack)
	{
		if (!HasAuthority())
		{
			bCanAttack = false;
			GetCharacterMovement()->SetMovementMode(MOVE_None);

			GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ThisClass::ResetAttack, AttackTime, false);

			GetMesh()->GetAnimInstance()->Montage_Play(ComboActionMontage);
		}

		ServerRPCAttack(GetWorld()->GetGameState()->GetServerWorldTimeSeconds());
	}
}

void AABCharacterPlayer::AttackHitCheck()
{
	if (IsLocallyControlled())
	{
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

		FHitResult OutHitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

		const float AttackRange = Stat->GetTotalStat().AttackRange;
		const float AttackRadius = Stat->GetAttackRadius();
		const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector End = Start + GetActorForwardVector() * AttackRange;

		bool HitDetected = GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, CCHANNEL_ABACTION, FCollisionShape::MakeSphere(AttackRadius), Params);
		const float HitCheckTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

		if (HasAuthority())
		{
			if (HitDetected)
			{
				FDamageEvent DamageEvent;
				const float AttackDamage = Stat->GetTotalStat().Attack;
				OutHitResult.GetActor()->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
			}
		}
		else
		{
			if (HitDetected)
			{
				ServerRPCNotifyHit(OutHitResult, HitCheckTime);
			}
			else
			{
				ServerRPCNotifyMiss(Start, End, GetActorForwardVector(), HitCheckTime);
			}
		}

#if ENABLE_DRAW_DEBUG
		FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
		float CapsuleHalfHeight = AttackRange * 0.5f;
		FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;

		DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 5.0f);
#endif
	}
}

bool AABCharacterPlayer::ServerRPCNotifyHit_Validate(const FHitResult& HitResult, const float HitCheckTime)
{
	return true;
}

void AABCharacterPlayer::ServerRPCNotifyHit_Implementation(const FHitResult& HitResult, const float HitCheckTime)
{
	AActor* HitActor = HitResult.GetActor();
	if (IsValid(HitActor))
	{
		const FVector HitLocation = HitResult.Location;
		const FBox HitBox = HitActor->GetComponentsBoundingBox();
		const FVector ActorBoxCenter = (HitBox.Min + HitBox.Max) * 0.5f;

		const float AcceptCheckDistance = 300.0f;
		if (FVector::DistSquared(HitLocation, ActorBoxCenter) <= AcceptCheckDistance * AcceptCheckDistance)
		{
			FDamageEvent DamageEvent;
			const float AttackDamage = Stat->GetTotalStat().Attack;
			HitActor->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
		}
	}
}

void AABCharacterPlayer::ServerRPCNotifyMiss_Implementation(const FVector TraceStart, const FVector TraceEnd, const FVector TraceDir, const float HitCheckTime) {}

bool AABCharacterPlayer::ServerRPCNotifyMiss_Validate(const FVector TraceStart, const FVector TraceEnd, const FVector TraceDir, const float HitCheckTime)
{
	return true;
}

bool AABCharacterPlayer::ServerRPCAttack_Validate(float AttackStartTime)
{
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	// 조건이 타이트해서 0.4만큼 오차를 추가
	return (AttackStartTime - LastAttackStartTime) > AttackTime - 0.4f;
}

void AABCharacterPlayer::ServerRPCAttack_Implementation(float AttackStartTime)
{
	bCanAttack = false;
	GetCharacterMovement()->SetMovementMode(MOVE_None);

	const float AttackTimeDifference = FMath::Clamp(GetWorld()->GetTimeSeconds() - AttackStartTime, 0.0f, AttackStartTime);

	GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ThisClass::ResetAttack, AttackTime - AttackTimeDifference, false);

	LastAttackStartTime = AttackStartTime;

	GetMesh()->GetAnimInstance()->Montage_Play(ComboActionMontage);

	//MulticastRPCAttack();

	for (APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		if (PlayerController && PlayerController != GetController() && !PlayerController->IsLocalController())
		{
			if (AABCharacterPlayer* PlayerCharacter = Cast<AABCharacterPlayer>(PlayerController->GetPawn()))
			{
				PlayerCharacter->ClientRPCPlayAttackAnimation(this);
			}
		}
	}
}

void AABCharacterPlayer::ClientRPCPlayAttackAnimation_Implementation(AABCharacterPlayer* PlayCharacter)
{
	if (PlayCharacter)
	{
		PlayCharacter->GetMesh()->GetAnimInstance()->Montage_Play(ComboActionMontage);
	}
}

void AABCharacterPlayer::SetupHUDWidget(UABHUDWidget* InHUDWidget)
{
	if (InHUDWidget)
	{
		InHUDWidget->UpdateStat(Stat->GetBaseStat(), Stat->GetModifierStat());
		InHUDWidget->UpdateHpBar(Stat->GetCurrentHp(), Stat->GetMaxHp());

		Stat->OnStatChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateStat);
		Stat->OnHpChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateHpBar);
	}
}

void AABCharacterPlayer::Teleport()
{
	Cast<UABCharacterMovementComponent>(GetCharacterMovement())->SetTeleportCommand();
}

void AABCharacterPlayer::ResetPlayer()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
	}

	Stat->SetLevelStat(1);
	Stat->ResetStat();

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	SetActorEnableCollision(true);

	HpBar->SetHiddenInGame(false);

	if (HasAuthority())
	{
		if (IABGameInterface* ABGameMode = GetWorld()->GetAuthGameMode<IABGameInterface>())
		{
			const FTransform NewTransform = ABGameMode->GetRandomStartTransform();
			TeleportTo(NewTransform.GetLocation(), NewTransform.GetRotation().Rotator());
		}
	}
}

void AABCharacterPlayer::ResetAttack()
{
	bCanAttack = true;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

// 서버에서 현재 누구한테 데미지를 받았는지 표시하도로 확장
float AABCharacterPlayer::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (Stat->GetCurrentHp() <= 0.0f)
	{
		if (IABGameInterface* ABGameMode = GetWorld()->GetAuthGameMode<IABGameInterface>())
		{
			ABGameMode->OnPlayerKilled(EventInstigator, GetController(), this);
		}
	}

	return ActualDamage;
}
