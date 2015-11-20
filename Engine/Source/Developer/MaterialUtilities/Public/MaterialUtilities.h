// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleInterface.h"
#include "Runtime/CoreUObject/Public/UObject/ObjectBase.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
#include "Runtime/Engine/Classes/Engine/Texture.h"
#include "Runtime/Engine/Public/SceneTypes.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "RawMesh.h"

/** Structure used for storing intermediate baked down material data/samples*/
struct FFlattenMaterial
{
	FFlattenMaterial()
		: DiffuseSize(0, 0)
		, NormalSize(0, 0)
		, MetallicSize(0, 0)
		, RoughnessSize(0, 0)
		, SpecularSize(0, 0)
		, OpacitySize(0, 0)
		, EmissiveSize(0, 0)
		, bTwoSided(false)
		, BlendMode(BLEND_Opaque)
		, EmissiveScale(1.0f)		
	{}

	/** Release all the sample data */
	void ReleaseData()
	{
		DiffuseSamples.Empty();
		NormalSamples.Empty();
		MetallicSamples.Empty();
		RoughnessSamples.Empty();
		SpecularSamples.Empty();
		OpacitySamples.Empty();
		EmissiveSamples.Empty();
	}
	
	/** Set all alpha channel values with InAlphaValue */
	void FillAlphaValues(const int InAlphaValue)
	{
		for (FColor& Sample : DiffuseSamples) 
		{ 
			Sample.A = InAlphaValue;
		}
		for (FColor& Sample : NormalSamples) 
		{ 
			Sample.A = InAlphaValue;
		}
		for (FColor& Sample : MetallicSamples) 
		{ 
			Sample.A = InAlphaValue;
		}
		for (FColor& Sample : RoughnessSamples) 
		{ 
			Sample.A = InAlphaValue;
		}
		for (FColor& Sample : SpecularSamples)
		{ 
			Sample.A = InAlphaValue;
		}
		for (FColor& Sample : OpacitySamples) 
		{ 
			Sample.A = InAlphaValue;
		}
		for (FColor& Sample : EmissiveSamples)
		{ 
			Sample.A = InAlphaValue; 
		}
	}
	
	/** Material Guid */
	FGuid			MaterialId;
		
	/** Texture sizes for each individual property*/
	FIntPoint		DiffuseSize;
	FIntPoint		NormalSize;
	FIntPoint		MetallicSize;	
	FIntPoint		RoughnessSize;	
	FIntPoint		SpecularSize;	
	FIntPoint		OpacitySize;
	FIntPoint		EmissiveSize;

	/** Baked down texture samples for each individual property*/
	TArray<FColor>	DiffuseSamples;
	TArray<FColor>	NormalSamples;
	TArray<FColor>	MetallicSamples;
	TArray<FColor>	RoughnessSamples;
	TArray<FColor>	SpecularSamples;
	TArray<FColor>	OpacitySamples;
	TArray<FColor>	EmissiveSamples;

	/** Flag whether or not the material will have to be two-sided */
	bool			bTwoSided;
	/** Blend mode for the new material */
	EBlendMode		BlendMode;
	/** Scale (maximum baked down value) for the emissive property */
	float			EmissiveScale;
};

/** Export material proxy cache*/
struct MATERIALUTILITIES_API FExportMaterialProxyCache
{
	// Material proxies for each property. Note: we're not handling all properties here,
	// so hold only up to MP_Normal inclusive.
	class FMaterialRenderProxy* Proxies[EMaterialProperty::MP_Normal + 1];

	FExportMaterialProxyCache();
	~FExportMaterialProxyCache();

	/** Releasing the cached render proxies */
	void Release();
};


class UMaterialInterface;
class UMaterial;
class UTexture2D;
class UTextureRenderTarget2D;
class UWorld;
class ALandscapeProxy;
class ULandscapeComponent;
class FPrimitiveComponentId;
struct FMaterialMergeData;

/**
 * Material utilities
 */
class MATERIALUTILITIES_API FMaterialUtilities : public IModuleInterface
{
public:
	/** Begin IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	/** End IModuleInterface implementation */

