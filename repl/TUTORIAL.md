# A lesson at the prompt

<!-- clang-repl: 23 -->

This walk runs lesson 08 in `repl/usd-repl` and keeps going at the prompt:
looking at what the lesson made, adding to it one line at a time, a mistake
and its undo, a crash and its replay, saving the session as a program, and a
session that stays for an editor or an agent. It needs the configured build
(`cmake --preset default`) and clang-repl 23 (README, "REPL").

## Run the lesson, keep the prompt

<!-- check: session nvidia_tutorials/08_xform/main.cpp -->
```
$ repl/usd-repl nvidia_tutorials/08_xform/main.cpp
usd> stage
#usda 1.0

def Xform "World"
{
}

usd> world
UsdGeomXform: def Xform /World
  authored properties: none
  children: none
```

The launcher runs the lesson's `main()` one statement at a time, in
`build/output/08_xform/` as ctest does, then leaves you at the prompt with
its variables: `stage` and `world` are the lesson's. A bare expression, with
no semicolon, prints its value: a stage its usda, a prim its type, authored
properties and children.

## The next call, one line away

```
usd> auto ball = UsdGeomSphere::Define(stage, world.GetPath().AppendChild(TfToken("Ball")));
usd> UsdGeomXformCommonAPI(ball).SetTranslate(GfVec3d(0, 2, 0));
usd> ball.GetXformOpOrderAttr()
/World/Ball.xformOpOrder (token[]) = [xformOp:translate]
usd> tree(stage)
def Xform "World"
  def Sphere "Ball"
usd> ball.GetPrim().GetPrimStack()
[@xform_prim.usda@</World/Ball>]
```

`using namespace pxr;` is in effect, so `pxr::` is optional, and the common
USD headers are included already. (In an error, clang spells `pxr::` as the
versioned namespace it stands for.) `tree(stage)` prints the hierarchy alone.
An attribute prints its type and value, and its time samples when it has
some. `GetPrimStack()` shows the layer that holds the prim's opinion: the
lesson's file.

## A mistake, and %undo

```
usd> ball.GetRadius()
<input>:1:6: error: no member named 'GetRadius' in 'pxrInternal_v0_26_11__pxrReserved__::UsdGeomSphere'
    1 | ball.GetRadius();
      | ~~~~ ^
error: Parsing failed.
usd> ball.GetRadiusAttr().Set(2.0);
usd> ball.GetRadiusAttr()
/World/Ball.radius (double) = 2
usd> %undo
undo: ball.GetRadiusAttr()
[undo: restored 20 inputs]
usd> %undo
undo: ball.GetRadiusAttr().Set(2.0);
[undo: restored 19 inputs]
usd> ball.GetRadiusAttr()
/World/Ball.radius (double) = 1
```

An input that does not compile leaves the session as it was. `%undo` drops
the last input that ran and rebuilds the session without it. A rebuild
replays the journal, the lesson's own statements included, so a lesson that
writes a file writes it again.

## Keep it as a program

```
usd> ball.GetRadiusAttr().Set(0.5);
usd> stage->Save();
usd> %save-cpp ball.cpp
saved 22 inputs to …/ball.cpp
```

`%save-cpp` writes the session as a C++ program: the lesson's statements and
yours in `main()`, each bare expression as a `show(...)` call. It builds as a
lesson does (a directory under `nvidia_tutorials/` with `add_usd_example`),
or straight away with the session's flags:

<!-- check: sh -->
```
$ clang++ $(repl/usd-repl --cflags) ball.cpp $(repl/usd-repl --libs) -o ball && echo built
built
```

`repl/usd-repl ball.cpp` runs it in the REPL again.

## A crash costs a replay

<!-- check: off -->
```
usd> UsdStageRefPtr none;
usd> none->GetPseudoRoot()

---------------------------- clang-repl terminated -----------------------------
clang-repl crashed. FATAL ERROR: attempted member lookup on NULL TfRefPtr<UsdStage>
in operator-> at line 933 of /path/to/OpenUSD/include/pxr/base/tf/refPtr.h
writing crash report to [ host:/home/you/.local/state/cpp-repl/tmp/st_clang-repl.1377635 ] ... done.
--------------------------------------------------------------------------------
[clang-repl crashed (SIGABRT): restored 23 inputs]
usd> tree(stage)
def Xform "World"
  def Sphere "Ball"
```

`->` on a null stage is a fatal error in USD: it says what went wrong and
where, writes its stack under cpp-repl's state directory, and ends
clang-repl. The session comes back, without the input that crashed, in a
spare clang-repl the launcher keeps warm, so `stage` and `ball` are still
there.

## A session that stays

`--serve` keeps one session running on a socket, and `--connect` runs files
or inputs in it, so an editor or an agent pays the start-up once:

<!-- check: sh -->
```
$ repl/usd-repl --serve repl.sock --cwd build/output/08_xform > serve.log 2>&1 &
$ for i in $(seq 100); do test -S repl.sock && break; sleep 0.1; done
$ repl/usd-repl --connect repl.sock -q nvidia_tutorials/08_xform/main.cpp
#usda 1.0

def Xform "World"
{
}


$ repl/usd-repl --connect repl.sock <<< 'tree(stage)'
usd> tree(stage)
def Xform "World"
$ repl/usd-repl --connect repl.sock <<< '%quit'
usd> %quit
```

The session's working directory goes on the `--serve` line, since a lesson
writes into `_assets/` there. `--connect` takes the server's profile and
build. cpp-repl's [tutorial 04](https://github.com/farhatnadim/cpp_repl/blob/main/docs/tutorials/04-scripts-and-agents.md)
covers `--json`, exit statuses and `--fresh`.

## Next

- The README's REPL section: the launcher's defaults, how a lesson becomes
  inputs, editing and colour, limits.
- [cpp-repl's tutorials](https://github.com/farhatnadim/cpp_repl/blob/main/docs/tutorials/README.md):
  the REPL apart from OpenUSD.
