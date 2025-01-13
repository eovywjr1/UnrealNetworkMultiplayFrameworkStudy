// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/ABFountain.h"

#include "ArenaBattle.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AABFountain::AABFountain()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Water = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Water"));

	RootComponent = Body;
	Water->SetupAttachment(Body);
	Water->SetRelativeLocation(FVector(0.0f, 0.0f, 132.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01'"));
	if (BodyMeshRef.Object)
	{
		Body->SetStaticMesh(BodyMeshRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WaterMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Fountain_02.SM_Plains_Fountain_02'"));
	if (WaterMeshRef.Object)
	{
		Water->SetStaticMesh(WaterMeshRef.Object);
	}

	bReplicates = true;
	NetUpdateFrequency = 1.0f;
	// NetDormancy = DORM_Initial;
	NetCullDistanceSquared = 4000000.0f;
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		// 휴면 상태 해지
		// FlushNetDormancy();
		//
		// FTimerHandle TimerHandle;
		// GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
		// 	                                       {
		// 		                                       // ServerLightColor = FLinearColor(FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), 1.0f);
		// 		                                       // OnRep_ServerLightColor();
		//
		// 		                                       const FLinearColor NewColor = FLinearColor(FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), 1.0f);
		// 		                                       // MulticastRPCChangeLightColor(NewColor);
		// 		                                       ClientRPCChangeLightColor(NewColor);
		// 	                                       }
		// 	                                       ), 1.0f, true, -1.0f);
		//
		// FTimerHandle Client1BeginPlayTimerHandle;
		// GetWorld()->GetTimerManager().SetTimer(Client1BeginPlayTimerHandle, FTimerDelegate::CreateLambda([this]()
		// 	                                       {
		// 		                                       for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		// 		                                       {
		// 			                                       APlayerController* PlayerController = Iterator->Get();
		// 			                                       if (PlayerController && !PlayerController->IsLocalController())
		// 			                                       {
		// 				                                       SetOwner(PlayerController);
		// 				                                       break;
		// 			                                       }
		// 		                                       }
		// 	                                       }
		// 	                                       ), 10.0f, false, -1.0f);
	}
	else
	{
		// FTimerHandle TimerHandle;
		// GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
		// 										   {
		// 											   ServerRPCChangeLightColor();
		// 										   }
		// 										   ), 1.0f, true, 0.0f);
	}
}

// Called every frame
void AABFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	constexpr float RotationRate = 30.0f;

	if (HasAuthority())
	{
		AddActorLocalRotation(FRotator(0.0f, RotationRate * DeltaTime, 0.0f));
		ServerRotationYaw = RootComponent->GetComponentRotation().Yaw;
	}
	else
	{
		ClientTimeSinceUpdate += DeltaTime;

		if (ClientTimeBetweenLastUpdate > KINDA_SMALL_NUMBER)
		{
			const float EstimateRotationYaw = ServerRotationYaw + RotationRate * ClientTimeBetweenLastUpdate;
			const float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;

			FRotator ClientRotator = RootComponent->GetComponentRotation();
			ClientRotator.Yaw = FMath::Lerp(ServerRotationYaw, EstimateRotationYaw, LerpRatio);
			RootComponent->SetWorldRotation(ClientRotator);
		}
	}
}

void AABFountain::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	Super::PreReplication(ChangedPropertyTracker);
}

void AABFountain::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AABFountain, ServerRotationYaw);
	DOREPLIFETIME_CONDITION(AABFountain, ServerLightColor, COND_InitialOnly);
}

void AABFountain::OnRep_ServerRotationYaw()
{
	FRotator NewRotator = RootComponent->GetComponentRotation();
	NewRotator.Yaw = ServerRotationYaw;
	RootComponent->SetWorldRotation(NewRotator);

	ClientTimeBetweenLastUpdate = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0.0f;
}

void AABFountain::OnRep_ServerLightColor()
{
	if (UPointLightComponent* PointLightComponent = FindComponentByClass<UPointLightComponent>())
	{
		PointLightComponent->SetLightColor(ServerLightColor);
	}
}

void AABFountain::ClientRPCChangeLightColor_Implementation(const FLinearColor& NewColor)
{
	AB_LOG(LogABNetwork, Log, TEXT("Light Color"));
	if (UPointLightComponent* PointLightComponent = FindComponentByClass<UPointLightComponent>())
	{
		PointLightComponent->SetLightColor(NewColor);
	}
}

bool AABFountain::ServerRPCChangeLightColor_Validate()
{
	return true;
}

void AABFountain::ServerRPCChangeLightColor_Implementation()
{
	const FLinearColor NewColor = FLinearColor(FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), 1.0f);
	MulticastRPCChangeLightColor_Implementation(NewColor);
}

void AABFountain::MulticastRPCChangeLightColor_Implementation(const FLinearColor& NewColor)
{
	AB_LOG(LogABNetwork, Log, TEXT("Light Color"));
	if (UPointLightComponent* PointLightComponent = FindComponentByClass<UPointLightComponent>())
	{
		PointLightComponent->SetLightColor(NewColor);
	}
}
