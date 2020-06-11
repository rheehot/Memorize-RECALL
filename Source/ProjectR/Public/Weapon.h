// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/AssetManager.h"
#include "Weapon.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnAsyncLoadEndedSingle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAsyncLoadEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInactive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeginSkill);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndSkill);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeginAttack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndAttack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShoot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExecute);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeftWeaponHitted, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRightWeaponHitted, AActor*, Target);

USTRUCT(Atomic, BlueprintType)
struct PROJECTR_API FWeaponInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform;
};

UCLASS(BlueprintType)
class PROJECTR_API AWeapon final : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();

	void Initialize(const struct FWeaponData* WeaponData);

	UFUNCTION(BlueprintCallable)
	void Equip();

	UFUNCTION(BlueprintCallable)
	void Unequip();

	UFUNCTION(BlueprintCallable)
	bool UseSkill(uint8 Index);

	UFUNCTION(BlueprintCallable)
	bool CanUseSkill(uint8 Index);

	UFUNCTION()
	void BeginSkill(UAnimMontage* Montage);

	UFUNCTION()
	void EndSkill(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintCallable)
	void RegisterOnAsyncLoadEnded(const FOnAsyncLoadEndedSingle& Callback);

	FORCEINLINE class UBlendSpaceBase* GetLocomotionSpace() const noexcept { return LocomotionSpace; }
	FORCEINLINE class UAnimSequenceBase* GetJumpStart() const noexcept { return JumpStart; }
	FORCEINLINE UAnimSequenceBase* GetJumpLoop() const noexcept { return JumpLoop; }
	FORCEINLINE UAnimSequenceBase* GetJumpEnd() const noexcept { return JumpEnd; }

private:
	void BeginPlay() override;

	void EquipOnce(UStaticMeshComponent* Weapon, const FWeaponInfo& Info);
	void UnequipOnce(UStaticMeshComponent* Weapon);

	template <class T>
	void AsyncLoad(T* Ptr, const TAssetPtr<T>& SoftPtr)
	{
		if (SoftPtr.IsNull())
		{
			CheckAndCallAsyncLoadDelegate();
			return;
		}

		if (SoftPtr.IsPending())
		{
			UAssetManager::GetStreamableManager().RequestAsyncLoad(
				SoftPtr.ToSoftObjectPath(),
				FStreamableDelegate::CreateLambda([this, &Ptr = Ptr, &SoftPtr = SoftPtr]() mutable
					{ OnAsyncLoaded(Ptr, SoftPtr); })
			);
		}
		else OnAsyncLoaded(Ptr, SoftPtr);
	}

	template <class T>
	void OnAsyncLoaded(T* Ptr, const TAssetPtr<T>& SoftPtr)
	{
		Ptr = SoftPtr.Get();
		CheckAndCallAsyncLoadDelegate();
	}

	UFUNCTION()
	void OnLeftWeaponOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRightWeaponOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	FORCEINLINE void CheckAndCallAsyncLoadDelegate() { if (--AsyncLoadCount == 0) OnAsyncLoadEnded.Broadcast(); }

public:
	UPROPERTY(BlueprintAssignable)
	FOnActive OnActive;

	UPROPERTY(BlueprintAssignable)
	FOnInactive OnInactive;

	UPROPERTY(BlueprintAssignable)
	FOnBeginSkill OnBeginSkill;

	UPROPERTY(BlueprintAssignable)
	FOnEndSkill OnEndSkill;

	UPROPERTY(BlueprintAssignable)
	FOnBeginAttack OnBeginAttack;

	UPROPERTY(BlueprintAssignable)
	FOnEndAttack OnEndAttack;

	UPROPERTY(BlueprintAssignable)
	FOnShoot OnShoot;

	UPROPERTY(BlueprintAssignable)
	FOnExecute OnExecute;

	UPROPERTY(BlueprintAssignable)
	FOnLeftWeaponHitted OnLeftWeaponHitted;

	UPROPERTY(BlueprintAssignable)
	FOnRightWeaponHitted OnRightWeaponHitted;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* LeftWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* RightWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = true))
	FWeaponInfo LeftWeaponInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = true))
	FWeaponInfo RightWeaponInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Info, meta = (AllowPrivateAccess = true))
	int32 Key;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Info, meta = (AllowPrivateAccess = true))
	FName Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Skill, meta = (AllowPrivateAccess = true))
	TArray<class ASkill*> Skills;

	UPROPERTY(VisibleAnywhere, Category = Animation, meta = (AllowPrivateAccess = true))
	UBlendSpaceBase* LocomotionSpace;

	UPROPERTY(VisibleAnywhere, Category = Animation, meta = (AllowPrivateAccess = true))
	UAnimSequenceBase* JumpStart;

	UPROPERTY(VisibleAnywhere, Category = Animation, meta = (AllowPrivateAccess = true))
	UAnimSequenceBase* JumpLoop;

	UPROPERTY(VisibleAnywhere, Category = Animation, meta = (AllowPrivateAccess = true))
	UAnimSequenceBase* JumpEnd;

	UPROPERTY(VisibleAnywhere, Category = Animation, meta = (AllowPrivateAccess = true))
	UAnimMontage* DodgeMontage;

	UPROPERTY(VisibleAnywhere, Category = Animation, meta = (AllowPrivateAccess = true))
	UAnimMontage* EquipMontage;

	FOnAsyncLoadEnded OnAsyncLoadEnded;

	uint8 AsyncLoadCount;
};