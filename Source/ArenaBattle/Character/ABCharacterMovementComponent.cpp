// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ABCharacterMovementComponent.h"

#include "GameFramework/Character.h"

constexpr float TeleportOffset = 600.0f;
constexpr float TeleportCoolTime = 3.0f;

FSavedMovePtr FABNetworkPredictionData_Client_Character::AllocateNewMove()
{
	return FSavedMovePtr(new FABSavedMove_Character());
}

void FABSavedMove_Character::Clear()
{
	Super::Clear();

	bPressedTeleport = false;
	bDidTeleport = false;
}

void FABSavedMove_Character::SetInitialPosition(ACharacter* C)
{
	Super::SetInitialPosition(C);

	if (UABCharacterMovementComponent* ABMoveComponent = Cast<UABCharacterMovementComponent>(C->GetCharacterMovement()))
	{
		bPressedTeleport = ABMoveComponent->IsPressedTeleport();
		bDidTeleport = ABMoveComponent->IsDidTeleport();
	}
}

uint8 FABSavedMove_Character::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bPressedTeleport)
	{
		Result |= FLAG_Custom_0;
	}

	if (bDidTeleport)
	{
		Result |= FLAG_Custom_1;
	}

	return Result;
}

FABNetworkPredictionData_Client_Character::FABNetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement)
	: FNetworkPredictionData_Client_Character(ClientMovement)
{}

void UABCharacterMovementComponent::SetTeleportCommand()
{
	bPressedTeleport = true;
}

void UABCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	if (bPressedTeleport && !bDidTeleport)
	{
		Teleport();
	}

	if (bPressedTeleport)
	{
		bPressedTeleport = false;
	}
}

FNetworkPredictionData_Client* UABCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UABCharacterMovementComponent* MutableThis = const_cast<UABCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FABNetworkPredictionData_Client_Character(*this);
	}

	return ClientPredictionData;
}

void UABCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bPressedTeleport = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bDidTeleport = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;

	if (CharacterOwner->HasAuthority() && bPressedTeleport && !bDidTeleport)
	{
		Teleport();
	}
}

void UABCharacterMovementComponent::Teleport()
{
	FVector TargetLocation = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * TeleportOffset;
	CharacterOwner->TeleportTo(TargetLocation, CharacterOwner->GetActorRotation(), false, true);

	bDidTeleport = true;

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
	{
		bDidTeleport = false;
	}), TeleportCoolTime, false, -1.0f);
}
