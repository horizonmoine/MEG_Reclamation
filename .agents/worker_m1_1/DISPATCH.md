## 2026-09-06T02:49:18Z

You are teamwork_preview_worker for Milestone M1 (R1: Skeletal Animation & Bestiary) of M.E.G. : Reclamation (Unreal Engine 5.8).
Your working directory is F:/MEG_Reclamation/.agents/worker_m1_1.
You MUST read F:/MEG_Reclamation/ORIGINAL_REQUEST.md and F:/MEG_Reclamation/PROJECT.md before doing anything else.

Carefully study the 3 architectural handoffs from the Explorers:
- F:/MEG_Reclamation/.agents/explorer_m1_1/handoff.md (Scavenger 1P arms & 3P body)
- F:/MEG_Reclamation/.agents/explorer_m1_2/handoff.md (Entity skeletal mesh & AnimNotify damage trace)
- F:/MEG_Reclamation/.agents/explorer_m1_3/handoff.md (Bestiary rigging, Blender procedural pipeline, audio wiring)

Scope of Work (Milestone M1):
1. **Scavenger Character (Features F01, F02)**:
   - In Source/MEG_Reclamation/Player/ScavengerCharacter.h/.cpp:
     * Add FirstPersonMesh (USkeletalMeshComponent) attached to FirstPersonCamera with bOnlyOwnerSee = true, CastShadow = false, NoCollision, offset FVector(-10.0f, 0.0f, -155.0f), rotation FRotator(0.0f, -90.0f, 0.0f).
     * Configure root GetMesh() with SKM_Manny_Simple and ABP_Unarmed (UABP_Unarmed_C), bOwnerNoSee = true, bCastHiddenShadow = true, offset FVector(0.0f, 0.0f, -90.0f), rotation FRotator(0.0f, -90.0f, 0.0f).
     * Attach FirstPersonToolMesh to FirstPersonMesh socket "hand_r" (or FirstPersonCamera) with bOnlyOwnerSee = true.
2. **Entity Architecture & AnimNotify Damage (Features F03, F05)**:
   - In Source/MEG_Reclamation/AI/LiminalEntity.h/.cpp:
     * Retain BodyMesh (UStaticMeshComponent) for legacy CDO compatibility, but activate GetMesh() (USkeletalMeshComponent) when DefaultSkeletalMesh is assigned. Add SetEntityVisualScale and SetEntityVisualVisibility helper functions.
     * Implement AnimNotify class UAnimNotify_LiminalAttackTrace (in a new header/cpp or within LiminalEntity) executing server-authoritative SweepMultiByChannel along ECC_Pawn from socket "AttackSocket" (with forward fallback), applying point damage to hit pawns and playing impact audio.
     * In PerformMeleeAttack(), preserve fallback damage for headless -nullrhi automation tests.
3. **Bestiary Rigging & Audio Wiring (Features F04, F06)**:
   - For the 5 humanoid entities (Partygoer, Skinwalker, Wretch, Duller, Smiler): map to SK_Mannequin / SKM_Manny_Simple via GetMesh(), customized with entity-specific scale, materials, and socket accessories.
   - For the 4 non-humanoid entities (Hound, Deathmoth, Clump, Jerry): run procedural Blender 5.2.1 (F:\blender\blender.exe) generation script to generate rigged FBX models with Idle, Walk/Fly, Attack, and Death actions, and import/wire them in UE.
   - Connect attack sounds, aggro cues, and footstep sounds to anim events / AnimNotifies.
4. **Build & Test Verification**:
   - Compile the project in Development Win64.
   - Execute powershell -ExecutionPolicy Bypass -File "F:/MEG_Reclamation/Run_Automation_Tests.ps1" and verify all 30 tests pass with Exit Code 0.
   - Execute powershell -ExecutionPolicy Bypass -File "F:/MEG_Reclamation/Run_Auto_Check.ps1" and verify 27/27 controls pass.
