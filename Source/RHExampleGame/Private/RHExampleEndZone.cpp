#include "GameFramework/RHGameModeBase.h"
#include "RHExampleEndZone.h"

void ARHExampleEndZone::AddPrimitiveToPawnOverlapCheck(UPrimitiveComponent* InPrimitive)
{
	// Authority Only
	if (!HasAuthority())
	{
		return;
	}

	if (InPrimitive != nullptr && !RegisteredPawnOverlapComponents.Contains(InPrimitive))
	{
		ECollisionResponse CollResp = InPrimitive->GetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn);
		if (CollResp != ECollisionResponse::ECR_Overlap)
		{
			//UE_LOG(RHExampleGame, Warning, TEXT("ARHExampleEndZone::AddPrimitiveToPawnOverlapCheck The primitive %s needs to overlap with the Pawn collision channel. It will not be added to the list of potential overlap components."), *GetNameSafe(InPrimitive));
			return;
		}

		RegisteredPawnOverlapComponents.Add(InPrimitive);
		InPrimitive->OnComponentBeginOverlap.AddDynamic(this, &ARHExampleEndZone::OnBeginOverlap);
		InPrimitive->OnComponentEndOverlap.AddDynamic(this, &ARHExampleEndZone::OnEndOverlap);

		if (IsActorInitialized())
		{
			TArray<AActor*> OverlappingActors;
			InPrimitive->GetOverlappingActors(OverlappingActors, AActor::StaticClass());
			for (AActor* pActor : OverlappingActors)
			{
				ACharacter* pCharacter = Cast<ACharacter>(pActor);
				if (pCharacter != nullptr)
				{
					AddOverlappingPawnPrivate(pCharacter);
				}
			}
		}
	}
}

void ARHExampleEndZone::RemovePrimitiveToPawnOverlapCheck(UPrimitiveComponent* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		int32 nIndex = RegisteredPawnOverlapComponents.Find(InPrimitive);
		if (nIndex != INDEX_NONE)
		{
			InternalRemovePrimitiveToPawnOverlapCheck(InPrimitive);
			RegisteredPawnOverlapComponents.RemoveAtSwap(nIndex);
		}
	}
}

void ARHExampleEndZone::UpdateStateFull()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* pWorld = GetWorld();
	if (pWorld != nullptr)
	{
		ARHGameModeBase* pGameMode = pWorld->GetAuthGameMode<ARHGameModeBase>();
		if (pGameMode != nullptr)
		{
			pGameMode->EndMatch();
		}
	}
}

void ARHExampleEndZone::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (RegisteredPawnOverlapComponents.Contains(OverlappedComponent))
	{
		AddOverlappingPawnPrivate(Cast<ACharacter>(OtherActor));
	}
}

void ARHExampleEndZone::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (RegisteredPawnOverlapComponents.Contains(OverlappedComponent))
	{
		ACharacter* pCharacter = Cast<ACharacter>(OtherActor);
		if (pCharacter != nullptr && !IsOverlappingOtherComponent(OtherActor, OverlappedComponent))
		{
			RemoveOverlappingPawnPrivate(pCharacter);
		}
	}
}

void ARHExampleEndZone::AddOverlappingPawnPrivate(ACharacter* InCharacter)
{
	if (InCharacter != nullptr && !OverlappingPawns.Contains(InCharacter))
	{
		OverlappingPawns.Add(InCharacter);
		UpdateStateFull();
	}
}

void ARHExampleEndZone::RemoveOverlappingPawnPrivate(ACharacter* InCharacter)
{
	if (InCharacter != nullptr)
	{
		int32 nIndex = OverlappingPawns.Find(InCharacter);
		if (nIndex != INDEX_NONE)
		{
			OverlappingPawns.RemoveAtSwap(nIndex);
			UpdateStateFull();
		}
	}
}

void ARHExampleEndZone::InternalRemovePrimitiveToPawnOverlapCheck(UPrimitiveComponent* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		InPrimitive->OnComponentBeginOverlap.RemoveDynamic(this, &ARHExampleEndZone::OnBeginOverlap);
		InPrimitive->OnComponentEndOverlap.RemoveDynamic(this, &ARHExampleEndZone::OnEndOverlap);

		if (IsActorInitialized())
		{
			TArray<AActor*> OverlappingActors;
			InPrimitive->GetOverlappingActors(OverlappingActors, ARHExampleEndZone::StaticClass());
			for (AActor* pActor : OverlappingActors)
			{
				ACharacter* pCharacter = Cast<ACharacter>(pActor);
				if (pCharacter != nullptr)
				{
					if (!IsOverlappingOtherComponent(pActor, InPrimitive))
					{
						RemoveOverlappingPawnPrivate(pCharacter);
					}
				}
			}
		}
	}
}

bool ARHExampleEndZone::IsOverlappingOtherComponent(AActor* InActor, UPrimitiveComponent* InExcludeComponent) const
{
	if (InActor != nullptr)
	{
		for (const UPrimitiveComponent* pPrimitive : RegisteredPawnOverlapComponents)
		{
			if (pPrimitive != nullptr && pPrimitive != InExcludeComponent && pPrimitive->IsOverlappingActor(InActor))
			{
				return true;
			}
		}
	}

	return false;
}
