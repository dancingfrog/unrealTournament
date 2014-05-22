// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTWeaponAttachment.generated.h"

/** the third person representation of a weapon
 * not spawned on dedicated servers
 */
UCLASS(Blueprintable, NotPlaceable)
class AUTWeaponAttachment : public AActor
{
	GENERATED_UCLASS_BODY()

protected:
	/** weapon class that resulted in this attachment; set at spawn time so no need to be reflexive here, just set AttachmentType in UTWeapon */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AUTWeapon> WeaponType;
	/** precast of Instigator to UTCharacter */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	AUTCharacter* UTOwner;
public:
	/** particle component for muzzle flash */
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<UParticleSystemComponent*> MuzzleFlash;

	/** third person mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubobjectPtr<USkeletalMeshComponent> Mesh;
	/** third person mesh attach point */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName AttachSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FVector AttachOffset;

	/** particle system for firing effects (instant hit beam and such)
	* particles will be sourced at FireOffset and a parameter HitLocation will be set for the target, if applicable
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<UParticleSystem*> FireEffect;

	virtual void BeginPlay() OVERRIDE;
	virtual void Destroyed() OVERRIDE;

	UFUNCTION(BlueprintNativeEvent)
	void AttachToOwner();
	UFUNCTION(BlueprintNativeEvent)
	void DetachFromOwner();

	/** play firing effects (both muzzle flash and any tracers/beams/impact effects)
	 * use UTOwner's FlashLocation and FireMode to determine firing data
	 * don't play sounds as those are played/replicated from UTWeapon by the server as the Pawn/WeaponAttachment may not be relevant
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void PlayFiringEffects();
	/** stops any looping fire effects */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StopFiringEffects();
};