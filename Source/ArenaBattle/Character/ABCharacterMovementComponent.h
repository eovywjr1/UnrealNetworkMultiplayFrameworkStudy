// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ABCharacterMovementComponent.generated.h"

class FABSavedMove_Character : public FSavedMove_Character
{
public:
	virtual void Clear() override final;
	virtual void SetInitialPosition(ACharacter* C) override final;
	virtual uint8 GetCompressedFlags() const override final;
	
private:
	uint8 bPressedTeleport : 1 = false; 
	uint8 bDidTeleport : 1 = false;
	
private:
	using Super = FSavedMove_Character;
};

class FABNetworkPredictionData_Client_Character : public FNetworkPredictionData_Client_Character
{
public:
	FABNetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement);
	virtual FSavedMovePtr AllocateNewMove() override final;
};

/**
 * 
 */
UCLASS()
class ARENABATTLE_API UABCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	void SetTeleportCommand();
	
	FORCEINLINE uint8 IsPressedTeleport() const { return bPressedTeleport; }
	FORCEINLINE uint8 IsDidTeleport() const { return bDidTeleport; }
	
private:
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override final;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override final;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override final;

	void Teleport();
	
private:
	uint8 bPressedTeleport : 1 = false; 
	uint8 bDidTeleport : 1 = false;
};
