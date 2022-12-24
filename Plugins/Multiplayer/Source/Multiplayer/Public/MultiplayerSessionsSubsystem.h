// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionsSubsystem.generated.h"

/**
 * Declaring Custom Dynamic multicast delegate
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionDelegate, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionDelegate,const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionDelegate, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionDelegate, bool, bWasSuccessful);

UCLASS()
class MULTIPLAYER_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UMultiplayerSessionsSubsystem();

	//To Be Called With Menu class
	void CreateSession(int32 NumPublicConnections = 4, FString MatchType = "FreeForAll");
	void FindSessions(int32 MaxSearchResults);
	void JoinSessions(const FOnlineSessionSearchResult& SearchResult);
	void DestroySessions();
	void StartSession();


	FMultiplayerOnCreateSessionDelegate MultiplayerOnCreateSessionDelegate;
	FMultiplayerOnFindSessionDelegate MultiplayerOnFindSessionDelegate;
	FMultiplayerOnJoinSessionDelegate MultiplayerOnJoinSessionDelegate;
	FMultiplayerOnStartSessionDelegate MultiplayerOnStartSessionDelegate;
	FMultiplayerOnDestroySessionDelegate MultiplayerOnDestroySessionDelegate;

private:

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	//Delegates
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;

	//Delegate Handles
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;


	bool bCreateSessionOnDestroy = false;
	int32 LastNumPublicConnections;
	FString LastMatchType;



protected:

	//CallBack Functions for delegates
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);


};
