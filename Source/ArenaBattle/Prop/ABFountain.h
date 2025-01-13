// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ABFountain.generated.h"

UCLASS()
class ARENABATTLE_API AABFountain : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AABFountain();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<class UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<class UStaticMeshComponent> Water;

private:
	// virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override final;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override final;

	UFUNCTION()
	void OnRep_ServerRotationYaw();

	UFUNCTION()
	void OnRep_ServerLightColor();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCChangeLightColor(const FLinearColor& NewColor);

	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerRPCChangeLightColor();
	
	UFUNCTION(Client, Unreliable)
	void ClientRPCChangeLightColor(const FLinearColor& NewColor);

private:
	UPROPERTY(ReplicatedUsing=OnRep_ServerRotationYaw)
	float ServerRotationYaw;
	
	float ClientTimeSinceUpdate = 0.0f;
	float ClientTimeBetweenLastUpdate = 0.0f;
	
	UPROPERTY(ReplicatedUsing=OnRep_ServerLightColor)
	FLinearColor ServerLightColor;
};
