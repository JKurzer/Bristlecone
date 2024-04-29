// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "UBristleconeConstants.generated.h"
//https://www.tomlooman.com/unreal-engine-developer-settings/ Welp, I've done SOMETHING wrong.
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Bristlecone Settings"))
class UBristleconeConstants : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", meta= (DisplayName = "Reflector IP"))
	FString default_address_c;

};

