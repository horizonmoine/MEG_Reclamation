---
description: Lance la mission C++ numérotée (ex. /mission 01)
agent: ue-cpp-architect
---

Exécute la mission numéro $ARGUMENTS du projet.

Protocole : @docs/missions/00_PROTOCOL.md

Règles : @.agents/rules/ue5_cpp_network_rules.md

Architecture : @docs/architecture/NETWORK_ARCHITECTURE.md

Ouvre le fichier `docs/missions/$ARGUMENTS_*.md`, lis tous les fichiers de sa section Contexte, propose un plan, attends ma validation, puis implémente et passe la porte de qualité complète.
