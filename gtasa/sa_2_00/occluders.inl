int occludersCount, interiorOccludersCount, activeOccludersCount;

uintptr_t GlobalOccluders_AddOne_Continue, GlobalOccluders_AddOne_Break, InteriorOccluders_AddOne_Continue, InteriorOccluders_AddOne_Break;
extern "C" uintptr_t GlobalOccluders_AddOne_Inject(int val)
{
    return (val < occludersCount) ? GlobalOccluders_AddOne_Continue : GlobalOccluders_AddOne_Break;
}
extern "C" uintptr_t InteriorOccluders_AddOne_Inject(int val)
{
    return (val < interiorOccludersCount) ? InteriorOccluders_AddOne_Continue : InteriorOccluders_AddOne_Break;
}
__attribute__((optnone)) __attribute__((naked)) void GlobalOccluders_AddOne_Patch(void)
{
    asm("LDR R0, [R0]");
    asm("LDR R0, [R0]");
    asm("PUSH {R0}"); // the continue target reuses R0 as the occluder count
    asm("BL GlobalOccluders_AddOne_Inject");
    asm("MOV R12, R0");
    asm("POP {R0}");
    asm("BX R12");
}
__attribute__((optnone)) __attribute__((naked)) void InteriorOccluders_AddOne_Patch(void)
{
    asm("LDR R0, [R0]");
    asm("LDR R0, [R0]");
    asm("PUSH {R0}"); // the continue target reuses R0 as the occluder count
    asm("BL InteriorOccluders_AddOne_Inject");
    asm("MOV R12, R0");
    asm("POP {R0}");
    asm("BX R12");
}

uintptr_t ActiveOccludersInterior_Continue, ActiveOccludersInterior_Break, ActiveOccludersNearby_Continue, ActiveOccludersNearby_Break;
extern "C" uintptr_t ActiveOccludersInterior_Inject(int val)
{
    return (val < activeOccludersCount) ? ActiveOccludersInterior_Continue : ActiveOccludersInterior_Break;
}
extern "C" uintptr_t ActiveOccludersNearby_Inject(int val)
{
    return (val < activeOccludersCount) ? ActiveOccludersNearby_Continue : ActiveOccludersNearby_Break;
}
__attribute__((optnone)) __attribute__((naked)) void ActiveOccludersInterior_Patch(void)
{
    asm("PUSH {R0}"); // the continue target reuses R0 as the active-occluder index
    asm("BL ActiveOccludersInterior_Inject");
    asm("MOV R12, R0");
    asm("POP {R0}");
    asm("BX R12");
}
__attribute__((optnone)) __attribute__((naked)) void ActiveOccludersNearby_Patch(void)
{
    asm("PUSH {R0, LR}"); // R0 = active-occluder index; LR is reused as a persistent stack-data pointer by this loop, on both the continue and break paths
    asm("BL ActiveOccludersNearby_Inject");
    asm("MOV R12, R0");
    asm("POP {R0, LR}");
    asm("BX R12");
}

void PatchOccluders()
{
    // Occluder distance
    float occludersMaxDist = cfg->GetFloat("OccludersMaxDistance", 3 * 300.0f, "Misc");
    aml->WriteFloat(pGameAddr + 0x5AEFB0, occludersMaxDist);
    // Occluders (non-interior)
    if(*(uint32_t*)(pGameAddr + 0x678444) == (pGameAddr + 0x00A41140))
    {
        static char* aOccluders;

        occludersCount = cfg->GetInt("GlobalOccluders", ADJUSTED_POOL_LIMIT(1000), "Misc");
        aOccluders = new char[18 * occludersCount];

        aml->WriteAddr(pGameAddr + 0x678444, aOccluders);

        GlobalOccluders_AddOne_Continue = pGameAddr + 0x5AE3B8 + 0x1;
        GlobalOccluders_AddOne_Break =    pGameAddr + 0x5AE488 + 0x1;
        aml->Redirect(pGameAddr + 0x5AE3AE + 0x1, (uintptr_t)GlobalOccluders_AddOne_Patch);
    }
    // Occluders (interior)
    if(*(uint32_t*)(pGameAddr + 0x6791D4) == (pGameAddr + 0x00A45794))
    {
        static char* aInteriorOccluders;

        interiorOccludersCount = cfg->GetInt("InteriorOccluders", ADJUSTED_POOL_LIMIT(40), "Misc");
        aInteriorOccluders = new char[18 * interiorOccludersCount];

        aml->WriteAddr(pGameAddr + 0x6791D4, aInteriorOccluders);

        InteriorOccluders_AddOne_Continue = pGameAddr + 0x5AE2FC + 0x1;
        InteriorOccluders_AddOne_Break =    pGameAddr + 0x5AE2FA + 0x1;
        aml->Redirect(pGameAddr + 0x5AE2F2 + 0x1, (uintptr_t)InteriorOccluders_AddOne_Patch);
    }
    // Occluders (active)
    if(*(uint32_t*)(pGameAddr + 0x676D6C) == (pGameAddr + 0x00A45A68))
    {
        static char* aActiveOccluders;

        activeOccludersCount = cfg->GetInt("ActiveOccluders", ADJUSTED_POOL_LIMIT(28), "Misc");
        aActiveOccluders = new char[172 * activeOccludersCount];

        aml->WriteAddr(pGameAddr + 0x676D6C, aActiveOccluders);

        // COcclusion::ProcessBeforeRendering: interior-occluders loop (compiler-rotated,
        // the "<= 27" cap check appears twice, both sharing the same continue/break targets)
        ActiveOccludersInterior_Continue = pGameAddr + 0x5AE578 + 0x1;
        ActiveOccludersInterior_Break =    pGameAddr + 0x5AE590 + 0x1;
        aml->Redirect(pGameAddr + 0x5AE566 + 0x1, (uintptr_t)ActiveOccludersInterior_Patch);
        aml->Redirect(pGameAddr + 0x5AE574 + 0x1, (uintptr_t)ActiveOccludersInterior_Patch);

        // COcclusion::ProcessBeforeRendering: nearby (outdoor) occluders loop
        ActiveOccludersNearby_Continue = pGameAddr + 0x5AE810 + 0x1;
        ActiveOccludersNearby_Break =    pGameAddr + 0x5AE84E + 0x1;
        aml->Redirect(pGameAddr + 0x5AE80C + 0x1, (uintptr_t)ActiveOccludersNearby_Patch);
    }
}