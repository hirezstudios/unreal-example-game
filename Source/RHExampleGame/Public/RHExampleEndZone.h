#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "RHExampleEndZone.generated.h"

UCLASS()
class RHEXAMPLEGAME_API ARHExampleEndZone : public AActor
{
	GENERATED_BODY()
	
public:	
	
	UFUNCTION(BlueprintCallable)
	void AddPrimitiveToPawnOverlapCheck(UPrimitiveComponent* InPrimitive);
	
	UFUNCTION(BlueprintCallable)
	void RemovePrimitiveToPawnOverlapCheck(UPrimitiveComponent* InPrimitive);

	UFUNCTION()
	virtual void UpdateStateFull();

private:

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void AddOverlappingPawnPrivate(ACharacter* InCharacter);

	void RemoveOverlappingPawnPrivate(ACharacter* InCharacter);

	void InternalRemovePrimitiveToPawnOverlapCheck(UPrimitiveComponent* InPrimitive);

	bool IsOverlappingOtherComponent(AActor* InActor, UPrimitiveComponent* InExcludeComponent) const;

	UPROPERTY()
	TArray<UPrimitiveComponent*> RegisteredPawnOverlapComponents;

	UPROPERTY()
	TArray<ACharacter*> OverlappingPawns;

};
