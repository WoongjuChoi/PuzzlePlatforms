// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuSystem/MenuInterface.h"
#include "PuzzlePlatformsGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLEPLATFORMS_API UPuzzlePlatformsGameInstance : public UGameInstance, public IMenuInterface
{
	GENERATED_BODY()

public:
	UPuzzlePlatformsGameInstance(const FObjectInitializer& ObjectInitializer);

	virtual void Init();

	UFUNCTION(BlueprintCallable)
	void LoadMenuWidget();

	UFUNCTION(BlueprintCallable)
	void InGameLoadMenu();

	UFUNCTION(Exec) // Exec을 사용하면 함수 이름으로 콘솔 명령어창에서 호출이 가능하다. PlayerController와 
	virtual void Host(FString ServerName) override;

	UFUNCTION(Exec)
	virtual void Join(uint32 Index) override;

	virtual void LoadMainMenu() override;

	virtual void RefreshServerList() override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> MenuClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> InGameMenuClass;
	
	UPROPERTY()
	class UMainMenu* Menu;
	
	UPROPERTY()
	class UInGameMenu* InGameMenu;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FString DesiredServerName;
	
	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionComplete(bool Success);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void CreateSession();
};
