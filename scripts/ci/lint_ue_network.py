#!/usr/bin/env python3
"""Garde-fous architecturaux Server-Authoritative pour M.E.G. : Reclamation.

Usage : python scripts/ci/lint_ue_network.py Source
Code de sortie 1 si au moins une ERREUR hors baseline. Les WARN n'echouent pas le job.

Baseline : scripts/ci/lint_baseline.txt liste des violations heritees tolerees
(format "chemin: message", une par ligne). Elles sont retrogradees en WARN (baseline)
et doivent disparaitre au fil des missions. Ne jamais y ajouter une violation nouvelle.
"""
import re
import sys
from pathlib import Path

ERRORS = []
WARNS = []
BASELINE = set()
BASELINE_PATH = Path(__file__).with_name("lint_baseline.txt")

RE_SERVER_RPC = re.compile(r"UFUNCTION\s*\([^)]*\bServer\b")
RE_REPLICATED = re.compile(r"UPROPERTY\s*\([^)]*\bReplicated(Using)?\b")
RE_TIME_REMAINING = re.compile(r"\bfloat\s+\w*TimeRemaining\w*\s*[;=]")
RE_RAW_UOBJECT_PTR = re.compile(r"^\s*(class\s+)?[AU][A-Z]\w*\s*\*\s*\w+\s*(=\s*nullptr)?\s*;")
RE_REPL_ARRAY_UOBJ = re.compile(r"\bTArray\s*<\s*[AU][A-Z]\w*\s*\*\s*>")
RE_REPL_TMAP = re.compile(r"\bTMap\s*<")
RE_HEAVY_INCLUDE_ERROR = re.compile(r'#include\s+"(Engine/Engine\.h|Kismet/[^"]+)"')
RE_HEAVY_INCLUDE_WARN = re.compile(r'#include\s+"Components/[^"]+"')
RE_INCLUDE = re.compile(r'^\s*#include\s+"([^"]+)"')
RE_DEBUG = re.compile(r"AddOnScreenDebugMessage|DrawDebug\w+\(")
RE_SHIPPING_GUARD = re.compile(r"#if\s+!UE_BUILD_SHIPPING|#if\s+WITH_EDITOR|#if\s+ENABLE_DRAW_DEBUG")
RE_PP_IF = re.compile(r"^\s*#\s*if")
RE_PP_ENDIF = re.compile(r"^\s*#\s*endif")
RE_GET_ALL_ACTORS = re.compile(r"GetAllActorsOfClass")
RE_TICK_DEF = re.compile(r"::Tick\s*\(")

FORBIDDEN_RPC_BASES = ("GameStateBase", "GameModeBase", "AGameState", "AGameMode")


def load_baseline():
    if not BASELINE_PATH.exists():
        return
    for raw in BASELINE_PATH.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if line and not line.startswith("#"):
            BASELINE.add(line)


def report(bucket, path, line_no, msg):
    if bucket is ERRORS and f"{path}: {msg}" in BASELINE:
        WARNS.append(f"{path}:{line_no}: (baseline) {msg}")
        return
    bucket.append(f"{path}:{line_no}: {msg}")


def lint_header(path, lines):
    text = "\n".join(lines)
    is_uclass = "GENERATED_BODY()" in text or "GENERATED_USTRUCT_BODY()" in text
    is_gamemode_or_state = any(b in text for b in FORBIDDEN_RPC_BASES)

    includes = [(i, m.group(1)) for i, l in enumerate(lines, 1) if (m := RE_INCLUDE.match(l))]
    if is_uclass and includes:
        gen = [i for i, inc in includes if inc.endswith(".generated.h")]
        if not gen:
            report(ERRORS, path, 1, "header UCLASS/USTRUCT sans include .generated.h")
        elif gen[0] != includes[-1][0]:
            report(ERRORS, path, gen[0], ".generated.h doit etre le DERNIER #include")

    prev_uproperty = False
    prev_replicated = False
    for i, line in enumerate(lines, 1):
        if RE_HEAVY_INCLUDE_ERROR.search(line):
            report(ERRORS, path, i, "include lourd interdit dans un .h (forward declare, include dans le .cpp)")
        elif RE_HEAVY_INCLUDE_WARN.search(line):
            report(WARNS, path, i, "include Components/* dans un .h : acceptable pour la classe parente, sinon forward declare")
        if is_gamemode_or_state and RE_SERVER_RPC.search(line):
            report(ERRORS, path, i, "Server RPC interdit sur GameState/GameMode (non possedes par un client)")
        if prev_replicated and RE_TIME_REMAINING.search(line):
            report(ERRORS, path, i, "float replique *TimeRemaining : repliquer un timestamp serveur (StartServerTime)")
        if prev_replicated and RE_REPL_ARRAY_UOBJ.search(line):
            report(ERRORS, path, i, "TArray<UObject*> replique : utiliser FFastArraySerializer ou TObjectPtr dans une struct")
        if prev_replicated and RE_REPL_TMAP.search(line):
            report(ERRORS, path, i, "TMap replique : non supporte par la replication UE")
        if prev_uproperty and RE_RAW_UOBJECT_PTR.match(line) and "TObjectPtr" not in line:
            report(WARNS, path, i, "pointeur brut UPROPERTY : utiliser TObjectPtr<T>")
        stripped = line.strip()
        if stripped.startswith("UPROPERTY"):
            prev_uproperty = True
            prev_replicated = bool(RE_REPLICATED.search(line))
        elif stripped and not stripped.startswith("//") and not stripped.startswith("/*"):
            prev_uproperty = False
            prev_replicated = False


def lint_source(path, lines):
    guard_depth = 0
    pp_stack = []
    in_tick = False
    brace_depth = 0
    for i, line in enumerate(lines, 1):
        if RE_PP_IF.match(line):
            pp_stack.append(bool(RE_SHIPPING_GUARD.search(line)))
            if pp_stack[-1]:
                guard_depth += 1
        elif RE_PP_ENDIF.match(line) and pp_stack:
            if pp_stack.pop():
                guard_depth -= 1
        if RE_DEBUG.search(line) and guard_depth == 0:
            report(WARNS, path, i, "debug visuel hors #if !UE_BUILD_SHIPPING")
        if RE_TICK_DEF.search(line):
            in_tick = True
            brace_depth = 0
        if in_tick:
            brace_depth += line.count("{") - line.count("}")
            if RE_GET_ALL_ACTORS.search(line):
                report(ERRORS, path, i, "GetAllActorsOfClass dans Tick interdit")
            if brace_depth <= 0 and "}" in line:
                in_tick = False


def main():
    load_baseline()
    root = Path(sys.argv[1] if len(sys.argv) > 1 else "Source")
    files = list(root.rglob("*.h")) + list(root.rglob("*.cpp"))
    for f in files:
        try:
            lines = f.read_text(encoding="utf-8", errors="ignore").splitlines()
        except OSError as exc:
            report(WARNS, f, 0, f"lecture impossible : {exc}")
            continue
        rel = f.as_posix()
        if f.suffix == ".h":
            lint_header(rel, lines)
        else:
            lint_source(rel, lines)

    for w in WARNS:
        print(f"WARN  {w}")
    for e in ERRORS:
        print(f"ERROR {e}")
    print(f"\n{len(files)} fichiers analyses : {len(ERRORS)} erreur(s), {len(WARNS)} avertissement(s), {len(BASELINE)} entree(s) baseline")
    sys.exit(1 if ERRORS else 0)


if __name__ == "__main__":
    main()
