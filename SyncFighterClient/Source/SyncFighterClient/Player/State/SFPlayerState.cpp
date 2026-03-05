#include "SFPlayerState.h"

ASFPlayerState::ASFPlayerState()
{
	Kills = 0;
	Deaths = 0;
	TotalDamageDealt = 0.0f;
	SFPlayerID = -1;
}

void ASFPlayerState::AddKill()
{
	Kills++;
	UE_LOG(LogTemp, Warning, TEXT("[PlayerState] 킬 추가! 현재 킬: %d"), Kills);
}

void ASFPlayerState::AddDeath()
{
	Deaths++;
	UE_LOG(LogTemp, Warning, TEXT("[PlayerState] 데스 추가! 현재 데스: %d"), Deaths);
}

void ASFPlayerState::AddDamage(float DamageAmount)
{
	TotalDamageDealt += DamageAmount;
	// 누적 데미지는 나중에 결과창에서 MVP를 선정하거나 기여도를 보여줄 때 아주 유용합니다.
}

void ASFPlayerState::ResetStats()
{
	Kills = 0;
	Deaths = 0;
	TotalDamageDealt = 0.0f;
}