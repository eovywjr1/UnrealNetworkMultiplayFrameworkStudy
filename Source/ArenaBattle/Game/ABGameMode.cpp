// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ABGameMode.h"
#include "ABGameMode.h"

#include "ABGameState.h"
#include "ArenaBattle.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Player/ABPlayerController.h"

AABGameMode::AABGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> DefaultPawnClassRef(TEXT("/Script/Engine.Blueprint'/Game/ArenaBattle/Blueprint/BP_ABCharacterPlayer.BP_ABCharacterPlayer_C'"));
	if (DefaultPawnClassRef.Class)
	{
		DefaultPawnClass = DefaultPawnClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(TEXT("/Script/ArenaBattle.ABPlayerController"));
	if (PlayerControllerClassRef.Class)
	{
		PlayerControllerClass = PlayerControllerClassRef.Class;
	}

	GameStateClass = AABGameState::StaticClass();
}

void AABGameMode::OnPlayerKilled(AController* Killer, AController* KilledPlayer, APawn* KilledPawn) const
{}

FTransform AABGameMode::GetRandomStartTransform()
{
	if(PlayerStarts.IsEmpty())
	{
		return FTransform(FVector(0.0f, 0.0f, 230.0f));
	}
	
	const int32 RandomIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
	return PlayerStarts[RandomIndex]->GetActorTransform();
}

void AABGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("==============================================="));
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	// ErrorMessage = TEXT("Server Is Full");

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

APlayerController* AABGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	APlayerController* NewPlayerController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));

	return NewPlayerController;
}

void AABGameMode::PostLogin(APlayerController* NewPlayer)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	Super::PostLogin(NewPlayer);
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABGameMode::StartPlay()
{
	Super::StartPlay();

	for (APlayerStart* PlayerStart : TActorRange<APlayerStart>(GetWorld()))
	{
		PlayerStarts.Add(PlayerStart);
	}
}
