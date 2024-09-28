// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"

#include "Components/Button.h"

void UMainMenu::SetMenuInterface(IMenuInterface* Interface)
{
	MenuInterface = Interface;
}

void UMainMenu::Setup()
{
	AddToViewport();

	UWorld* World = GetWorld();
	if (World == nullptr) return;
	
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController == nullptr) return;
	
	FInputModeUIOnly InputMode;
	PlayerController->SetInputMode(InputMode);

	PlayerController->bShowMouseCursor = true;
}

bool UMainMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (Success == false) return false;

	if (Host == nullptr) return false;
	Host->OnClicked.AddDynamic(this, &UMainMenu::HostServer);
	
	return true;
}

void UMainMenu::NativeDestruct()
{
	Super::NativeDestruct();
	
	UE_LOG(LogTemp, Warning, TEXT("NativeDestruct"));
	
	RemoveFromParent();

	UWorld* World = GetWorld();
	if (World == nullptr) return;
	
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController == nullptr) return;
	
	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);

	PlayerController->bShowMouseCursor = false;
}

void UMainMenu::HostServer()
{
	if (MenuInterface == nullptr) return;

	MenuInterface->Host();
}
