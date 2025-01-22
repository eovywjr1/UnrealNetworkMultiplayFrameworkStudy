// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interface/ABGameInterface.h"
#include "ABGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABGameMode : public AGameModeBase, public IABGameInterface
{
	GENERATED_BODY()

public:
	AABGameMode();

	virtual void OnPlayerKilled(AController* Killer, AController* KilledPlayer, APawn* KilledPawn) const override final;
	virtual FTransform GetRandomStartTransform() override final;
	
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override final;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override final;
	virtual void PostLogin(APlayerController* NewPlayer) override final;
	virtual void StartPlay() override final;
	
private:
	TArray<TObjectPtr<class APlayerStart>> PlayerStarts;
};
