// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/WeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Buff/Cast.h"
#include "Framework/PRCharacter.h"
#include "Library/BuffLibrary.h"
#include "Weapon/Weapon.h"
#include "Weapon/WeaponContext.h"

UWeaponComponent::UWeaponComponent()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = false;

	RightWeapon = CreateWeaponComponent(TEXT("RightWeapon"), TEXT("weapon_r"));
	LeftWeapon = CreateWeaponComponent(TEXT("LeftWeapon"), TEXT("weapon_l"));

	WeaponContext = CreateDefaultSubobject<UWeaponContext>(TEXT("WeaponContext"));
	WeaponContext->Initialize(RightWeapon, LeftWeapon);

	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		NoWeapon = CreateDefaultSubobject<UWeapon>(TEXT("NoWeapon"));
		NoWeapon->Initialize(WeaponContext, 0u);
	}

	WeaponIndex = 0u;
	SkillIndex = 0u;

	Initialize();
}

void UWeaponComponent::Attack(bool bIsStrongAttack)
{
	ServerAttack(bIsStrongAttack);
}

void UWeaponComponent::Parry()
{
	ServerParry();
}

void UWeaponComponent::StopSkill()
{
	ServerStopSkill();
}

void UWeaponComponent::SwapWeapon(uint8 Index)
{
	ServerSwapWeapon(Index);
}

void UWeaponComponent::AddWeapon(uint8 Index, int32 Key)
{
	ServerAddWeapon(Index, Key);
}

#if WITH_EDITOR

void UWeaponComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	static const FName& KeiesName = GET_MEMBER_NAME_CHECKED(UWeaponComponent, Keies);
	if (PropertyChangedEvent.Property && PropertyChangedEvent.GetPropertyName() == KeiesName)
		Initialize();
}

#endif

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	auto* User = Cast<APRCharacter>(GetOwner());
	User->OnDeath.AddDynamic(this, &UWeaponComponent::Detach);
}

UStaticMeshComponent* UWeaponComponent::CreateWeaponComponent(const FName& Name, const FName& SocketName)
{
	auto* Component = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	check(Component);

	Component->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	Component->SetMobility(EComponentMobility::Movable);
	Component->SetCollisionProfileName(TEXT("Weapon"));
	Component->SetGenerateOverlapEvents(true);

	if (auto* Owner = GetOwner())
	{
		auto* Character = CastChecked<ACharacter>(Owner); // Only humanoid characters can have weapons.

		auto* MeshComponent = Character->GetMesh();
		const auto Rules = FAttachmentTransformRules::KeepRelativeTransform;
		if (MeshComponent->DoesSocketExist(SocketName))
			Component->AttachToComponent(MeshComponent, Rules, SocketName);
	}

	return Component;
}

void UWeaponComponent::EquipWeapon(UWeapon* NewWeapon, bool bNeedUnequip)
{
	if (!NewWeapon) return;

	if (bNeedUnequip)
		Weapons[WeaponIndex]->Unequip();

	NewWeapon->Equip();
}

void UWeaponComponent::Initialize()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
		return;

	const uint8 WeaponNum = Keies.Num();
	Weapons.SetNum(WeaponNum);

	for (int32 Idx = 0; Idx < WeaponNum; ++Idx)
	{
		Weapons[Idx] = NewObject<UWeapon>(GetOwner());
		Weapons[Idx]->Initialize(WeaponContext, Keies[Idx]);
	}

	if (Weapons.Num() > 0)
		EquipWeapon(Weapons[0], false);
}

void UWeaponComponent::ServerAttack_Implementation(bool bIsStrongAttack)
{
	SkillIndex *= 2u;
	if (bIsStrongAttack)
		++SkillIndex;

	Weapons[WeaponIndex]->BeginSkill(SkillIndex);
}

bool UWeaponComponent::ServerAttack_Validate(bool bIsStrongAttack)
{
	return true;
}

void UWeaponComponent::ServerParry_Implementation()
{
	Weapons[WeaponIndex]->BeginParrying();
}

bool UWeaponComponent::ServerParry_Validate()
{
	return true;
}

void UWeaponComponent::ServerStopSkill_Implementation()
{
	UWeapon* Weapon = Weapons[WeaponIndex];
	Weapon->EndSkill(SkillIndex);
	Weapon->EndParrying();
	SkillIndex = 0u;
}

bool UWeaponComponent::ServerStopSkill_Validate()
{
	return true;
}

void UWeaponComponent::ServerSwapWeapon_Implementation(uint8 Index)
{
	if (WeaponIndex == Index || UBuffLibrary::IsActivate<UCast>(GetOwner()))
		return;

	EquipWeapon(Weapons[Index], true);
	WeaponIndex = Index;
}

bool UWeaponComponent::ServerSwapWeapon_Validate(uint8 Index)
{
	return Weapons.Num() > Index;
}

void UWeaponComponent::ServerAddWeapon_Implementation(uint8 Index, int32 Key)
{
	const int32 BeforeWeaponNum = Weapons.Num();
	if (Index > BeforeWeaponNum)
		Weapons.SetNum(Index + 1u);

	const int32 AfterWeaponNum = Weapons.Num();
	for (int32 Idx = BeforeWeaponNum; Idx < AfterWeaponNum; ++Idx)
		Weapons[Idx] = NoWeapon;

	auto* NewWeapon = NewObject<UWeapon>(GetOwner());
	NewWeapon->Initialize(WeaponContext, Key);

	if (Index == WeaponIndex)
		EquipWeapon(NewWeapon, true);

	Weapons[Index] = NewWeapon;
}

bool UWeaponComponent::ServerAddWeapon_Validate(uint8 Index, int32 Key)
{
	return true;
}

void UWeaponComponent::Detach()
{
	DetachOnce(RightWeapon);
	DetachOnce(LeftWeapon);
}

void UWeaponComponent::DetachOnce(UStaticMeshComponent* Weapon)
{
	Weapon->SetCollisionProfileName(TEXT("Ragdoll"));
	Weapon->SetSimulatePhysics(true);

	auto Rules = FDetachmentTransformRules::KeepWorldTransform;
	Weapon->DetachFromComponent(Rules);
}
