import os
import re

base_dir = "F:/MEG_Reclamation/Source/MEG_Reclamation/AI"

entities = {
    'Clump': ('S_Clump_Gurgle', 'S_Clump_Drag'),
    'Deathmoth': ('S_Deathmoth_Flutter', 'S_Deathmoth_Screech'),
    'Duller': ('S_Duller_Growl', 'S_Duller_Rush'),
    'Jerry': ('S_Jerry_Whisper', 'S_Jerry_Laugh'),
    'Skinwalker': ('S_Skinwalker_Mimic', 'S_Skinwalker_Scream'),
    'Watcher': ('S_Watcher_Hum', 'S_Watcher_Alert'),
    'Wretch': ('S_Wretch_Snarl', 'S_Wretch_Lunge')
}

for entity, (aggro, attack) in entities.items():
    file_path = os.path.join(base_dir, f"LiminalEntity_{entity}.cpp")
    if os.path.exists(file_path):
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # We find the constructor ALiminalEntity_X::ALiminalEntity_X()
        constructor_pattern = re.compile(rf"(ALiminalEntity_{entity}::ALiminalEntity_{entity}\(\)[\s\S]*?)(}})", re.MULTILINE)
        
        code_to_insert = f"""
\tstatic ConstructorHelpers::FObjectFinder<USoundBase> AggroFinder(TEXT("/Game/Audio/{aggro}.{aggro}"));
\tif (AggroFinder.Succeeded())
\t{{
\t\tAggroSound = AggroFinder.Object;
\t}}

\tstatic ConstructorHelpers::FObjectFinder<USoundBase> AttackFinder(TEXT("/Game/Audio/{attack}.{attack}"));
\tif (AttackFinder.Succeeded())
\t{{
\t\tAttackSound = AttackFinder.Object;
\t}}
"""
        new_content = constructor_pattern.sub(lambda m: m.group(1) + code_to_insert + "\n" + m.group(2), content, count=1)
        
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        
        print(f"Updated {entity}")
