// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStat/ABCharacterStatComponent.h"
#include "GameData/ABGameSingleton.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UABCharacterStatComponent::UABCharacterStatComponent()
{
	CurrentLevel = 1;
	AttackRadius = 50.0f;

	bWantsInitializeComponent = true;

	SetIsReplicatedByDefault(true);
}

void UABCharacterStatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SetLevelStat(CurrentLevel);
	MaxHp = BaseStat.MaxHp;
	SetHp(MaxHp);
	
	OnStatChanged.AddUObject(this, &ThisClass::SetMaxHp);
}

void UABCharacterStatComponent::SetLevelStat(int32 InNewLevel)
{
	CurrentLevel = FMath::Clamp(InNewLevel, 1, UABGameSingleton::Get().CharacterMaxLevel);
	SetBaseStat(UABGameSingleton::Get().GetCharacterStat(CurrentLevel));
	check(BaseStat.MaxHp > 0.0f);
}

float UABCharacterStatComponent::ApplyDamage(float InDamage)
{
	const float PrevHp = CurrentHp;
	const float ActualDamage = FMath::Clamp<float>(InDamage, 0, InDamage);

	SetHp(PrevHp - ActualDamage);

	return ActualDamage;
}

void UABCharacterStatComponent::OnRep_CurrentHp()
{
	OnHpChanged.Broadcast(CurrentHp, MaxHp);
	if (CurrentHp <= KINDA_SMALL_NUMBER)
	{
		OnHpZero.Broadcast();
	}
}

void UABCharacterStatComponent::OnRep_MaxHp()
{
	OnHpChanged.Broadcast(CurrentHp, MaxHp);
}

void UABCharacterStatComponent::OnRep_BaseStat()
{
	OnStatChanged.Broadcast(BaseStat, ModifierStat);
}

void UABCharacterStatComponent::OnRep_ModifierStat()
{
	OnStatChanged.Broadcast(BaseStat, ModifierStat);
}

void UABCharacterStatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentHp);
	DOREPLIFETIME(ThisClass, MaxHp);
	DOREPLIFETIME_CONDITION(ThisClass, BaseStat, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, ModifierStat, COND_OwnerOnly);
}

void UABCharacterStatComponent::SetHp(float NewHp)
{
	CurrentHp = FMath::Clamp<float>(NewHp, 0.0f, MaxHp);

	if (GetOwner()->HasAuthority())
	{
		OnRep_CurrentHp();
	}
}

void UABCharacterStatComponent::SetMaxHp(const FABCharacterStat& InBaseStat, const FABCharacterStat& InModifierStat)
{
	float PrevMaxHp = MaxHp;
	MaxHp = GetTotalStat().MaxHp;
	if (PrevMaxHp != MaxHp)
	{
		OnHpChanged.Broadcast(CurrentHp, MaxHp);
	}
}
