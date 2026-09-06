# Règles ProcGen et Environnements Liminaux

1. Déterminisme :
   - Toute génération de salle, placement d'item et spawn d'entité dépend exclusivement du Seed global d'expédition (`FRandomStream`).
   
2. Navigation & Accessibilité :
   - Vérification de la connectivité via graphe BFS avant d'instancier la géométrie.
   - Toujours une sortie valide (Sas M.E.G. d'extraction) située à une distance euclidienne minimale de 8000 unités (80m) du spawn.

3. Optimisation :
   - Utiliser `UInstancedStaticMeshComponent` ou `UHierarchicalInstancedStaticMeshComponent` pour les éléments récurrents (murs, sols, néons).
   - Collision simple (`BlockAll` ou `WorldStatic`) pour les murs porteurs, `Overlap` pour les déclencheurs d'ambiance sonore.
