// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"

#include "Components/Button.h"

void UMainMenu::SetMenuInterface(IMenuInterface* Interface)
{
	MenuInterface = Interface;
}

bool UMainMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (Success == false) return false;

	if (Host == nullptr) return false;
	Host->OnClicked.AddDynamic(this, &UMainMenu::HostServer);
	
	return true;
}

void UMainMenu::HostServer()
{
	if (MenuInterface == nullptr) return;

	MenuInterface->Host();
}
