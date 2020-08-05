// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Weapon.generated.h"

DECLARE_DELEGATE(FOnAsyncLoadEndedSingle);
DECLARE_MULTICAST_DELEGATE(FOnAsyncLoadEnded);

UCLASS(BlueprintType)
class PROJECTR_API UWeapon final : public UObject
{
	GENERATED_BODY()
	
public:
	UWeapon();
	void Initialize(class UWeaponContext* InContext, int32 Key);

	void Equip();
	void Unequip();

	void BeginSkill(uint8 Index);
	void EndSkill(uint8 Index);
	
	void BeginParrying();
	void EndParrying();

	FORCEINLINE UWorld* GetWorld() const override
	{
		return GetOuter()->GetWorld();
	}

private:
	void LoadAll(const struct FWeaponData& WeaponData);

private:
	UPROPERTY(Transient)
	class APRCharacter* User;
	
	UPROPERTY()
	TArray<class USkill*> Skills;

	UPROPERTY(Transient)
	USkill* WeakAttack;

	UPROPERTY(Transient)
	USkill* StrongAttack;

	UPROPERTY(Transient)
	USkill* Parrying;

	UPROPERTY(Transient)
	UWeaponContext* Context;

	UPROPERTY(Transient)
	TSubclassOf<class UAnimInstance> UpperAnimInstance;

	UPROPERTY(Transient)
	class UAnimMontage* EquipAnim;

	UPROPERTY(Transient)
	class UStaticMesh* RightWeaponMesh;

	UPROPERTY(Transient)
	UStaticMesh* LeftWeaponMesh;

	UPROPERTY()
	class UDataTable* WeaponDataTable;

	FTransform RightWeaponTransform;
	FTransform LeftWeaponTransform;

	FOnAsyncLoadEnded OnAsyncLoadEnded;

	uint8 AsyncLoadCount;
};
