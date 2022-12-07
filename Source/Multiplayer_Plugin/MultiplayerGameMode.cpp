// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerGameMode.h"
#include "GameFrameWork/PlayerState.h"
#include "GameFramework/GameState.h"

void AMultiplayerGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState)
	{
		int32 PlayerCount = GameState.Get()->PlayerArray.Num();
		APlayerState* NewPlayerState = NewPlayer->GetPlayerState<APlayerState>();

		if (NewPlayerState)
		{
			FString NewPlayerName = NewPlayerState->GetPlayerName();

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					1,
					60.0f,
					FColor::Cyan,
					FString::Printf(TEXT("%s Joined in.\nTotal Players :: %d"), *NewPlayerName, PlayerCount)
				);
			}
		}
	}
}

void AMultiplayerGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (GameState)
	{
		int32 PlayerCount = GameState.Get()->PlayerArray.Num();
		APlayerState* LeftPlayerState = Exiting->GetPlayerState<APlayerState>();

		if (LeftPlayerState)
		{
			FString LeftPlayerName = LeftPlayerState->GetPlayerName();

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					1,
					60.0f,
					FColor::Cyan,
					FString::Printf(TEXT("%s Has Left.\nTotal Players :: %d"), *LeftPlayerName, PlayerCount-1)
				);
			}
		}
	}
}