import unreal

task = unreal.AssetImportTask()
task.filename = "F:/MEG_Reclamation/test_box.fbx"
task.destination_path = "/Game/Meshes/Test"
task.destination_name = "SM_TestBox"
task.automated = True
task.save = True
task.replace_existing = True

fbx_options = unreal.FbxImportUI()
fbx_options.import_mesh = True
fbx_options.import_as_skeletal = False
fbx_options.import_materials = False
fbx_options.import_textures = False

static_mesh_data = unreal.FbxStaticMeshImportData()
static_mesh_data.combine_meshes = True
static_mesh_data.generate_lightmap_u_vs = True
fbx_options.static_mesh_import_data = static_mesh_data

task.options = fbx_options

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_tools.import_asset_tasks([task])
print(">>> IMPORTED OBJECTS:", task.imported_object_paths)