	/**
	 * Whether material utilities support exporting specified material blend mode and property 
	 */
	static bool SupportsExport(EBlendMode InBlendMode, EMaterialProperty InMaterialProperty);

	/**
	 * Renders specified material property into texture
	 *
	 * @param InWorld				World object to use for material property rendering
	 * @param InMaterial			Target material
	 * @param InMaterialProperty	Material property to render
	 * @param InRenderTarget		Render target to render to
	 * @param OutBMP				Output array of rendered samples 
	 * @return						Whether operation was successful
	 */	
	DEPRECATED(4.11, "Please use ExportMaterialProperty function with new signature")
	static bool ExportMaterialProperty(UWorld* InWorld, UMaterialInterface* InMaterial, EMaterialProperty InMaterialProperty, UTextureRenderTarget2D* InRenderTarget, TArray<FColor>& OutBMP);

	/**
	* Renders specified material property into texture
	*
	* @param InMaterial			Target material
	* @param InMaterialProperty	Material property to render	
	* @param OutBMP				Output array of rendered samples
	* @param OutSize			Output size of the rendered samples
	* @return					Whether operation was successful
	*/
	static bool ExportMaterialProperty(UMaterialInterface* InMaterial, EMaterialProperty InMaterialProperty, TArray<FColor>& OutBMP, FIntPoint& OutSize );

	/**
	* Renders specified material property into texture
	*
	* @param InMaterial			Target material
	* @param InMaterialProperty	Material property to render
	* @param InSize				Input size for the rendered samples
	* @param OutBMP				Output array of rendered samples	
	* @return					Whether operation was successful
	*/
	static bool ExportMaterialProperty(UMaterialInterface* InMaterial, EMaterialProperty InMaterialProperty, FIntPoint InSize, TArray<FColor>& OutBMP );

	/**
	 * Renders specified material property into texture
	 *
	 * @param InWorld				World object to use for material property rendering
	 * @param InMaterial			Target material
	 * @param InMaterialProperty	Material property to render
	 * @param OutBMP				Output array of rendered samples 
	 * @return						Whether operation was successful
	 */
	DEPRECATED(4.11, "Please use ExportMaterialProperty function with new signature")
	static bool ExportMaterialProperty(UWorld* InWorld, UMaterialInterface* InMaterial, EMaterialProperty InMaterialProperty, FIntPoint& OutSize, TArray<FColor>& OutBMP);

	/**
	 * Flattens specified material
	 *
	 * @param InWorld				World object to use for material rendering
	 * @param InMaterial			Target material
	 * @param OutFlattenMaterial	Output flattened material
	 * @return						Whether operation was successful
	 */
	DEPRECATED(4.11, "Please use ExportMaterial function with new signature")
	static bool ExportMaterial(UWorld* InWorld, UMaterialInterface* InMaterial, FFlattenMaterial& OutFlattenMaterial);
	
	/**
	* Flattens specified material
	*
	* @param InMaterial				Target material
	* @param OutFlattenMaterial		Output flattened material
	* @return						Whether operation was successful
	*/
	static bool ExportMaterial(UMaterialInterface* InMaterial, FFlattenMaterial& OutFlattenMaterial, struct FExportMaterialProxyCache* ProxyCache = NULL);

	/**
	* Flattens specified material using mesh data	
	*
	* @param InMaterial			Target material
	* @param InMesh				Mesh data to use for flattening
	* @param InMaterialIndex	Material index
	* @param InTexcoordBounds	Texture bounds 
	* @param InTexCoords		Replacement non-overlapping texture coordinates
	* @param OutFlattenMaterial Output flattened material
	* @return					Whether operation was successful
	*/
	static bool ExportMaterial(UMaterialInterface* InMaterial, const FRawMesh* InMesh, int32 InMaterialIndex, const FBox2D& InTexcoordBounds, const TArray<FVector2D>& InTexCoords, FFlattenMaterial& OutFlattenMaterial, struct FExportMaterialProxyCache* ProxyCache = NULL);

