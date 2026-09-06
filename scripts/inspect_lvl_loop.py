import unreal

report_path = r"F:\MEG_Reclamation\scripts\lvl_loop_report.txt"
world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/Lvl_Loop")
actors = unreal.EditorLevelLibrary.get_all_level_actors()

lines = [f"=== ACTORS IN Lvl_Loop ({len(actors)}) ==="]
for a in actors:
    loc = a.get_actor_location()
    lines.append(f"Actor: {a.get_name():30} Class: {a.get_class().get_name():25} Loc: ({loc.x:7.1f}, {loc.y:7.1f}, {loc.z:7.1f})")

with open(report_path, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))
