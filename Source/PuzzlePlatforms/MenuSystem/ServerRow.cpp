// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuSystem/ServerRow.h"

#include "MainMenu.h"
#include "Components/Button.h"

void UServerRow::SetUp(UMainMenu* InParent, uint32 InIndex)
{
	Parent = InParent;
	Index = InIndex;
}

void UServerRow::NativeConstruct()
{
	Super::NativeConstruct();

	RowButton->OnClicked.AddDynamic(this, &UServerRow::OnClicked);
}

void UServerRow::OnClicked()
{
	if (Parent == nullptr)
		return;

	Parent->SelectIndex(Index);
}
