'''Defines a Sphere Prim'''

from pxr import Usd, UsdGeom

file_path = "_assets/sphere_prim.usda"
stage: Usd.Stage = Usd.Stage.CreateNew(file_path)
sphere : UsdGeom.Sphere = UsdGeom.Sphere.Define(stage,"/hello")
sphere.CreateRadiusAttr().Set(2)

stage.Save()