	/**
	 * Flattens specified landscape material
	 *
	 * @param InLandscape			Target landscape
	 * @param HiddenPrimitives		Primitives to hide while rendering scene to texture
	 * @param OutFlattenMaterial	Output flattened material
	 * @return						Whether operation was successful
	 */
	static bool ExportLandscapeMaterial(ALandscapeProxy* InLandscape, const TSet<FPrimitiveComponentId>& HiddenPrimitives, FFlattenMaterial& OutFlattenMaterial);
	
	/**
 	 * Generates a texture from an array of samples 
	 *
	 * @param Outer					Outer for the material and texture objects, if NULL new packages will be created for each asset
	 * @param AssetLongName			Long asset path for the new texture
	 * @param Size					Resolution of the texture to generate (must match the number of samples)
	 * @param Samples				Color data for the texture
	 * @param CompressionSettings	Compression settings for the new texture
	 * @param LODGroup				LODGroup for the new texture
	 * @param Flags					ObjectFlags for the new texture
	 * @param bSRGB					Whether to set the bSRGB flag on the new texture
	 * @param SourceGuidHash		(optional) Hash (stored as Guid) to use part of the texture source's DDC key.
	 * @return						The new texture.
	 */
	static UTexture2D* CreateTexture(UPackage* Outer, const FString& AssetLongName, FIntPoint Size, const TArray<FColor>& Samples, TextureCompressionSettings CompressionSettings, TextureGroup LODGroup, EObjectFlags Flags, bool bSRGB, const FGuid& SourceGuidHash = FGuid());

	/**
	 * Creates UMaterial object from a flatten material
	 *
	 * @param Outer					Outer for the material and texture objects, if NULL new packages will be created for each asset
	 * @param BaseName				BaseName for the material and texture objects, should be a long package name in case Outer is not specified
	 * @param Flags					Object flags for the material and texture objects.
	 * @param OutGeneratedAssets	List of generated assets - material, textures
	 * @return						Returns a pointer to the constructed UMaterial object.
	 */
	static UMaterial* CreateMaterial(const FFlattenMaterial& InFlattenMaterial, UPackage* InOuter, const FString& BaseName, EObjectFlags Flags, TArray<UObject*>& OutGeneratedAssets);

	/**
	* Creates bakes textures for a ULandscapeComponent
	*
	* @param LandscapeComponent		The component to bake textures for
	* @return						Whether operation was successful
	*/
	static bool ExportBaseColor(ULandscapeComponent* LandscapeComponent, int32 TextureSize, TArray<FColor>& OutSamples);

	/**
	* Creates a FFlattenMaterial instance with the given MaterialProxySettings data
	*
	* @param InMaterialLODSettings  Settings containing how to material should be merged
	* @return						Set up FFlattenMaterial according to the given settings
	*/
	static FFlattenMaterial CreateFlattenMaterialWithSettings(const FMaterialProxySettings& InMaterialLODSettings);

	/**
	* Analyzes given material to determine how many texture coordinates and whether or not vertex colors are used within the material Graph
	*
	* @param InMaterial 			Material to analyze
	* @param InMaterialSettings 	Settings containing how to material should be merged
	* @param OutNumTexCoords		Number of texture coordinates used across all properties flagged for merging
	* @param OutUseVertexColor		Flag whether or not Vertex Colors are used in the material graph for the properties flagged for merging
	*/
	static void AnalyzeMaterial(class UMaterialInterface* InMaterial, const struct FMaterialProxySettings& InMaterialSettings, int32& OutNumTexCoords, bool& OutUseVertexColor);

