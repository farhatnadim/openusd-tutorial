// usd_repl.h: the prelude every repl/usd-repl session starts from.
//
// repl/usd-repl passes this file to clang-repl with -include, so it is parsed
// once at startup, before the first input. clang-repl 18 never executes code
// from an -include'd file. This header may therefore hold declarations, inline
// functions and templates, but no global with a dynamic initialiser and nothing
// that has to run.
//
// <iostream> comes first on purpose. GCC 13's <iostream> carries module-level
// asm that clang-repl 18 cannot JIT. Parsed here, it never reaches code
// generation, and an `#include <iostream>` typed later is a no-op.

#pragma once

#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include <sys/prctl.h>   // used by repl/usd-repl's start-up inputs

#include <pxr/base/arch/demangle.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/tf/enum.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/vt/value.h>

#include <pxr/usd/kind/registry.h>
#include <pxr/usd/sdf/attributeSpec.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/types.h>

#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/collectionAPI.h>
#include <pxr/usd/usd/editTarget.h>
#include <pxr/usd/usd/inherits.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/usd/payloads.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primCompositionQuery.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/references.h>
#include <pxr/usd/usd/relationship.h>
#include <pxr/usd/usd/schemaBase.h>
#include <pxr/usd/usd/specializes.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/variantSets.h>

#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/capsule.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/plane.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/primvar.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/scope.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>

#include <pxr/usd/usdLux/diskLight.h>
#include <pxr/usd/usdLux/distantLight.h>
#include <pxr/usd/usdLux/domeLight.h>
#include <pxr/usd/usdLux/lightAPI.h>
#include <pxr/usd/usdLux/rectLight.h>
#include <pxr/usd/usdLux/sphereLight.h>

#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdShade/shader.h>

using namespace pxr;

