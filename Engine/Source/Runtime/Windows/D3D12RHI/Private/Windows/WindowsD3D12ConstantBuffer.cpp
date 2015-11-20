// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	WindowsD3D12ConstantBuffer.cpp: D3D Constant Buffer functions
=============================================================================*/

#include "D3D12RHIPrivate.h"
#include "WindowsD3D12ConstantBuffer.h"

void FWinD3D12ConstantBuffer::InitDynamicRHI()
{
    CurrentBuffer = new FD3D12ResourceLocation;
	FD3D12ConstantBuffer::InitDynamicRHI();
}

void FWinD3D12ConstantBuffer::ReleaseDynamicRHI()
{
	FD3D12ConstantBuffer::ReleaseDynamicRHI();
    CurrentBuffer = nullptr;
}

bool FWinD3D12ConstantBuffer::CommitConstantsToDevice(FD3D12DynamicHeapAllocator& UploadHeap, bool bDiscardSharedConstants)
{
	// New circular buffer system for faster constant uploads.  Avoids CopyResource and speeds things up considerably
	if (CurrentUpdateSize == 0)
	{
		return false;
	}

	SCOPE_CYCLE_COUNTER(STAT_D3D12GlobalConstantBufferUpdateTime);

	if ( bDiscardSharedConstants )
	{
		// If we're discarding shared constants, just use constants that have been updated since the last Commit.
		TotalUpdateSize = CurrentUpdateSize;
	}
	else
	{
		// If we're re-using shared constants, use all constants.
		TotalUpdateSize = FMath::Max( CurrentUpdateSize, TotalUpdateSize );
	}

	// This only used for the IndexSlot==0 constant buffer on the vertex shader.
	// Which will have an indeterminate number of constant values that are generated by material shaders.

    // Get the next constant buffer
    void *pData = UploadHeap.FastAlloc(TotalUpdateSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, CurrentBuffer);

	check(((uint64)(ShadowData)& 0xf) == 0);
	FMemory::Memcpy(pData, ShadowData, TotalUpdateSize);

	CurrentUpdateSize = 0;

	return true;
}

void FD3D12CommandContext::InitConstantBuffers()
{
	// Allocate shader constant buffers.  All shader types can have access to all buffers.
	//  Index==0 is reserved for "custom" params, and the rest are reserved by the system for Common
	//	constants
	VSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	PSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	HSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	DSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	GSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	CSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	for(int32 BufferIndex = 0;BufferIndex < MAX_CONSTANT_BUFFER_SLOTS;BufferIndex++)
	{
		uint32 Size = GConstantBufferSizes[BufferIndex];
		uint32 SubBuffers = 1;
		if(BufferIndex == GLOBAL_CONSTANT_BUFFER_INDEX)
		{
			SubBuffers = 5;
		}

		// Vertex shader can have subbuffers for index==0.  This is from Epic's original design for the auto-fit of size to
		//	reduce the update costs of the buffer.

		// New circular buffer system for faster constant uploads.  Avoids CopyResource and speeds things up considerably
		VSConstantBuffers.Add(new FWinD3D12ConstantBuffer(GetParentDevice(), Size, SubBuffers));
		PSConstantBuffers.Add(new FWinD3D12ConstantBuffer(GetParentDevice(), Size, SubBuffers));
		HSConstantBuffers.Add(new FWinD3D12ConstantBuffer(GetParentDevice(), Size));
		DSConstantBuffers.Add(new FWinD3D12ConstantBuffer(GetParentDevice(), Size));
		GSConstantBuffers.Add(new FWinD3D12ConstantBuffer(GetParentDevice(), Size));
		CSConstantBuffers.Add(new FWinD3D12ConstantBuffer(GetParentDevice(), Size));
	}
}

DEFINE_STAT(STAT_D3D12GlobalConstantBufferUpdateTime);