	/**
	* Remaps material indices where possible to reduce the number of materials required for creating a proxy material
	*
	* @param InMaterials 					List of Material interfaces (non-unique)
	* @param InMeshData						Array of meshes who use the materials in InMaterials
	* @param InMaterialMap					Map of MeshIDAndLOD keys with a material index array as value mapping InMeshData to the InMaterials array
	* @param InMaterialProxySettings		Settings for creating the proxy material
	* @param bBakeVertexData 				Flag whether or not Vertex Data should be baked down
	* @param bMergeMaterials 				Flag whether or not materials with be merged for this mesh
	* @param OutMeshShouldBakeVertexData	Array with Flag for each mesh whether or not Vertex Data should be baked down or is required to
	* @param OutMaterialMap					Map of MeshIDAndLOD keys with a material index array as value mapping InMeshData to the OutMaterials array
	* @param OutMaterials					List of Material interfaces (unique)
	*/
	static void RemapUniqueMaterialIndices(const TArray<UMaterialInterface*>& InMaterials, const TArray<struct FRawMeshExt>& InMeshData, const TMap<FIntPoint, TArray<int32> >& InMaterialMap, const FMaterialProxySettings& InMaterialProxySettings, const bool bBakeVertexData, const bool bMergeMaterials, TArray<bool>& OutMeshShouldBakeVertexData, TMap<FIntPoint, TArray<int32> >& OutMaterialMap, TArray<UMaterialInterface*>& OutMaterials);

	/**
	* Tries to optimize the flatten material's data by picking out constant values for the various properties
	*
	* @param InFlattenMaterial				Flatten material to optimize
	*/
	static void OptimizeFlattenMaterial(FFlattenMaterial& InFlattenMaterial);
private:
	
	/**
	* Private export material function to which unique signatures are mapped for unified code path
	*
	* @param InMaterialData			Target material data 
	* @param OutFlattenMaterial		Output flattened material
	* @return						Whether operation was successful
	*/
	static bool ExportMaterial(struct FMaterialMergeData& InMaterialData, FFlattenMaterial& OutFlattenMaterial, struct FExportMaterialProxyCache* ProxyCache = NULL);

	/**
	* Renders out the specified material property with the given material data to a texture
	*
	* @param InMaterialData			Target material data
	* @param InMaterialProperty		Target material property
	* @param bInForceLinearGamma	Whether or not to force linear gamma (used for Normal property)
	* @param InPixelFormat			Pixel format of the target texture
	* @param InTargetSize			Dimensions of the target texture
	* @param OutSamples				Array of FColor samples containing the rendered out texture pixel data
	* @return						Whether operation was successful
	*/
	static bool RenderMaterialPropertyToTexture(struct FMaterialMergeData& InMaterialData, EMaterialProperty InMaterialProperty, bool bInForceLinearGamma, EPixelFormat InPixelFormat, FIntPoint& InTargetSize, TArray<FColor>& OutSamples);

	/**
	* Creates and add or reuses a RenderTarget from the pool
	*
	* @param bInForceLinearGamma	Whether or not to force linear gamma
	* @param InPixelFormat			Pixel format of the render target
	* @param InTargetSize			Dimensions of the render target
	* @return						Created render target
	*/
	static UTextureRenderTarget2D* CreateRenderTarget(bool bInForceLinearGamma, EPixelFormat InPixelFormat, FIntPoint& InTargetSize);
	
	/** Clears the pool with available render targets */
	static void ClearRenderTargetPool();	

	/** Helper function to make sure all the Material and accompanying textures are fully loaded. This function will do nothing useful if material is already loaded.
	It's intended to work when we're baking a new material during engine startup, inside a PostLoad call - in this case we could have ExportMaterial() call for 
	material which has not all components loaded yet.*/
	static void FullyLoadMaterialStatic(UMaterialInterface* Material);

	/** Call back for garbage collector, cleans up the RenderTargetPool if CurrentlyRendering is set to false */
	void OnPreGarbageCollect();

	/** Tries to optimize the sample array (will set to const value if all samples are equal) */
	static void OptimizeSampleArray(TArray<FColor>& InSamples, FIntPoint& InSampleSize);
private:
	/** Flag to indicate whether or not a texture is currently being rendered out */
	static bool CurrentlyRendering;
	/** Pool of available render targets, cached for re-using on consecutive property rendering */
	static TArray<UTextureRenderTarget2D*> RenderTargetPool;
};