// Helpers for seeing what the API did. show(x) prints any value; a bare
// expression typed without a semicolon is printed through it. tree(stage)
// prints the prim hierarchy the way usda nests it.
namespace repl {

inline const char *specifier(SdfSpecifier spec)
{
  switch (spec) {
    case SdfSpecifierDef:   return "def";
    case SdfSpecifierOver:  return "over";
    case SdfSpecifierClass: return "class";
    default:                return "?";
  }
}

// One value as text, without a newline. Inside a list a prim, a property or
// a wrapper around an attribute (UsdGeomPrimvar, UsdShadeInput, ...) is its
// path.
template <class T>
void put(std::ostream &os, const T &x)
{
  if constexpr (std::is_same_v<T, bool>) {
    os << (x ? "true" : "false");
  } else if constexpr (std::is_same_v<T, std::string>) {
    os << '"' << x << '"';
  } else if constexpr (std::is_base_of_v<UsdObject, T> ||
                       std::is_base_of_v<UsdSchemaBase, T>) {
    os << x.GetPath();
  } else if constexpr (requires { x.GetAttr().GetPath(); }) {
    os << x.GetAttr().GetPath();
  } else if constexpr (std::is_enum_v<T>) {
    const std::string name = TfEnum::GetName(TfEnum(x));
    if (name.empty())
      os << static_cast<std::underlying_type_t<T>>(x);
    else
      os << name;
  } else if constexpr (requires (std::ostream &out) { out << x; }) {
    os << x;
  } else if constexpr (requires { x.begin(); x.end(); }) {
    os << '[';
    bool first = true;
    for (const auto &item : x) {
      if (!first)
        os << ", ";
      first = false;
      put(os, item);
    }
    os << ']';
  } else {
    os << '<' << ArchGetDemangled<T>() << '>';
  }
}

inline void show_stage(const UsdStage *stage)
{
  if (!stage) {
    std::cout << "null stage" << std::endl;
    return;
  }
  std::string text;
  stage->ExportToString(&text, /*addSourceFileComment=*/false);
  std::cout << text << std::flush;
}

inline void show_layer(const SdfLayer *layer)
{
  if (!layer) {
    std::cout << "null layer" << std::endl;
    return;
  }
  std::string text;
  layer->ExportToString(&text);
  std::cout << text << std::flush;
}

inline void show(const UsdStageRefPtr &stage) { show_stage(get_pointer(stage)); }
inline void show(const UsdStagePtr &stage) { show_stage(get_pointer(stage)); }
inline void show(const SdfLayerRefPtr &layer) { show_layer(get_pointer(layer)); }
inline void show(const SdfLayerHandle &layer) { show_layer(get_pointer(layer)); }

inline void show(const UsdPrim &prim)
{
  if (!prim) {
    std::cout << "invalid prim" << std::endl;
    return;
  }
  std::cout << specifier(prim.GetSpecifier());
  if (!prim.GetTypeName().IsEmpty())
    std::cout << ' ' << prim.GetTypeName();
  std::cout << ' ' << prim.GetPath() << "\n  authored properties:";
  const TfTokenVector names = prim.GetAuthoredPropertyNames();
  if (names.empty())
    std::cout << " none";
  for (const TfToken &name : names)
    std::cout << ' ' << name;
  std::cout << "\n  children:";
  bool any = false;
  for (const UsdPrim &child : prim.GetAllChildren()) {
    std::cout << ' ' << child.GetName();
    any = true;
  }
  if (!any)
    std::cout << " none";
  std::cout << std::endl;
}

inline void show(const UsdAttribute &attr)
{
  if (!attr) {
    std::cout << "invalid attribute" << std::endl;
    return;
  }
  std::cout << attr.GetPath() << " (" << attr.GetTypeName().GetAsToken() << ')';
  VtValue value;
  if (attr.Get(&value))
    std::cout << " = " << value;
  else
    std::cout << " has no default value";
  std::vector<double> times;
  if (attr.GetTimeSamples(&times) && !times.empty()) {
    std::cout << "\n  time samples: {";
    for (size_t i = 0; i < times.size(); ++i) {
      VtValue sample;
      attr.Get(&sample, times[i]);
      std::cout << (i ? ", " : "") << times[i] << ": " << sample;
    }
    std::cout << '}';
  }
  std::cout << std::endl;
}

inline void show(const UsdRelationship &rel)
{
  if (!rel) {
    std::cout << "invalid relationship" << std::endl;
    return;
  }
  SdfPathVector targets;
  rel.GetTargets(&targets);
  std::cout << rel.GetPath() << " ->";
  if (targets.empty())
    std::cout << " no targets";
  for (const SdfPath &target : targets)
    std::cout << ' ' << target;
  std::cout << std::endl;
}

inline void show(const UsdProperty &prop)
{
  if (prop.Is<UsdAttribute>())
    show(prop.As<UsdAttribute>());
  else if (prop.Is<UsdRelationship>())
    show(prop.As<UsdRelationship>());
  else
    std::cout << "invalid property" << std::endl;
}

inline void show(bool value) { std::cout << (value ? "true" : "false") << std::endl; }
inline void show(const std::string &text) { std::cout << '"' << text << '"' << std::endl; }
inline void show(const char *text)
{
  if (text)
    std::cout << '"' << text << '"' << std::endl;
  else
    std::cout << "nullptr" << std::endl;
}

// Everything else: a schema object shows its prim, a wrapper around an
// attribute (UsdGeomPrimvar, UsdShadeInput, ...) shows that attribute, and
// the rest prints through put().
template <class T>
void show(const T &x)
{
  if constexpr (std::is_base_of_v<UsdSchemaBase, T>) {
    std::cout << ArchGetDemangled<T>() << ": ";
    show(x.GetPrim());
  } else if constexpr (requires { x.GetAttr(); } &&
                       !requires (std::ostream &out) { out << x; }) {
    show(x.GetAttr());
  } else {
    put(std::cout, x);
    std::cout << std::endl;
  }
}

// The prim hierarchy, one line per prim: specifier, type, name.
inline void tree(const UsdPrim &prim, int depth = 0)
{
  std::cout << std::string(2 * depth, ' ') << specifier(prim.GetSpecifier());
  if (!prim.GetTypeName().IsEmpty())
    std::cout << ' ' << prim.GetTypeName();
  std::cout << " \"" << prim.GetName() << '"' << std::endl;
  for (const UsdPrim &child : prim.GetAllChildren())
    tree(child, depth + 1);
}

inline void tree(const UsdStageRefPtr &stage)
{
  if (!stage) {
    std::cout << "null stage" << std::endl;
    return;
  }
  for (const UsdPrim &prim : stage->GetPseudoRoot().GetAllChildren())
    tree(prim);
}

// What repl/usd-repl sends for a bare expression: show its value, or just run
// it when it is void. A void expression ends its output with '\x1e', which
// tells repl/usd-repl to save it as a plain statement in %save-cpp.
template <class F>
void print_expr(F f)
{
  if constexpr (std::is_void_v<decltype(f())>) {
    f();
    std::cout << '\x1e' << std::flush;
  } else {
    show(f());
  }
}

}  // namespace repl

using repl::show;
using repl::tree;
