int stuntJumpsPoolCount;
uintptr_t StuntJumpsSaveBound_BackTo;
extern "C" int StuntJumpsSaveBound_Inject()
{
    return stuntJumpsPoolCount;
}
__attribute__((optnone)) __attribute__((naked)) void StuntJumpsSaveBound_Patch(void)
{
    asm("BL StuntJumpsSaveBound_Inject");
    asm("CMP R6, R0");
    asm volatile("MOV R12, %0\n" :: "r"(StuntJumpsSaveBound_BackTo));
    asm("BX R12");
}

int entryExitsPoolCount;
/*
uintptr_t EntryExitsSaveBound_BackTo;
extern "C" int EntryExitsSaveBound_Inject()
{
    return entryExitsPoolCount * 60;
}
__attribute__((optnone)) __attribute__((naked)) void EntryExitsSaveBound_Patch(void)
{
    asm("PUSH {R0}"); // R0 holds a half-computed literal-pool address still needed after return
    asm("BL EntryExitsSaveBound_Inject");
    asm("MOV R9, R0");
    asm("POP {R0}");
    asm volatile("MOV R12, %0\n" :: "r"(EntryExitsSaveBound_BackTo));
    asm("BX R12");
}
*/

// CStuntJumpManager::Save / CEntryExitManager::Save re-scan their pools with the loop
// bound hardcoded to the vanilla size (256 / 455) instead of the pool's real capacity,
// so entries allocated past that desync the saved data and corrupt the file on load.
void PatchSaveGame()
{
    // CStuntJumpManager::Save: "CMP.W R6, #0x100"
    if(*(uint32_t*)(pGameAddr + 0x48EA24) == 0x7F80F5B6)
    {
        StuntJumpsSaveBound_BackTo = pGameAddr + 0x48EA28 + 0x1;
        aml->Redirect(pGameAddr + 0x48EA24 + 0x1, (uintptr_t)StuntJumpsSaveBound_Patch);
    }

    // CEntryExitManager::Save: "MOVW R9, #0x6AA4" (27300 == 455 * sizeof(CEntryExit))
    // disabled - Load has no bounds check on the index it reads back, so saves with
    // entries past 454 can write out of bounds when loaded on a smaller EntryExits pool.
    /*
    if(*(uint32_t*)(pGameAddr + 0x48EB5C) == 0x29A4F646)
    {
        EntryExitsSaveBound_BackTo = pGameAddr + 0x48EB60 + 0x1;
        aml->Redirect(pGameAddr + 0x48EB5C + 0x1, (uintptr_t)EntryExitsSaveBound_Patch);
    }
    */
}
