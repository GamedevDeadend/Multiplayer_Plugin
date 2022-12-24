// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuSystem.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYER_API UMenuSystem : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
		void MenuSetup(int32 INumConnections = 4, FString IMatchType = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));

protected:
	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* Inlevel, UWorld* InWorld) override;


private:

	int32 NumConnections;
	FString MatchType;
	FString PathToLobby;

	UPROPERTY(meta = (BindWidget))
		class UButton *Join;

	UPROPERTY(meta = (BindWidget))
		UButton *Host;

	UFUNCTION()
		void HostButtonClicked();

	UFUNCTION()
		void JoinButtonClicked();

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);


	void Menuteardown();

	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

};
