#pragma once
#include "CoreMinimal.h"
#include "Serialization/PackageWriter.h"
#include "Serialization/BasePackageWriter.h"
#include "PackageWriterToSharedBuffer.h"

class FHotPatcherPackageWriter:public TPackageWriterToSharedBuffer<FBaseCookedPackageWriter>
{
public:
	virtual FCookCapabilities GetCookCapabilities() const override
	{
		FCookCapabilities Result;
		Result.bDiffModeSupported = true;
		return Result;
	}

	virtual void BeginPackage(const FBeginPackageInfo& Info) override;
	virtual FDateTime GetPreviousCookTime() const override;
	virtual void Initialize(const FCookInfo& Info) override;
	virtual void BeginCook(
		const FCookInfo& Info
	) override;
	virtual void EndCook(
		const FCookInfo& Info
	) override;
	// virtual void Flush() override;
	
	virtual FCbObject GetOplogAttachment(FName PackageName, FUtf8StringView AttachmentKey) override;
	virtual void PopulateOplog(const FAssetRegistryState& PreviousState, int32& OutNumPackagesInOplog) override;
	virtual void UpdateLastReferenceDateAndPruneStaleOps(UE::Cook::Artifact::FUpdateOplogPackagesContext& OplogContext) override;
	virtual TArray<FName> GetOplogPackageNames() override;
	virtual void GetOplogAttachments(TArrayView<FName> PackageNames,
		TArrayView<FUtf8StringView> AttachmentKeys,
		TUniqueFunction<void(FName PackageName, FUtf8StringView AttachmentKey, FCbObject&& Attachment)>&& Callback) override;
	virtual void GetBaseGameOplogAttachments(TArrayView<FName> PackageNames,
		TArrayView<FUtf8StringView> AttachmentKeys,
		TUniqueFunction<void(FName PackageName, FUtf8StringView AttachmentKey, FCbObject&& Attachment)>&& Callback) override;
	virtual ECommitStatus GetCommitStatus(FName PackageName) override;
	virtual void RemoveCookedPackages(TArrayView<const FName> PackageNamesToRemove) override;
	virtual void RemoveCookedPackages() override;
	virtual void UpdatePackageModifiedStatus(FUpdatePackageModifiedStatusContext& Context) override;
	virtual void SetCooker(UE::PackageWriter::Private::ICookerInterface* CookerInterface) override;
	virtual bool GetPreviousCookedBytes(const FPackageInfo& Info, FPreviousCookedBytesData& OutData) override;
	virtual EPackageWriterResult BeginCacheForCookedPlatformData(FBeginCacheForCookedPlatformDataInfo& Info){ return EPackageWriterResult::Success;}
	virtual void CommitPackageInternal(FPackageRecord&& Record,const IPackageWriter::FCommitPackageInfo& Info)override;

	virtual FPackageWriterRecords::FPackage* ConstructRecord() override;
	virtual TFuture<FCbObject> WriteMPCookMessageForPackage(FName PackageName) override;
	/** Read PackageData written by WriteMPCookMessageForPackage on a CookWorker. Called only on CookDirector. */
	virtual bool TryReadMPCookMessageForPackage(FName PackageName, FCbObjectView Message) override { return false; }
	// virtual TFuture<FMD5Hash> CommitPackage(FCommitPackageInfo&& Info)override;
	// virtual void WritePackageData(const FPackageInfo& Info, FLargeMemoryWriter& ExportsArchive, const TArray<FFileRegion>& FileRegions) override;
	// virtual void WriteBulkData(const FBulkDataInfo& Info, const FIoBuffer& BulkData, const TArray<FFileRegion>& FileRegions) override;
	// virtual void WriteLinkerAdditionalData(const FLinkerAdditionalDataInfo& Info, const FIoBuffer& Data, const TArray<FFileRegion>& FileRegions) override;

private:
	/** Version of the superclass's per-package record that includes our class-specific data. */
	struct FRecord : public FPackageWriterRecords::FPackage
	{
		bool bCompletedExportsArchiveForDiff = false;
	};
	
	/** Buffers that are combined into the HeaderAndExports file (which is then split into .uasset + .uexp or .uoasset + .uoexp). */
	struct FExportBuffer
	{
		FSharedBuffer Buffer;
		TArray<FFileRegion> Regions;
	};

	/**
	 * The data needed to asynchronously write one of the files (.uasset, .uexp, .ubulk, any optional and any additional),
	 * without reference back to other data on this writer.
	 */
	struct FWriteFileData
	{
		FString Filename;
		FCompositeBuffer Buffer;
		TArray<FFileRegion> Regions;
		bool bIsSidecar;
		bool bContributeToHash = true;

		void Write(FMD5& AccumulatedHash, EWriteOptions WriteOptions) const;
	};

	/** Stack data for the helper functions of CommitPackageInternal. */
	struct FCommitContext
	{
		const FCommitPackageInfo& Info;
		TArray<TArray<FExportBuffer>> ExportsBuffers;
		TArray<FWriteFileData> OutputFiles;
	};

	void CollectForSavePackageData(FRecord& Record, FCommitContext& Context);
	void CollectForSaveBulkData(FRecord& Record, FCommitContext& Context);
	void CollectForSaveLinkerAdditionalDataRecords(FRecord& Record, FCommitContext& Context);
	void CollectForSaveAdditionalFileRecords(FRecord& Record, FCommitContext& Context);
	void CollectForSaveExportsFooter(FRecord& Record, FCommitContext& Context);
	void CollectForSaveExportsBuffers(FRecord& Record, FCommitContext& Context);

	TMap<FName, TRefCountPtr<FPackageHashes>>& GetPackageHashes() override
	{
		return AllPackageHashes;
	}
	// If EWriteOptions::ComputeHash is not set, the package will not get added to this.
	TMap<FName, TRefCountPtr<FPackageHashes>> AllPackageHashes;
	
	TFuture<FMD5Hash> AsyncSave(FRecord& Record, const FCommitPackageInfo& Info);
	TFuture<FMD5Hash> AsyncSaveOutputFiles(FRecord& Record, FCommitContext& Context);
};

