// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
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
	void LoadMenu();

	UFUNCTION(Exec) // Exec을 사용하면 함수 이름으로 콘솔 명령어창에서 호출이 가능하다. PlayerController와 
	virtual void Host() override;

	UFUNCTION(Exec)
	void Join(const FString& Address);

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> MenuClass;
};
