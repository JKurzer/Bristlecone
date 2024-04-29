// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "UBristleconeConstants.generated.h"

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Save Game Settings"))
class UBristleconeConstants : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", AdvancedDisplay)
	FString default_address_c;

};

