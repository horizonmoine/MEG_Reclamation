#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "ProcGen/LiminalLayoutLibrary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegExpeditionSeedCampaign,
	"Project.Functional Tests.MEG.ProcGen.ExpeditionSeedCampaign", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMegExpeditionSeedCampaign::RunTest(const FString& Parameters)
{
	const int32 Widths[] = {24, 36, 48, 64};
	const int32 Counts[] = {8, 16, 24, 36};
	for (int32 Config = 0; Config < 4; ++Config)
	{
		for (int32 Seed = 0; Seed < 1000; ++Seed)
		{
			FGeneratedLayout Layout;
			if (!FLiminalLayoutBuilder::TryGenerateExpedition(Seed, Widths[Config], Widths[Config], Counts[Config], 3, 8, 0.35f, 400.0f, Layout))
			{
				AddError(FString::Printf(TEXT("Rejected seed=%d width=%d"), Seed, Widths[Config]));
				return false;
			}
			if (!Layout.Rooms.IsValidIndex(Layout.ExtractionRoomIndex)) return false;
			const TArray<int32> Distances = Layout.GetRoomPathLengths();
			const FProcRoom& Spawn = Layout.Rooms[0];
			const FProcRoom& Exit = Layout.Rooms[Layout.ExtractionRoomIndex];
			const FVector2D Delta(Exit.CenterX - Spawn.CenterX, Exit.CenterY - Spawn.CenterY);
			if (Distances.Contains(INDEX_NONE) || Distances[Layout.ExtractionRoomIndex] * 400.0f < 8000.0f || Delta.Size() * 400.0f < 8000.0f)
			{
				AddError(FString::Printf(TEXT("Invalid distance/connectivity seed=%d width=%d"), Seed, Widths[Config]));
				return false;
			}
			FGeneratedLayout Repeated;
			if (!FLiminalLayoutBuilder::TryGenerateExpedition(Seed, Widths[Config], Widths[Config], Counts[Config], 3, 8, 0.35f, 400.0f, Repeated) || Repeated.Hash != Layout.Hash || Repeated.ExtractionRoomIndex != Layout.ExtractionRoomIndex || Repeated.Cells != Layout.Cells)
			{
				AddError(FString::Printf(TEXT("Non deterministic seed=%d width=%d"), Seed, Widths[Config]));
				return false;
			}
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegExpeditionFailureSafety,
	"Project.Functional Tests.MEG.ProcGen.ExpeditionFailureSafety", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMegExpeditionFailureSafety::RunTest(const FString& Parameters)
{
	FGeneratedLayout Existing;
	Existing.Hash = 12345u;
	TestFalse(TEXT("Impossible expedition rejected"), FLiminalLayoutBuilder::TryGenerateExpedition(0, 8, 8, 2, 2, 3, 0.0f, 400.0f, Existing));
	TestEqual(TEXT("Failure preserves output"), Existing.Hash, 12345u);
	TestFalse(TEXT("Zero cell size rejected"), FLiminalLayoutBuilder::TryGenerateExpedition(0, 24, 24, 8, 3, 8, 0.35f, 0.0f, Existing));
	TestTrue(TEXT("Fallback rescues unusable random room configuration"), FLiminalLayoutBuilder::TryGenerateExpedition(0, 24, 24, 1, 3, 8, 0.35f, 400.0f, Existing));
	TestEqual(TEXT("Fallback contains two rooms"), Existing.Rooms.Num(), 2);
	TestTrue(TEXT("Fallback remains connected"), Existing.IsEveryRoomConnected());
	TestFalse(TEXT("X out of bounds cannot wrap into another row"), Existing.IsFloor(-1, 2));
	TestTrue(TEXT("Invalid dimensions safely rejected"), FLiminalLayoutBuilder::Generate(0, -1, 8, 2, 2, 3, 0.0f).Rooms.IsEmpty());
	return true;
}
#endif