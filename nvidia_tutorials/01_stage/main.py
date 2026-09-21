from pxr import Usd, Sdf 
import os 

stage : Usd.Stage = Usd.Stage.CreateNew("_assets/root_layer_example.usda")

root_layer : Sdf.Layer = stage.GetRootLayer()
print("Root Layer identifier:", os.path.relpath(root_layer.identifier))
stage.DefinePrim("/World", "Xform")

extra_layer : Sdf.Layer = Sdf.Layer.CreateNew("_assets/extra_layer.usdc")

rel_path = "./" + os.path.basename(extra_layer.identifier)
print(f"the rel_path{rel_path}")
root_layer.subLayerPaths.append(rel_path)
stage.Save()
extra_layer.Save()

# Print the contents of the root layer:
print("Root layer contents:")
print(root_layer.ExportToString())