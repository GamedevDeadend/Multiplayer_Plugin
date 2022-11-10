// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
		void MenuSetup();

};