// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/VisualData.h"
#include "Weapon.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnAsyncLoadEnded);

UCLASS(BlueprintType)
class PROJECTR_API UWeapon final : public UObject
{
	GENERATED_BODY()
	
public:
	UWeapon();
	bool Initialize(class USkillContext* InContext, int32 InKey);

	void BeginPlay();
	void EndPlay() {}

	void BeginSkill(uint8 Index);
	void EndSkill(uint8 Index);
	
	void Execute(uint8 Index);
	void BeginExecute(uint8 Index);
	void EndExecute(uint8 Index);

	FORCEINLINE const FVisualData& GetVisualData() const noexcept { return VisualData; }

private:
	void LoadAll(const struct FWeaponData& WeaponData);

	void InitSkill(uint8 Level);

public:
	FOnAsyncLoadEnded OnAsyncLoadEnded;

private:
	UPROPERTY(Transient)
	class APRCharacter* User;
	
	UPROPERTY()
	TArray<class USkill*> Skills;

	UPROPERTY(Transient)
	USkillContext* Context;

	UPROPERTY(Transient)
	FVisualData VisualData;

	UPROPERTY()
	class UDataTable* WeaponDataTable;

	UPROPERTY()
	UDataTable* SkillDataTable;

	int32 Key;
	uint8 AsyncLoadCount;
};
