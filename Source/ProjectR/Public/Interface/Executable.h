// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Executable.generated.h"

UINTERFACE(BlueprintType, MinimalAPI)
class UExecutable : public UInterface
{
	GENERATED_BODY()
};

class PROJECTR_API IExecutable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Execute();

protected:
	virtual void Execute_Implementation() {}
};
