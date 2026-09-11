---
description: Revue d'architecture réseau par ue_cpp_architect sur le diff courant
---

1. Lister les fichiers modifiés (`git diff --name-only main...HEAD`).
2. Pour chaque `.h`/`.cpp`, vérifier point par point les sections 2, 3 et 4 de `.agents/rules/ue5_cpp_network_rules.md`.
3. Vérifier la matrice de `docs/architecture/NETWORK_ARCHITECTURE.md` : chaque état est-il possédé par la bonne classe ? Chaque mutation passe-t-elle par le serveur ?
4. Produire un rapport `.agents/sentinel/architect_review_<date>.md` avec : verdict (APPROUVÉ / REJETÉ), liste des violations avec fichier:ligne, correctif proposé pour chacune.
5. Si REJETÉ, ne pas commiter. Relancer `/mission` avec le rapport en contexte.
