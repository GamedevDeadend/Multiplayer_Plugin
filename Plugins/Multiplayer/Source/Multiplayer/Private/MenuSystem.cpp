    // Fill out your copyright notice in the Description page of Project Settings.


#include "MenuSystem.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"


bool UMenuSystem::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (Host)
    {
        //Binding Function To Button
        Host->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
    }

    if (Join)
    {
        //Binding Function To Button
        Join->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
    }

    return true;
}

void UMenuSystem::OnLevelRemovedFromWorld(ULevel* Inlevel, UWorld* InWorld)
{
    Menuteardown();
    Super::OnLevelRemovedFromWorld(Inlevel, InWorld);
}

void UMenuSystem :: MenuSetup(int32 INumConnections, FString IMatchType, FString LobbyPath)
{
    PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
    NumConnections = INumConnections;
    MatchType = IMatchType;
    //Menu Widget Setup
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    UWorld *World = GetWorld();
    if (World)
    {
        APlayerController* PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeUIOnly InputModeData;
            InputModeData.SetWidgetToFocus(TakeWidget());
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);

        }
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
      MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    }
    
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionDelegate.AddDynamic(this, &ThisClass::OnCreateSession);
        MultiplayerSessionsSubsystem->MultiplayerOnFindSessionDelegate.AddUObject(this, &ThisClass::OnFindSession);
        MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionDelegate.AddUObject(this, &ThisClass::OnJoinSession);
        MultiplayerSessionsSubsystem->MultiplayerOnStartSessionDelegate.AddDynamic(this, &ThisClass::OnStartSession);
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionDelegate.AddDynamic(this, &ThisClass::OnDestroySession);
    }
}

void UMenuSystem::HostButtonClicked()
{
    Host->SetIsEnabled(false);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("HelloHostButtonClicked"));
    }

    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->CreateSession(NumConnections,MatchType);
    }
}

void UMenuSystem::JoinButtonClicked()
{
    Join->SetIsEnabled(false);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("JoinButtonClicked"));
    }
    //UE_LOG(LogTemp, Warning, TEXT("JOIN BUTTON CLICKED"));
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->FindSessions(10000);
    }
}

void UMenuSystem::Menuteardown()
{
    RemoveFromParent();
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController *PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeGameOnly InputMode;
            PlayerController->SetInputMode(InputMode);
            PlayerController->SetShowMouseCursor(false);
        }
    }
}



void UMenuSystem::OnCreateSession(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Session Was Created Menu Callback"));
        }

        UWorld* World = GetWorld();
        if (World)
        {
            World->ServerTravel(FString(PathToLobby)); //?listen Opens Level as listen server
        }
    }

    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Session Creation Failed"));
        }

        Host->SetIsEnabled(true);
    }
}

void UMenuSystem::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("JoinButtonClicked7"));
    }
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            FString IPAddress;
            SessionInterface->GetResolvedConnectString(NAME_GameSession, IPAddress);

            APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();

            if (PlayerController)
            {
                PlayerController->ClientTravel(IPAddress, ETravelType::TRAVEL_Absolute);
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage
                    (
                        -1,
                        15.0f,
                        FColor::Green,
                        FString::Printf(TEXT("Connected to IP : %s"), *IPAddress)
                    );
                }

            }
        }
    }

    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        Join->SetIsEnabled(true);
    }
}

void UMenuSystem::OnStartSession(bool bWasSuccessful)
{
    if (bWasSuccessful == true)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Yellow, FString("Session Has Started and Progression is True"));
        }
    }
}

void UMenuSystem::OnDestroySession(bool bWasSuccessful)
{
}

void UMenuSystem::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful)
{

    if (GEngine)
    {
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("JoinButtonClicked4 %d"), SessionResult.Num()));
    }

    for (auto Result : SessionResult)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("JoinButtonClicked4.1"));
        }

        FString Id = Result.GetSessionIdStr();
        FString User = Result.Session.OwningUserName;
        FString MatchMode;
        Result.Session.SessionSettings.Get(FName("MatchType"), MatchMode);

        if (MatchMode == FString("FreeForAll"))
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage
                (
                    -1,
                    15.0f,
                    FColor::Green,
                    FString::Printf(TEXT("User:%s, Id:%s"), *Id, *User)
                );
            }

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage
                (
                    -1,
                    15.0f,
                    FColor::Green,
                    FString::Printf(TEXT("Joined %s Match"), *MatchType)
                );
            }

            MultiplayerSessionsSubsystem->JoinSessions(Result);
        }
    }

    if (!bWasSuccessful || SessionResult.Num() == 0)
    {
        Join->SetIsEnabled(true);
    }
}