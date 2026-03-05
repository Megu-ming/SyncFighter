// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SFPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class SYNCFIGHTERCLIENT_API ASFPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ASFPlayerState();

	// ==========================================
	// 핵심 스탯 변수들
	// ==========================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 Kills;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 Deaths;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float TotalDamageDealt;

	// 서버 통신용 플레이어 고유 ID
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	int32 SFPlayerID;
	
	// ==========================================
	// 스탯 갱신 함수들 (서버 패킷을 받을 때 호출)
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddKill();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddDeath();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddDamage(float DamageAmount);

	// 게임 시작 시 초기화
	void ResetStats();
};
