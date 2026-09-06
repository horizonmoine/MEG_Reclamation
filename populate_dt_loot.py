import unreal
import json

json_data = [
  {
    "Name": "Scrap_Electronics",
    "ItemId": "Scrap_Electronics",
    "DisplayName": "Composants Electroniques",
    "WeightKg": 4.5,
    "CreditsValue": 85,
    "Mesh": "/Game/Meshes/Props/SM_Scrap_Electronics.SM_Scrap_Electronics",
    "CollisionSound": "/Game/Audio/S_Loot_Impact_Plastic.S_Loot_Impact_Plastic",
    "EffectTag": "None"
  },
  {
    "Name": "Scrap_CopperCable",
    "ItemId": "Scrap_CopperCable",
    "DisplayName": "Bobine de Cuivre M.E.G.",
    "WeightKg": 12.0,
    "CreditsValue": 140,
    "Mesh": "/Game/Meshes/Props/SM_Scrap_CopperCable.SM_Scrap_CopperCable",
    "CollisionSound": "/Game/Audio/S_Loot_Impact_Metal.S_Loot_Impact_Metal",
    "EffectTag": "None"
  },
  {
    "Name": "Item_MegRadio",
    "ItemId": "Item_MegRadio",
    "DisplayName": "Radio d'escouade endommagee",
    "WeightKg": 3.5,
    "CreditsValue": 190,
    "Mesh": "/Game/Meshes/Tools/SM_WalkieTalkie.SM_WalkieTalkie",
    "CollisionSound": "/Game/Audio/S_Loot_Impact_Plastic.S_Loot_Impact_Plastic",
    "EffectTag": "None"
  },
  {
    "Name": "Item_AlmondWater",
    "ItemId": "Item_AlmondWater",
    "DisplayName": "Bouteille d'Eau d'Amande (500ml)",
    "WeightKg": 1.0,
    "CreditsValue": 90,
    "Mesh": "/Game/Meshes/Props/SM_Almond_Water_Bottle.SM_Almond_Water_Bottle",
    "CollisionSound": "/Game/Audio/S_Loot_Impact_Plastic.S_Loot_Impact_Plastic",
    "EffectTag": "Heal"
  },
  {
    "Name": "Item_Battery9V",
    "ItemId": "Item_Battery9V",
    "DisplayName": "Pile 9V Industrielle",
    "WeightKg": 0.5,
    "CreditsValue": 50,
    "Mesh": "/Game/Meshes/Props/SM_Battery_9V.SM_Battery_9V",
    "CollisionSound": "/Game/Audio/S_Loot_Impact_Metal.S_Loot_Impact_Metal",
    "EffectTag": "Battery"
  },
  {
    "Name": "Item_RetroComputer",
    "ItemId": "Item_RetroComputer",
    "DisplayName": "Terminal portable Mod. 4",
    "WeightKg": 18.0,
    "CreditsValue": 350,
    "Mesh": "/Game/Meshes/Props/SM_Retro_Computer.SM_Retro_Computer",
    "CollisionSound": "/Game/Audio/S_Loot_Impact_Heavy.S_Loot_Impact_Heavy",
    "EffectTag": "Intel"
  },
  {
    "Name": "Item_DeclassifiedTape",
    "ItemId": "Item_DeclassifiedTape",
    "DisplayName": "Cassette classee Secret M.E.G.",
    "WeightKg": 0.2,
    "CreditsValue": 220,
    "Mesh": "/Game/Meshes/Props/SM_Declassified_Tape.SM_Declassified_Tape",
    "CollisionSound": "/Game/Audio/S_Loot_Impact_Plastic.S_Loot_Impact_Plastic",
    "EffectTag": "Lore"
  },
  {
    "Name": "Item_MilitaryMRE",
    "ItemId": "Item_MilitaryMRE",
    "DisplayName": "Ration de survie scellee",
    "WeightKg": 1.5,
    "CreditsValue": 60,
    "Mesh": "/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG",
    "CollisionSound": "/Game/Audio/S_Loot_Impact_Plastic.S_Loot_Impact_Plastic",
    "EffectTag": "Food"
  },
  {
    "Name": "Item_GeigerCounter",
    "ItemId": "Item_GeigerCounter",
    "DisplayName": "Compteur Geiger analogique",
    "WeightKg": 3.0,
    "CreditsValue": 160,
    "Mesh": "/Game/Meshes/Props/SM_Geiger_Counter.SM_Geiger_Counter",
    "CollisionSound": "/Game/Audio/S_Loot_Impact_Metal.S_Loot_Impact_Metal",
    "EffectTag": "Detection"
  },
  {
    "Name": "Item_AnomalousCore",
    "ItemId": "Item_AnomalousCore",
    "DisplayName": "Fragment de cristal non-euclidien",
    "WeightKg": 7.0,
    "CreditsValue": 500,
    "Mesh": "/Game/Meshes/Props/SM_Anomalous_Core.SM_Anomalous_Core",
    "CollisionSound": "/Game/Audio/S_Loot_Impact_Metal.S_Loot_Impact_Metal",
    "EffectTag": "Anomaly"
  },
  {
    "Name": "Item_FirstAidKit",
    "ItemId": "Item_FirstAidKit",
    "DisplayName": "Trousse de secours d'urgence",
    "WeightKg": 2.5,
    "CreditsValue": 110,
    "Mesh": "/Game/Meshes/Props/SM_Medkit_MEG.SM_Medkit_MEG",
    "CollisionSound": "/Game/Audio/S_Loot_Impact_Plastic.S_Loot_Impact_Plastic",
    "EffectTag": "Medical"
  },
  {
    "Name": "Item_HeavyLeadPlate",
    "ItemId": "Item_HeavyLeadPlate",
    "DisplayName": "Blindage anti-radiations en plomb",
    "WeightKg": 35.0,
    "CreditsValue": 280,
    "Mesh": "/Game/Meshes/Props/SM_Heavy_Lead_Plate.SM_Heavy_Lead_Plate",
    "CollisionSound": "/Game/Audio/S_Loot_Impact_Heavy.S_Loot_Impact_Heavy",
    "EffectTag": "Heavy"
  }
]

table = unreal.load_asset('/Game/Data/DT_LootItems')
if table:
    json_str = json.dumps(json_data)
    ok = unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(table, json_str)
    print(f"Fill data table result: {ok}")
    if ok:
        unreal.EditorAssetLibrary.save_loaded_asset(table)
        print("DT_LootItems successfully updated with Meshes, Audio, Weights, and Values!")
    else:
        print("Failed to fill DT_LootItems from JSON.")
else:
    print("DT_LootItems not found!")
