// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"

#include "ServerRow.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/KismetSystemLibrary.h"

UMainMenu::UMainMenu()
{
	ConstructorHelpers::FClassFinder<UUserWidget> ServerRowBPClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/MenuSystem/WBP_ServerRow.WBP_ServerRow_C'"));
	if (ensure(ServerRowBPClass.Class != nullptr) == false)
		return;

	ServerRowClass = ServerRowBPClass.Class;
}

void UMainMenu::SetServerList(TArray<FServerData> ServerData)
{
	UWorld* World = GetWorld();
	if (ensure(World != nullptr) == false)
		return;

	ServerList->ClearChildren();

	uint32 i = 0;
	for (const FServerData& Data : ServerData)
	{
		if (ensure(ServerRowClass != nullptr) == false)
			return;

		UServerRow* Row = CreateWidget<UServerRow>(World, ServerRowClass);
		if (ensure(Row != nullptr) == false)
			return;
		if (ensure(ServerList != nullptr) == false)
			return;

		Row->ServerName->SetText(FText::FromString(Data.Name));
		Row->HostUser->SetText(FText::FromString(Data.HostUserName));
		FString FractionText = FString::Printf(TEXT("%d/%d"), Data.CurrentPlayers, Data.MaxPlayers);
		Row->ConnectionFraction->SetText(FText::FromString(FractionText));
		Row->SetUp(this, i);
		++i;

		ServerList->AddChild(Row);	
	}
}

void UMainMenu::SelectIndex(uint32 Index)
{
	SelectedIndex = Index;
	UpdateChildren();
}

void UMainMenu::UpdateChildren()
{
	for (int32 i = 0; i < ServerList->GetChildrenCount(); ++i)
	{
		auto Row = Cast<UServerRow>(ServerList->GetChildAt(i));
		if (Row != nullptr)
		{
			Row->Selected = (SelectedIndex.IsSet() && SelectedIndex.GetValue() == i);
		}
	}
}

bool UMainMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (Success == false)
		return false;

	if (HostButton == nullptr)
		return false;
	HostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);

	if (JoinButton == nullptr)
		return false;
	JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

	if (CancelJoinMenuButton == nullptr)
		return false;
	CancelJoinMenuButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

	if (ConfirmJoinMenuButton == nullptr)
		return false;
	ConfirmJoinMenuButton->OnClicked.AddDynamic(this, &UMainMenu::JoinServer);

	if (ConfirmHostMenuButton == nullptr)
		return false;
	ConfirmHostMenuButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

	if (CancelHostMenuButton == nullptr)
		return false;
	CancelHostMenuButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

	if (QuitButton == nullptr)
		return false;
	QuitButton->OnClicked.AddDynamic(this, &UMainMenu::Quit);
	
	return true;
}

void UMainMenu::HostServer()
{
	if (MenuInterface == nullptr)
		return;

	FString ServerName = ServerHostName->GetText().ToString();
	MenuInterface->Host(ServerName);
}

void UMainMenu::JoinServer()
{
	if (SelectedIndex.IsSet() && MenuInterface != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected index %d."), SelectedIndex.GetValue());
		MenuInterface->Join(SelectedIndex.GetValue());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected index not set."));
	}
}

void UMainMenu::OpenJoinMenu()
{
	if (MenuSwitcher == nullptr)
		return;
	if (JoinMenu == nullptr)
		return;

	MenuSwitcher->SetActiveWidget(JoinMenu);

	if (MenuInterface == nullptr)
		return;

	MenuInterface->RefreshServerList();
}

void UMainMenu::OpenMainMenu()
{
	if (MenuSwitcher == nullptr)
		return;
	if (MainMenu == nullptr)
		return;

	MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::OpenHostMenu()
{
	if (MenuSwitcher == nullptr)
		return;
	if (HostMenu == nullptr)
		return;

	MenuSwitcher->SetActiveWidget(HostMenu);
}

void UMainMenu::Quit()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;
	
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController == nullptr) return;

	UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, false);
}


