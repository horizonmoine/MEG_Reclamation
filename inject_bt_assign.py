import os
import glob

src_dir = r"F:\MEG_Reclamation\Source\MEG_Reclamation\AI"

files = glob.glob(os.path.join(src_dir, "LiminalEntity_*.cpp"))
entities = ["Partygoer", "Smiler", "Clump", "Deathmoth", "Duller", "Jerry", "Skinwalker", "Watcher", "Wretch"]

for file in files:
    filename = os.path.basename(file)
    entity_name = filename.replace("LiminalEntity_", "").replace(".cpp", "")
    if entity_name in entities:
        with open(file, "r") as f:
            content = f.read()
        
        if "InitialBehaviorTree =" not in content:
            marker = f"MonsterType = EMonsterType::{entity_name};"
            injection = f"""MonsterType = EMonsterType::{entity_name};

\tstatic ConstructorHelpers::FObjectFinder<UBehaviorTree> BTFinder(TEXT("/Game/AI/BT_{entity_name}.BT_{entity_name}"));
\tif (BTFinder.Succeeded())
\t{{
\t\tInitialBehaviorTree = BTFinder.Object;
\t}}"""
            if marker in content:
                content = content.replace(marker, injection)
                with open(file, "w") as f:
                    f.write(content)
                print(f"Updated {filename}")
            else:
                print(f"Marker not found in {filename}")
